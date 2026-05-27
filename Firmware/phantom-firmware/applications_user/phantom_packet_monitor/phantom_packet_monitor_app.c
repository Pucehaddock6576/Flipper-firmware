#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <expansion/expansion.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <string.h>
#include <stdio.h>

#define UART_BAUD      115200
#define RX_BUF_SIZE    2048
#define GRAPH_WIDTH    128
#define GRAPH_HEIGHT   32
#define GRAPH_Y_OFFSET 20

#define WORKER_EVT_RX   (1UL << 0)
#define WORKER_EVT_STOP (1UL << 1)

typedef enum { EventTypeKey, EventTypeTick } EventType;
typedef struct { EventType type; InputEvent input; } AppEvent;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    FuriMutex* mutex;
    FuriTimer* timer;

    FuriHalSerialHandle* serial_handle;
    FuriStreamBuffer* rx_stream;
    FuriThread* worker_thread;
    Expansion* expansion;

    uint8_t graph[GRAPH_WIDTH];
    uint8_t graph_pos;
    uint32_t total_packets;
    uint32_t current_count;
    uint16_t pps;
    uint8_t channel;
    bool capturing;
} App;

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
            uint8_t data[256];
            size_t len;
            do {
                len = furi_stream_buffer_receive(app->rx_stream, data, sizeof(data), 0);
                /* Count newlines as packet indicators */
                for(size_t i = 0; i < len; i++) {
                    if(data[i] == '\n') {
                        __atomic_add_fetch(&app->current_count, 1, __ATOMIC_RELAXED);
                    }
                }
            } while(len > 0);
        }
    }
    return 0;
}

static void timer_cb(void* ctx) {
    FuriMessageQueue* q = ctx;
    AppEvent ev = {.type = EventTypeTick};
    furi_message_queue_put(q, &ev, 0);
}

static void draw_cb(Canvas* canvas, void* ctx) {
    App* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Packet Monitor");

    char buf[32];
    canvas_set_font(canvas, FontSecondary);
    snprintf(buf, sizeof(buf), "Ch:%d  %d pkt/s  Total:%lu",
        app->channel, app->pps, (unsigned long)app->total_packets);
    canvas_draw_str(canvas, 0, 18, buf);

    /* Draw rolling bar graph */
    for(uint8_t i = 0; i < GRAPH_WIDTH; i++) {
        uint8_t idx = (app->graph_pos + i) % GRAPH_WIDTH;
        uint8_t val = app->graph[idx];
        if(val > GRAPH_HEIGHT) val = GRAPH_HEIGHT;
        if(val > 0) {
            canvas_draw_line(canvas,
                i, GRAPH_Y_OFFSET + GRAPH_HEIGHT - val,
                i, GRAPH_Y_OFFSET + GRAPH_HEIGHT);
        }
    }

    /* Graph frame */
    canvas_draw_frame(canvas, 0, GRAPH_Y_OFFSET, GRAPH_WIDTH, GRAPH_HEIGHT + 1);

    /* Bottom status */
    const char* status = app->capturing ? "OK=Stop  <>=Channel" : "OK=Start  BACK=Exit";
    canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, status);

    furi_mutex_release(app->mutex);
}

static void input_cb(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* q = ctx;
    AppEvent ev = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(q, &ev, FuriWaitForever);
}

static void uart_send(App* app, const char* cmd) {
    furi_hal_serial_tx(app->serial_handle, (const uint8_t*)cmd, strlen(cmd));
}

int32_t phantom_packet_monitor_app(void* p) {
    UNUSED(p);
    App* app = malloc(sizeof(App));
    memset(app, 0, sizeof(App));
    app->channel = 1;
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    app->expansion = furi_record_open(RECORD_EXPANSION);
    expansion_disable(app->expansion);

    app->rx_stream = furi_stream_buffer_alloc(RX_BUF_SIZE, 1);
    app->worker_thread = furi_thread_alloc_ex("PktMonRx", 1024, uart_worker, app);
    furi_thread_start(app->worker_thread);

    app->serial_handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    furi_check(app->serial_handle);
    furi_hal_serial_init(app->serial_handle, UART_BAUD);
    furi_hal_serial_dma_rx_start(app->serial_handle, uart_rx_cb, app, false);

    app->event_queue = furi_message_queue_alloc(16, sizeof(AppEvent));
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_cb, app);
    view_port_input_callback_set(app->view_port, input_cb, app->event_queue);
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    NotificationApp* notif = furi_record_open(RECORD_NOTIFICATION);
    notification_message_block(notif, &sequence_display_backlight_enforce_on);

    /* Timer fires every 100ms to update the graph */
    app->timer = furi_timer_alloc(timer_cb, FuriTimerTypePeriodic, app->event_queue);
    furi_timer_start(app->timer, furi_ms_to_ticks(100));

    AppEvent event;
    bool running = true;
    while(running) {
        FuriStatus status = furi_message_queue_get(app->event_queue, &event, 200);
        if(status == FuriStatusOk) {
            if(event.type == EventTypeTick) {
                furi_mutex_acquire(app->mutex, FuriWaitForever);
                uint32_t count = __atomic_exchange_n(&app->current_count, 0, __ATOMIC_RELAXED);
                app->total_packets += count;
                app->pps = count * 10; /* 100ms interval -> *10 for per-second */
                uint8_t bar = (count > GRAPH_HEIGHT) ? GRAPH_HEIGHT : (uint8_t)count;
                app->graph[app->graph_pos] = bar;
                app->graph_pos = (app->graph_pos + 1) % GRAPH_WIDTH;
                furi_mutex_release(app->mutex);
            } else if(event.type == EventTypeKey && event.input.type == InputTypePress) {
                switch(event.input.key) {
                case InputKeyOk:
                    if(!app->capturing) {
                        uart_send(app, "sniffraw\n");
                        app->capturing = true;
                    } else {
                        uart_send(app, "stopscan\n");
                        app->capturing = false;
                    }
                    break;
                case InputKeyLeft:
                    if(app->channel > 1) {
                        app->channel--;
                        char cmd[20];
                        snprintf(cmd, sizeof(cmd), "channel %d\n", app->channel);
                        uart_send(app, cmd);
                    }
                    break;
                case InputKeyRight:
                    if(app->channel < 14) {
                        app->channel++;
                        char cmd[20];
                        snprintf(cmd, sizeof(cmd), "channel %d\n", app->channel);
                        uart_send(app, cmd);
                    }
                    break;
                case InputKeyBack:
                    if(app->capturing) uart_send(app, "stopscan\n");
                    running = false;
                    break;
                default:
                    break;
                }
            }
        }
        view_port_update(app->view_port);
    }

    notification_message(notif, &sequence_display_backlight_enforce_auto);
    furi_timer_free(app->timer);

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
    furi_record_close(RECORD_NOTIFICATION);
    furi_message_queue_free(app->event_queue);
    furi_mutex_free(app->mutex);

    expansion_enable(app->expansion);
    furi_record_close(RECORD_EXPANSION);
    free(app);
    return 0;
}
