#include "jammer_view.h"

#include <furi.h>
#include <gui/elements.h>

#define FREQ_NAME_MAX 32

struct JammerView {
    View* view;
    JammerViewCallback callback;
    void* callback_context;
};

typedef struct {
    uint32_t frequency;
    char freq_name[FREQ_NAME_MAX];
    bool is_jamming;
} JammerViewModel;

static void jammer_view_draw_callback(Canvas* canvas, void* model) {
    JammerViewModel* m = model;
    
    canvas_clear(canvas);
    
    // Draw header bar
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_box(canvas, 0, 0, canvas_width(canvas), 13);
    canvas_invert_color(canvas);
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Sub-GHz Jammer");
    canvas_invert_color(canvas);
    
    // Draw frequency with arrows
    canvas_set_font(canvas, FontSecondary);
    
    // Up arrow
    canvas_draw_str_aligned(canvas, 64, 16, AlignCenter, AlignTop, "^");
    
    // Frequency display
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(canvas, 64, 24, AlignCenter, AlignTop, m->freq_name);
    
    // Down arrow
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignTop, "v");
    
    // Draw status button (narrower)
    if(m->is_jamming) {
        // Jamming indicator
        canvas_draw_rframe(canvas, 34, 48, 60, 14, 3);
        canvas_draw_rbox(canvas, 35, 49, 58, 12, 2);
        canvas_invert_color(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 50, AlignCenter, AlignTop, "JAMMING");
        canvas_invert_color(canvas);
    } else {
        canvas_draw_rframe(canvas, 34, 48, 60, 14, 3);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 50, AlignCenter, AlignTop, "READY");
    }
    
    // Draw button hints at bottom
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 0, 64, AlignLeft, AlignBottom, "Info");
    canvas_draw_str_aligned(canvas, 128, 64, AlignRight, AlignBottom, "Credits");
}

static bool jammer_view_input_callback(InputEvent* event, void* context) {
    JammerView* jammer_view = context;
    bool consumed = false;
    
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
            case InputKeyUp:
                if(jammer_view->callback) {
                    jammer_view->callback(JammerViewEventFrequencyUp, jammer_view->callback_context);
                }
                consumed = true;
                break;
            case InputKeyDown:
                if(jammer_view->callback) {
                    jammer_view->callback(JammerViewEventFrequencyDown, jammer_view->callback_context);
                }
                consumed = true;
                break;
            case InputKeyOk:
                if(jammer_view->callback) {
                    jammer_view->callback(JammerViewEventStartStop, jammer_view->callback_context);
                }
                consumed = true;
                break;
            case InputKeyLeft:
                if(jammer_view->callback) {
                    jammer_view->callback(JammerViewEventInfo, jammer_view->callback_context);
                }
                consumed = true;
                break;
            case InputKeyRight:
                if(jammer_view->callback) {
                    jammer_view->callback(JammerViewEventCredits, jammer_view->callback_context);
                }
                consumed = true;
                break;
            case InputKeyBack:
                if(jammer_view->callback) {
                    jammer_view->callback(JammerViewEventExit, jammer_view->callback_context);
                }
                consumed = true;
                break;
            default:
                break;
        }
    }
    
    return consumed;
}

JammerView* jammer_view_alloc(void) {
    JammerView* jammer_view = malloc(sizeof(JammerView));
    
    jammer_view->view = view_alloc();
    jammer_view->callback = NULL;
    jammer_view->callback_context = NULL;
    
    view_allocate_model(jammer_view->view, ViewModelTypeLocking, sizeof(JammerViewModel));
    view_set_context(jammer_view->view, jammer_view);
    view_set_draw_callback(jammer_view->view, jammer_view_draw_callback);
    view_set_input_callback(jammer_view->view, jammer_view_input_callback);
    
    with_view_model(
        jammer_view->view,
        JammerViewModel * model,
        {
            model->frequency = 433920000;
            strncpy(model->freq_name, "433.92", FREQ_NAME_MAX);
            model->is_jamming = false;
        },
        true);
    
    return jammer_view;
}

void jammer_view_free(JammerView* jammer_view) {
    furi_assert(jammer_view);
    view_free(jammer_view->view);
    free(jammer_view);
}

View* jammer_view_get_view(JammerView* jammer_view) {
    furi_assert(jammer_view);
    return jammer_view->view;
}

void jammer_view_set_frequency(JammerView* jammer_view, uint32_t frequency) {
    furi_assert(jammer_view);
    with_view_model(
        jammer_view->view,
        JammerViewModel * model,
        { model->frequency = frequency; },
        true);
}

void jammer_view_set_frequency_name(JammerView* jammer_view, const char* name) {
    furi_assert(jammer_view);
    with_view_model(
        jammer_view->view,
        JammerViewModel * model,
        { strncpy(model->freq_name, name, FREQ_NAME_MAX - 1); },
        true);
}

void jammer_view_set_jamming(JammerView* jammer_view, bool is_jamming) {
    furi_assert(jammer_view);
    with_view_model(
        jammer_view->view,
        JammerViewModel * model,
        { model->is_jamming = is_jamming; },
        true);
}

void jammer_view_set_callback(JammerView* jammer_view, JammerViewCallback callback, void* context) {
    furi_assert(jammer_view);
    jammer_view->callback = callback;
    jammer_view->callback_context = context;
}
