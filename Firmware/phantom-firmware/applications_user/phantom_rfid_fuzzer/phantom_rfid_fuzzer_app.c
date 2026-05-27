#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_random.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <string.h>
#include <stdio.h>

#define MAX_FILES 9999

typedef enum { EventTypeKey, EventTypeTick } EventType;
typedef struct { EventType type; InputEvent input; } AppEvent;

typedef enum {
    ProtoEM4100,
    ProtoHIDProx,
    ProtoNTAG,
    ProtoMifareClassic,
    ProtoCount,
} Protocol;

typedef enum {
    FuzzSequential,
    FuzzRandom,
    FuzzPattern,
    FuzzModeCount,
} FuzzMode;

static const char* proto_names[] = {"EM4100", "HID Prox", "NTAG/UL", "MF Classic"};
static const char* fuzz_mode_names[] = {"Sequential", "Random", "Pattern"};
static const uint8_t proto_uid_len[] = {5, 3, 7, 4};

/* Common/default UIDs for pattern mode */
static const uint8_t patterns_5[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0x01, 0x02, 0x03, 0x04, 0x05},
    {0xDE, 0xAD, 0xBE, 0xEF, 0x00},
    {0x12, 0x34, 0x56, 0x78, 0x9A},
};
static const uint8_t patterns_4[][4] = {
    {0x00, 0x00, 0x00, 0x00},
    {0xFF, 0xFF, 0xFF, 0xFF},
    {0xDE, 0xAD, 0xBE, 0xEF},
    {0x01, 0x23, 0x45, 0x67},
    {0xCA, 0xFE, 0xBA, 0xBE},
};
#define PATTERN_COUNT 5

typedef struct {
    FuriMutex* mutex;
    Protocol proto;
    FuzzMode fuzz_mode;
    uint8_t current_uid[7];
    uint32_t attempt;
    bool running;
    uint32_t files_written;
} AppState;

static void uid_increment(uint8_t* uid, uint8_t len) {
    for(int i = len - 1; i >= 0; i--) {
        uid[i]++;
        if(uid[i] != 0) break;
    }
}

static void uid_random(uint8_t* uid, uint8_t len) {
    furi_hal_random_fill_buf(uid, len);
}

static void write_rfid_file(Storage* storage, const uint8_t* uid, uint32_t index, const char* proto) {
    char path[64];
    snprintf(path, sizeof(path), "/ext/phantom/rfid_fuzz/%s_%04lu.rfid", proto, (unsigned long)index);

    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        char buf[128];
        int len = snprintf(buf, sizeof(buf),
            "Filetype: Flipper RFID key\nVersion: 1\nKey type: %s\nData: %02X %02X %02X %02X %02X\n",
            proto, uid[0], uid[1], uid[2], uid[3], uid[4]);
        storage_file_write(file, buf, len);
        storage_file_close(file);
    }
    storage_file_free(file);
}

static void write_nfc_file(Storage* storage, const uint8_t* uid, uint8_t uid_len, uint32_t index) {
    char path[64];
    snprintf(path, sizeof(path), "/ext/phantom/nfc_fuzz/nfc_%04lu.nfc", (unsigned long)index);

    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        char buf[256];
        int len;
        if(uid_len == 7) {
            len = snprintf(buf, sizeof(buf),
                "Filetype: Flipper NFC device\nVersion: 4\nDevice type: NTAG/Ultralight\n"
                "UID: %02X %02X %02X %02X %02X %02X %02X\nATQA: 44 00\nSAK: 00\n",
                uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
        } else {
            len = snprintf(buf, sizeof(buf),
                "Filetype: Flipper NFC device\nVersion: 4\nDevice type: Mifare Classic\n"
                "UID: %02X %02X %02X %02X\nATQA: 04 00\nSAK: 08\n",
                uid[0], uid[1], uid[2], uid[3]);
        }
        storage_file_write(file, buf, len);
        storage_file_close(file);
    }
    storage_file_free(file);
}

static void generate_next(AppState* state, Storage* storage) {
    uint8_t uid_len = proto_uid_len[state->proto];

    switch(state->fuzz_mode) {
    case FuzzSequential:
        uid_increment(state->current_uid, uid_len);
        break;
    case FuzzRandom:
        uid_random(state->current_uid, uid_len);
        break;
    case FuzzPattern: {
        uint32_t idx = state->attempt % PATTERN_COUNT;
        if(uid_len == 5)
            memcpy(state->current_uid, patterns_5[idx], 5);
        else if(uid_len == 4)
            memcpy(state->current_uid, patterns_4[idx], 4);
        else {
            memcpy(state->current_uid, patterns_5[idx], 5);
            state->current_uid[5] = (uint8_t)(idx);
            state->current_uid[6] = (uint8_t)(idx + 1);
        }
        break;
    }
    default:
        break;
    }

    if(state->proto == ProtoEM4100 || state->proto == ProtoHIDProx) {
        write_rfid_file(storage, state->current_uid, state->files_written,
            state->proto == ProtoEM4100 ? "EM4100" : "HIDProx");
    } else {
        write_nfc_file(storage, state->current_uid, uid_len, state->files_written);
    }
    state->attempt++;
    state->files_written++;
}

