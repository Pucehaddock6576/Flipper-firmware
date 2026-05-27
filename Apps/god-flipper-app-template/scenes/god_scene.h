#pragma once

#include <gui/scene_manager.h>

#define ADD_SCENE(prefix, name, id) GodScene##id,
typedef enum {
#include "god_scene_config.h"
    GodSceneNum,
} GodScene;
#undef ADD_SCENE

extern const SceneManagerHandlers god_scene_handlers;

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void*);
#include "god_scene_config.h"
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "god_scene_config.h"
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "god_scene_config.h"
#undef ADD_SCENE
