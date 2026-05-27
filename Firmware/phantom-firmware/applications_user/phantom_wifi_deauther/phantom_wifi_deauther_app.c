#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>
#include <expansion/expansion.h>

#define UART_CH             FuriHalSerialIdUsart
#define UART_BAUD           115200
#define RX_BUF_SIZE         2048
#define TEXT_BOX_STORE_SIZE 4096

#define WORKER_EVT_RX   (1UL << 0)
#define WORKER_EVT_STOP (1UL << 1)

typedef enum {
    ViewIdSubmenu,
    ViewIdTextBox,
} ViewId;

typedef enum {
    IdxScanAPs,
    IdxScanStations,
    IdxSelectAll,
    IdxDeauthAttack,
    IdxBeaconAttack,
    IdxProbeAttack,
    IdxStopAttack,
    IdxShowResults,
    IdxReboot,
} SubmenuIdx;

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    TextBox* text_box;
    Expansion* expansion;
    FuriHalSerialHandle* serial_handle;
    FuriStreamBuffer* rx_stream;
    FuriThread* worker_thread;
    FuriString* text_buf;
} App;

static void uart_send(App* app, const char* cmd) {
    furi_hal_serial_tx(app->serial_handle, (const uint8_t*)cmd, strlen(cmd));
}

static void uart_rx_cb(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent ev,
    size_t size,
    void* context) {
    App* app = context;
    if(ev & (FuriHalSerialRxEventData | FuriHalSerialRxEventIdle)) {
        uint8_t data[FURI_HAL_SERIAL_DMA_BUFFER_SIZE];
        while(size) {
            size_t ret = furi_hal_serial_dma_rx(
                handle, data, (size > FURI_HAL_SERIAL_DMA_BUFFER_SIZE) ? FURI_HAL_SERIAL_DMA_BUFFER_SIZE : size);
            furi_stream_buffer_send(app->rx_stream, data, ret, 0);
            size -= ret;
        }
        furi_thread_flags_set(furi_thread_get_id(app->worker_thread), WORKER_EVT_RX);
    }
}

static int32_t uart_worker(void* context) {
    App* app = context;
    uint8_t buf[256];

    while(true) {
        uint32_t events = furi_thread_flags_wait(
            WORKER_EVT_RX | WORKER_EVT_STOP, FuriFlagWaitAny, FuriWaitForever);
        furi_check(!(events & FuriFlagError));
        if(events & WORKER_EVT_STOP) break;

        if(events & WORKER_EVT_RX) {
            size_t len;
            do {
                len = furi_stream_buffer_receive(app->rx_stream, buf, sizeof(buf), 0);
                if(len > 0) {
                    for(size_t i = 0; i < len; i++) {
                        char c = (char)buf[i];
                        if(c == '\r') continue;
                        if((c >= ' ' && c <= '~') || c == '\n') {
                            furi_string_push_back(app->text_buf, c);
                        }
                    }
                    if(furi_string_size(app->text_buf) > TEXT_BOX_STORE_SIZE) {
                        furi_string_right(app->text_buf, furi_string_size(app->text_buf) - TEXT_BOX_STORE_SIZE);
                    }
                    text_box_set_text(app->text_box, furi_string_get_cstr(app->text_buf));
                    text_box_set_focus(app->text_box, TextBoxFocusEnd);
                }
            } while(len > 0);
        }
    }
    return 0;
}

static void send_and_show(App* app, const char* cmd) {
    furi_string_reset(app->text_buf);
    furi_string_cat_printf(app->text_buf, "> %s\n", cmd);
    text_box_set_text(app->text_box, furi_string_get_cstr(app->text_buf));
    uart_send(app, cmd);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIdTextBox);
}

static void submenu_cb(void* context, uint32_t index) {
    App* app = context;
    switch(index) {
    case IdxScanAPs:
        send_and_show(app, "scan -m ap\n");
        break;
    case IdxScanStations:
        send_and_show(app, "scan -m st\n");
        break;
    case IdxSelectAll:
        send_and_show(app, "select all\n");
        break;
    case IdxDeauthAttack:
        send_and_show(app, "attack -m deauth\n");
        break;
    case IdxBeaconAttack:
        send_and_show(app, "attack -m beacon\n");
        break;
    case IdxProbeAttack:
        send_and_show(app, "attack -m probe\n");
        break;
    case IdxStopAttack:
        send_and_show(app, "stop\n");
        break;
    case IdxShowResults:
        send_and_show(app, "show selected\n");
        break;
    case IdxReboot:
        send_and_show(app, "reboot\n");
        break;
    }
}