static void draw_cb(Canvas* canvas, void* ctx) {
    AppState* state = ctx;
    furi_mutex_acquire(state->mutex, FuriWaitForever);
    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    char title[32];
    snprintf(title, sizeof(title), "RFID Fuzzer - %s", proto_names[state->proto]);
    canvas_draw_str(canvas, 0, 10, title);

    canvas_set_font(canvas, FontSecondary);
    char mode_str[32];
    snprintf(mode_str, sizeof(mode_str), "Mode: %s", fuzz_mode_names[state->fuzz_mode]);
    canvas_draw_str(canvas, 0, 22, mode_str);

    /* Display current UID */
    uint8_t uid_len = proto_uid_len[state->proto];
    char uid_str[32] = {0};
    size_t off = 0;
    for(uint8_t i = 0; i < uid_len; i++) {
        off += snprintf(uid_str + off, sizeof(uid_str) - off,
            "%s%02X", (i > 0) ? ":" : "", state->current_uid[i]);
    }
    canvas_draw_str(canvas, 0, 33, "UID:");
    canvas_draw_str(canvas, 26, 33, uid_str);

    char attempt_str[32];
    snprintf(attempt_str, sizeof(attempt_str), "Files: %lu", (unsigned long)state->files_written);
    canvas_draw_str(canvas, 0, 44, attempt_str);

    /* Status */
    const char* status = state->running ? "OK=Stop  BACK=Exit" : "OK=Start  LR=Mode  UD=Proto";
    canvas_draw_str_aligned(canvas, 64, 58, AlignCenter, AlignCenter, status);

    if(state->running) {
        /* Simple progress bar */
        uint8_t bar_w = (uint8_t)((state->files_written * 120) / MAX_FILES);
        if(bar_w > 120) bar_w = 120;
        canvas_draw_frame(canvas, 3, 48, 122, 6);
        canvas_draw_box(canvas, 4, 49, bar_w, 4);
    }

    furi_mutex_release(state->mutex);
}

static void input_cb(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* q = ctx;
    AppEvent ev = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(q, &ev, FuriWaitForever);
}

static void tick_cb(void* ctx) {
    FuriMessageQueue* q = ctx;
    AppEvent ev = {.type = EventTypeTick};
    furi_message_queue_put(q, &ev, 0);
}

int32_t phantom_rfid_fuzzer_app(void* p) {
    UNUSED(p);

    AppState* state = malloc(sizeof(AppState));
    memset(state, 0, sizeof(AppState));
    state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, "/ext/phantom");
    storage_simply_mkdir(storage, "/ext/phantom/rfid_fuzz");
    storage_simply_mkdir(storage, "/ext/phantom/nfc_fuzz");

    FuriMessageQueue* queue = furi_message_queue_alloc(16, sizeof(AppEvent));

    ViewPort* vp = view_port_alloc();
    view_port_draw_callback_set(vp, draw_cb, state);
    view_port_input_callback_set(vp, input_cb, queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, vp, GuiLayerFullscreen);

    NotificationApp* notif = furi_record_open(RECORD_NOTIFICATION);

    FuriTimer* timer = furi_timer_alloc(tick_cb, FuriTimerTypePeriodic, queue);

    AppEvent event;
    bool running = true;
    while(running) {
        FuriStatus status = furi_message_queue_get(queue, &event, 200);
        if(status == FuriStatusOk) {
            if(event.type == EventTypeTick && state->running) {
                furi_mutex_acquire(state->mutex, FuriWaitForever);
                if(state->files_written < MAX_FILES) {
                    generate_next(state, storage);
                } else {
                    state->running = false;
                    furi_timer_stop(timer);
                    notification_message(notif, &sequence_success);
                }
                furi_mutex_release(state->mutex);
            } else if(event.type == EventTypeKey && event.input.type == InputTypePress) {
                furi_mutex_acquire(state->mutex, FuriWaitForever);
                if(!state->running) {
                    switch(event.input.key) {
                    case InputKeyOk:
                        state->running = true;
                        state->attempt = 0;
                        state->files_written = 0;
                        memset(state->current_uid, 0, sizeof(state->current_uid));
                        furi_timer_start(timer, furi_ms_to_ticks(50));
                        break;
                    case InputKeyLeft:
                        state->fuzz_mode = (state->fuzz_mode == 0) ? FuzzModeCount - 1 : state->fuzz_mode - 1;
                        break;
                    case InputKeyRight:
                        state->fuzz_mode = (state->fuzz_mode + 1) % FuzzModeCount;
                        break;
                    case InputKeyUp:
                        state->proto = (state->proto == 0) ? ProtoCount - 1 : state->proto - 1;
                        break;
                    case InputKeyDown:
                        state->proto = (state->proto + 1) % ProtoCount;
                        break;
                    case InputKeyBack:
                        running = false;
                        break;
                    default:
                        break;
                    }
                } else {
                    if(event.input.key == InputKeyOk || event.input.key == InputKeyBack) {
                        state->running = false;
                        furi_timer_stop(timer);
                    }
                }
                furi_mutex_release(state->mutex);
            }
        }
        view_port_update(vp);
    }

    furi_timer_free(timer);
    view_port_enabled_set(vp, false);
    gui_remove_view_port(gui, vp);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_STORAGE);
    view_port_free(vp);
    furi_message_queue_free(queue);
    furi_mutex_free(state->mutex);
    free(state);
    return 0;
}
