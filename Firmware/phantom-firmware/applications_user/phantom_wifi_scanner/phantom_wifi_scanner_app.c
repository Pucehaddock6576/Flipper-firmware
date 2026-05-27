#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>
#include <expansion/expansion.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define UART_BAUD      115200
#define RX_BUF_SIZE    2048
#define LINE_BUF_SIZE  256
#define MAX_NETWORKS   64
#define VISIBLE_LINES  5

#define WORKER_EVT_RX   (1UL << 0)
#define WORKER_EVT_STOP (1UL << 1)

typedef struct {
    char ssid[33];
    int8_t rssi;
    uint8_t channel;
    char encryption[8];
} WifiNetwork;

typedef enum { EventTypeKey, EventTypeTick } EventType;
typedef struct { EventType type; InputEvent input; } AppEvent;
typedef enum { ScanIdle, ScanActive, ScanDone } ScanState;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    FuriMutex* mutex;
    WifiNetwork networks[MAX_NETWORKS];
    uint8_t network_count;
    int16_t scroll_pos;
    ScanState scan_state;
    FuriHalSerialHandle* serial_handle;
    FuriStreamBuffer* rx_stream;
    FuriThread* worker_thread;
    char line_buf[LINE_BUF_SIZE];
    size_t line_pos;
    Expansion* expansion;
} App;

static uint8_t rssi_to_bars(int8_t rssi) {
    if(rssi >= -50) return 4;
    if(rssi >= -60) return 3;
    if(rssi >= -70) return 2;
    if(rssi >= -80) return 1;
    return 0;
}

static void parse_line(App* app, const char* line) {
    if(app->network_count >= MAX_NETWORKS) return;
    WifiNetwork net = {0};

    const char* ssid_tag = strstr(line, "SSID: ");
    const char* rssi_tag = strstr(line, "RSSI: ");
    const char* ch_tag = strstr(line, "Ch: ");

    if(ssid_tag && rssi_tag && ch_tag) {
        ssid_tag += 6;
        const char* end = strstr(ssid_tag, ",");
        if(!end) return;
        size_t len = (size_t)(end - ssid_tag);
        if(len > 32) len = 32;
        memcpy(net.ssid, ssid_tag, len);
        while(len > 0 && net.ssid[len - 1] == ' ') net.ssid[--len] = '\0';
        net.rssi = (int8_t)strtol(rssi_tag + 6, NULL, 10);
        net.channel = (uint8_t)strtoul(ch_tag + 4, NULL, 10);
        const char* enc = strstr(line, "Enc: ");
        if(enc) {
            enc += 5;
            size_t el = 0;
            while(enc[el] && enc[el] != ',' && enc[el] != '\n' && el < 7) el++;
            memcpy(net.encryption, enc, el);
        }
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        app->networks[app->network_count++] = net;
        furi_mutex_release(app->mutex);
        return;
    }

    char sbuf[33] = {0};
    int rv = 0, cv = 0;
    char ebuf[8] = {0};
    if(sscanf(line, " %32[^,],%d,%d,%7s", sbuf, &rv, &cv, ebuf) >= 3) {
        strncpy(net.ssid, sbuf, 32);
        net.rssi = (int8_t)rv;
        net.channel = (uint8_t)cv;
        strncpy(net.encryption, ebuf, 7);
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        app->networks[app->network_count++] = net;
        furi_mutex_release(app->mutex);
    }
}

static void uart_rx_cb(
    FuriHalSerialHandle* handle, FuriHalSerialRxEvent ev, size_t size, void* ctx) {
    App* app = ctx;
    if(ev & (FuriHalSerialRxEventData | FuriHalSerialRxEventIdle)) {
        uint8_t data[FURI_HAL_SERIAL_DMA_BUFFER_SIZE];
        while(size) {
            size_t ret = furi_hal_serial_dma_rx(handle, data,
                (size > FURI_HAL_SERIAL_DMA_BUFFER_SIZE) ? FURI_HAL_SERIAL_DMA_BUFFER_SIZE : size);
            furi_stream_buffer_send(app->rx_stream, data, ret, 0);
            size -= ret;
        }
        furi_thread_flags_set(furi_thread_get_id(app->worker_thread), WORKER_EVT_RX);
    }
}

static int32_t uart_worker(void* ctx) {
    App* app = ctx;
    while(true) {
        uint32_t events = furi_thread_flags_wait(
            WORKER_EVT_RX | WORKER_EVT_STOP, FuriFlagWaitAny, FuriWaitForever);
        furi_check(!(events & FuriFlagError));
        if(events & WORKER_EVT_STOP) break;
        if(events & WORKER_EVT_RX) {
            uint8_t data[64];
            size_t len;
            do {
                len = furi_stream_buffer_receive(app->rx_stream, data, sizeof(data), 0);
                for(size_t i = 0; i < len; i++) {
                    char c = (char)data[i];
                    if(c == '\n' || c == '\r') {
                        if(app->line_pos > 0) {
                            app->line_buf[app->line_pos] = '\0';
                            parse_line(app, app->line_buf);
                            app->line_pos = 0;
                        }
                    } else if(app->line_pos < LINE_BUF_SIZE - 1) {
                        app->line_buf[app->line_pos++] = c;
                    }
                }
            } while(len > 0);
            view_port_update(app->view_port);
        }
    }
    return 0;
}

