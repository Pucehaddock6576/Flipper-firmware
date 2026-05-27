#include "../god_app_i.h"

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const god_scene_on_enter_handlers[])(void*) = {
#include "god_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const god_scene_on_event_handlers[])(void* context, SceneManagerEvent event) = {
#include "god_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const god_scene_on_exit_handlers[])(void* context) = {
#include "god_scene_config.h"
};
#undef ADD_SCENE

const SceneManagerHandlers god_scene_handlers = {
    .on_enter_handlers = god_scene_on_enter_handlers,
    .on_event_handlers = god_scene_on_event_handlers,
    .on_exit_handlers = god_scene_on_exit_handlers,
    .scene_num = GodSceneNum,
};
