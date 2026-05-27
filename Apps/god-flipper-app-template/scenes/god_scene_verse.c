#include "../god_app_i.h"

#define GOD_WORDS_PER_SCREEN 15

void god_scene_verse_on_enter(void* context) {
    GodApp* app = context;
    FuriString* display = furi_string_alloc();

    god_add_entropy_from_timer(app);

    god_get_random_words(app, display, GOD_WORDS_PER_SCREEN);
    if(furi_string_size(display) == 0) {
        furi_string_printf(display, "\e#No dict\e!\n\nkjv.txt word list\nmissing or empty.");
    }

    widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, furi_string_get_cstr(display));
    view_dispatcher_switch_to_view(app->view_dispatcher, GodViewWidget);

    furi_string_free(display);
}

bool god_scene_verse_on_event(void* context, SceneManagerEvent event) {
    GodApp* app = context;
    if(event.type == SceneManagerEventTypeBack) {
        return false;
    }
    if(event.type == SceneManagerEventTypeCustom) {
        return false;
    }
    UNUSED(app);
    return false;
}

void god_scene_verse_on_exit(void* context) {
    GodApp* app = context;
    widget_reset(app->widget);
}