static void draw_cb(Canvas* canvas, void* ctx) {
    App* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    char title[40];
    if(app->scan_state == ScanActive)
        snprintf(title, sizeof(title), "WiFi Scanner [scanning]");
    else
        snprintf(title, sizeof(title), "WiFi Scanner [%d found]", app->network_count);
    canvas_draw_str(canvas, 0, 10, title);
    canvas_draw_line(canvas, 0, 12, 127, 12);

    canvas_set_font(canvas, FontSecondary);
    uint8_t fh = canvas_current_font_height(canvas);

    if(app->network_count == 0) {
        const char* msg = (app->scan_state == ScanActive) ? "Scanning..." : "Press OK to scan";
        canvas_draw_str_aligned(canvas, 64, 38, AlignCenter, AlignCenter, msg);
    } else {
        uint8_t vis = (app->network_count < VISIBLE_LINES) ? app->network_count : VISIBLE_LINES;
        for(uint8_t i = 0; i < vis; i++) {
            int16_t idx = app->scroll_pos + i;
            if(idx >= app->network_count) break;
            WifiNetwork* net = &app->networks[idx];
            uint8_t y = 14 + i * (fh + 1);
            uint8_t bars = rssi_to_bars(net->rssi);
            uint8_t bar_base = y + fh - 1;
            for(uint8_t b = 0; b < 4; b++) {
                uint8_t bh = (b + 1) * 2;
                if(b < bars)
                    canvas_draw_box(canvas, 1 + b * 3, bar_base - bh + 1, 2, bh);
                else
                    canvas_draw_box(canvas, 1 + b * 3, bar_base, 2, 1);
            }
            char ssid_disp[18];
            if(strlen(net->ssid) == 0)
                strncpy(ssid_disp, "<hidden>", sizeof(ssid_disp));
            else if(strlen(net->ssid) > 14) {
                memcpy(ssid_disp, net->ssid, 14);
                ssid_disp[14] = '\0';
            } else
                strncpy(ssid_disp, net->ssid, sizeof(ssid_disp));
            ssid_disp[sizeof(ssid_disp) - 1] = '\0';
            canvas_draw_str(canvas, 15, y + fh - 1, ssid_disp);
            char ch_str[8];
            snprintf(ch_str, sizeof(ch_str), "ch%d", net->channel);
            canvas_draw_str(canvas, 104, y + fh - 1, ch_str);
        }
        if(app->network_count > VISIBLE_LINES) {
            elements_scrollbar(canvas, app->scroll_pos, app->network_count);
        }
    }
    furi_mutex_release(app->mutex);
}

static void input_cb(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* q = ctx;
    AppEvent ev = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(q, &ev, FuriWaitForever);
}

static void send_scan(App* app) {
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    app->network_count = 0;
    app->scroll_pos = 0;
    app->scan_state = ScanActive;
    app->line_pos = 0;
    furi_mutex_release(app->mutex);
    furi_stream_buffer_reset(app->rx_stream);
    furi_hal_serial_tx(app->serial_handle, (const uint8_t*)"scanap\n", 7);
    view_port_update(app->view_port);
}

int32_t phantom_wifi_scanner_app(void* p) {
    UNUSED(p);
    App* app = malloc(sizeof(App));
    memset(app, 0, sizeof(App));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    app->expansion = furi_record_open(RECORD_EXPANSION);
    expansion_disable(app->expansion);

    app->rx_stream = furi_stream_buffer_alloc(RX_BUF_SIZE, 1);
    app->worker_thread = furi_thread_alloc_ex("WifiScanRx", 1024, uart_worker, app);
    furi_thread_start(app->worker_thread);

    app->serial_handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    furi_check(app->serial_handle);
    furi_hal_serial_init(app->serial_handle, UART_BAUD);
    furi_hal_serial_dma_rx_start(app->serial_handle, uart_rx_cb, app, false);

    app->event_queue = furi_message_queue_alloc(8, sizeof(AppEvent));
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_cb, app);
    view_port_input_callback_set(app->view_port, input_cb, app->event_queue);
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    send_scan(app);

    AppEvent event;
    bool running = true;
    while(running) {
        FuriStatus status = furi_message_queue_get(app->event_queue, &event, 200);
        if(status == FuriStatusOk && event.type == EventTypeKey) {
            if(event.input.type == InputTypePress || event.input.type == InputTypeRepeat) {
                switch(event.input.key) {
                case InputKeyUp:
                    furi_mutex_acquire(app->mutex, FuriWaitForever);
                    if(app->scroll_pos > 0) app->scroll_pos--;
                    furi_mutex_release(app->mutex);
                    break;
                case InputKeyDown:
                    furi_mutex_acquire(app->mutex, FuriWaitForever);
                    if(app->scroll_pos < app->network_count - VISIBLE_LINES) app->scroll_pos++;
                    furi_mutex_release(app->mutex);
                    break;
                case InputKeyOk:
                    send_scan(app);
                    break;
                case InputKeyBack:
                    running = false;
                    break;
                default:
                    break;
                }
            }
        }
        if(app->scan_state == ScanActive && app->network_count > 0 && status == FuriStatusErrorTimeout) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            app->scan_state = ScanDone;
            furi_mutex_release(app->mutex);
        }
        view_port_update(app->view_port);
    }

    furi_hal_serial_dma_rx_stop(app->serial_handle);
    furi_hal_serial_deinit(app->serial_handle);
    furi_hal_serial_control_release(app->serial_handle);

    furi_thread_flags_set(furi_thread_get_id(app->worker_thread), WORKER_EVT_STOP);
    furi_thread_join(app->worker_thread);
    furi_thread_free(app->worker_thread);
    furi_stream_buffer_free(app->rx_stream);

    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(app->event_queue);
    furi_mutex_free(app->mutex);

    expansion_enable(app->expansion);
    furi_record_close(RECORD_EXPANSION);
    free(app);
    return 0;
}
