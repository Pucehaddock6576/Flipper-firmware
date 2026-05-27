#include "god_app_i.h"
#include <furi.h>

static bool god_app_custom_event_callback(void* context, uint32_t event) {
    GodApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool god_app_back_event_callback(void* context) {
    GodApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

GodApp* god_app_alloc(void) {
    GodApp* app = malloc(sizeof(GodApp));
    memset(app, 0, sizeof(GodApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&god_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, god_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, god_app_back_event_callback);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, GodViewWidget, widget_get_view(app->widget));

    god_seed_from_hal(app);
    god_load_words(app);

    scene_manager_next_scene(app->scene_manager, GodSceneVerse);
    return app;
}

void god_app_free(GodApp* app) {
    furi_assert(app);
    view_dispatcher_remove_view(app->view_dispatcher, GodViewWidget);
    widget_free(app->widget);
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    if(app->word_offsets) free(app->word_offsets);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
    free(app);
}

int32_t god_app(void* p) {
    UNUSED(p);
    GodApp* app = god_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    god_app_free(app);
    return 0;
}