static uint32_t textbox_prev_cb(void* context) {
    UNUSED(context);
    return ViewIdSubmenu;
}

static uint32_t submenu_prev_cb(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static App* app_alloc(void) {
    App* app = malloc(sizeof(App));
    app->text_buf = furi_string_alloc();

    app->expansion = furi_record_open(RECORD_EXPANSION);
    expansion_disable(app->expansion);

    app->serial_handle = furi_hal_serial_control_acquire(UART_CH);
    furi_check(app->serial_handle);
    furi_hal_serial_init(app->serial_handle, UART_BAUD);

    app->rx_stream = furi_stream_buffer_alloc(RX_BUF_SIZE, 1);
    app->worker_thread = furi_thread_alloc_ex("DeautherRx", 1024, uart_worker, app);
    furi_thread_start(app->worker_thread);
    furi_hal_serial_dma_rx_start(app->serial_handle, uart_rx_cb, app, false);

    app->gui = furi_record_open(RECORD_GUI);

    app->submenu = submenu_alloc();
    submenu_set_header(app->submenu, "WiFi Deauther");
    submenu_add_item(app->submenu, "Scan APs", IdxScanAPs, submenu_cb, app);
    submenu_add_item(app->submenu, "Scan Stations", IdxScanStations, submenu_cb, app);
    submenu_add_item(app->submenu, "Select All APs", IdxSelectAll, submenu_cb, app);
    submenu_add_item(app->submenu, "Deauth Attack", IdxDeauthAttack, submenu_cb, app);
    submenu_add_item(app->submenu, "Beacon Attack", IdxBeaconAttack, submenu_cb, app);
    submenu_add_item(app->submenu, "Probe Attack", IdxProbeAttack, submenu_cb, app);
    submenu_add_item(app->submenu, "Stop Attack", IdxStopAttack, submenu_cb, app);
    submenu_add_item(app->submenu, "Show Results", IdxShowResults, submenu_cb, app);
    submenu_add_item(app->submenu, "Reboot", IdxReboot, submenu_cb, app);
    view_set_previous_callback(submenu_get_view(app->submenu), submenu_prev_cb);

    app->text_box = text_box_alloc();
    text_box_set_font(app->text_box, TextBoxFontText);
    text_box_set_focus(app->text_box, TextBoxFocusEnd);
    view_set_previous_callback(text_box_get_view(app->text_box), textbox_prev_cb);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_add_view(app->view_dispatcher, ViewIdSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_add_view(app->view_dispatcher, ViewIdTextBox, text_box_get_view(app->text_box));
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    return app;
}

static void app_free(App* app) {
    uart_send(app, "stop\n");
    furi_delay_ms(50);

    furi_hal_serial_dma_rx_stop(app->serial_handle);
    furi_thread_flags_set(furi_thread_get_id(app->worker_thread), WORKER_EVT_STOP);
    furi_thread_join(app->worker_thread);
    furi_thread_free(app->worker_thread);

    furi_stream_buffer_free(app->rx_stream);
    furi_hal_serial_deinit(app->serial_handle);
    furi_hal_serial_control_release(app->serial_handle);

    view_dispatcher_remove_view(app->view_dispatcher, ViewIdSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIdTextBox);
    view_dispatcher_free(app->view_dispatcher);
    submenu_free(app->submenu);
    text_box_free(app->text_box);

    furi_record_close(RECORD_GUI);
    furi_string_free(app->text_buf);

    expansion_enable(app->expansion);
    furi_record_close(RECORD_EXPANSION);
    free(app);
}

int32_t phantom_wifi_deauther_app(void* p) {
    UNUSED(p);
    App* app = app_alloc();
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIdSubmenu);
    view_dispatcher_run(app->view_dispatcher);
    app_free(app);
    return 0;
}
