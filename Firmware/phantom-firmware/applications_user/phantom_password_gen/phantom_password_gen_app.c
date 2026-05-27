#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_random.h>
#include <gui/gui.h>
#include <input/input.h>
#include <string.h>
#include <stdio.h>

#define MAX_PASSWORD_LEN 32

typedef enum { EventTypeKey } EventType;
typedef struct { EventType type; InputEvent input; } AppEvent;

typedef enum {
    ModeAlphaNum,
    ModeFull,
    ModeNumericPin,
    ModeHex,
    ModeCount,
} GenMode;

static const char* mode_names[] = {"AlphaNum", "Full", "PIN", "Hex"};
static const uint8_t length_options[] = {8, 12, 16, 20, 24, 32};
#define LENGTH_OPTION_COUNT 6

static const char charset_alphanum[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static const char charset_full[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:,.<>?";
static const char charset_numeric[] = "0123456789";
static const char charset_hex[] = "0123456789ABCDEF";

typedef struct {
    FuriMutex* mutex;
    GenMode mode;
    uint8_t length_idx;
    char password[MAX_PASSWORD_LEN + 1];
    bool generated;
} AppState;

static void generate_password(AppState* state) {
    const char* charset;
    size_t charset_len;

    switch(state->mode) {
    case ModeAlphaNum:
        charset = charset_alphanum;
        charset_len = strlen(charset_alphanum);
        break;
    case ModeFull:
        charset = charset_full;
        charset_len = strlen(charset_full);
        break;
    case ModeNumericPin:
        charset = charset_numeric;
        charset_len = strlen(charset_numeric);
        break;
    case ModeHex:
        charset = charset_hex;
        charset_len = strlen(charset_hex);
        break;
    default:
        charset = charset_alphanum;
        charset_len = strlen(charset_alphanum);
        break;
    }

    uint8_t len = length_options[state->length_idx];
    uint8_t random_bytes[MAX_PASSWORD_LEN];
    furi_hal_random_fill_buf(random_bytes, len);

    for(uint8_t i = 0; i < len; i++) {
        state->password[i] = charset[random_bytes[i] % charset_len];
    }
    state->password[len] = '\0';
    state->generated = true;
}

static void draw_cb(Canvas* canvas, void* ctx) {
    AppState* state = ctx;
    furi_mutex_acquire(state->mutex, FuriWaitForever);
    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Password Generator");

    canvas_set_font(canvas, FontSecondary);
    char info[40];
    snprintf(info, sizeof(info), "Mode: %s  Len: %d",
        mode_names[state->mode], length_options[state->length_idx]);
    canvas_draw_str_aligned(canvas, 64, 16, AlignCenter, AlignTop, info);

    canvas_draw_line(canvas, 4, 26, 124, 26);

    if(state->generated) {
        canvas_set_font(canvas, FontKeyboard);
        uint8_t len = length_options[state->length_idx];
        if(len <= 16) {
            canvas_draw_str_aligned(canvas, 64, 36, AlignCenter, AlignCenter, state->password);
        } else {
            /* Split into two lines */
            char line1[17], line2[17];
            memcpy(line1, state->password, 16);
            line1[16] = '\0';
            strncpy(line2, state->password + 16, 16);
            line2[16] = '\0';
            canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, line1);
            canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, line2);
        }
    } else {
        canvas_draw_str_aligned(canvas, 64, 36, AlignCenter, AlignCenter, "Press OK to generate");
    }

    canvas_draw_line(canvas, 4, 50, 124, 50);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 58, AlignCenter, AlignCenter, "OK=Gen  LR=Mode  UD=Len");

    furi_mutex_release(state->mutex);
}

static void input_cb(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* q = ctx;
    AppEvent ev = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(q, &ev, FuriWaitForever);
}

int32_t phantom_password_gen_app(void* p) {
    UNUSED(p);

    AppState* state = malloc(sizeof(AppState));
    memset(state, 0, sizeof(AppState));
    state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    state->length_idx = 2; /* default 16 chars */

    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(AppEvent));

    ViewPort* vp = view_port_alloc();
    view_port_draw_callback_set(vp, draw_cb, state);
    view_port_input_callback_set(vp, input_cb, queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, vp, GuiLayerFullscreen);

    AppEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(queue, &event, FuriWaitForever) == FuriStatusOk) {
            if(event.input.type == InputTypePress) {
                furi_mutex_acquire(state->mutex, FuriWaitForever);
                switch(event.input.key) {
                case InputKeyOk:
                    generate_password(state);
                    break;
                case InputKeyLeft:
                    state->mode = (state->mode == 0) ? ModeCount - 1 : state->mode - 1;
                    state->generated = false;
                    break;
                case InputKeyRight:
                    state->mode = (state->mode + 1) % ModeCount;
                    state->generated = false;
                    break;
                case InputKeyUp:
                    if(state->length_idx < LENGTH_OPTION_COUNT - 1) state->length_idx++;
                    state->generated = false;
                    break;
                case InputKeyDown:
                    if(state->length_idx > 0) state->length_idx--;
                    state->generated = false;
                    break;
                case InputKeyBack:
                    running = false;
                    break;
                default:
                    break;
                }
                furi_mutex_release(state->mutex);
                view_port_update(vp);
            }
        }
    }

    view_port_enabled_set(vp, false);
    gui_remove_view_port(gui, vp);
    furi_record_close(RECORD_GUI);
    view_port_free(vp);
    furi_message_queue_free(queue);
    furi_mutex_free(state->mutex);
    free(state);
    return 0;
}
