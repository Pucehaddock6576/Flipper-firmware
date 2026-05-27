/* 60fps Animation Engine - Smooth Transitions & Effects */

#include <gui/animation.h>
#include <furi.h>
#include <math.h>
#include <string.h>

// Animation system
static animation_manager_s* g_animation_manager = NULL;

// Easing functions
float ease_linear(float t) {
    return t;
}

float ease_in_quad(float t) {
    return t * t;
}

float ease_out_quad(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

float ease_in_out_quad(float t) {
    return t < 0.5f ? 2.0f * t * t : 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
}

float ease_in_cubic(float t) {
    return t * t * t;
}

float ease_out_cubic(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
}

float ease_in_out_cubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - 4.0f * (1.0f - t) * (1.0f - t) * (1.0f - t);
}

float ease_in_sine(float t) {
    return 1.0f - cosf((t * M_PI) / 2.0f);
}

float ease_out_sine(float t) {
    return sinf((t * M_PI) / 2.0f);
}

float ease_in_out_sine(float t) {
    return -(cosf(M_PI * t) - 1.0f) / 2.0f;
}

float ease_bounce(float t) {
    if(t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if(t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if(t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    }
}

float ease_elastic(float t) {
    if(t == 0.0f || t == 1.0f) return t;
    return -powf(2.0f, 10.0f * (t - 1.0f)) * sinf((t - 1.1f) * (2.0f * M_PI) / 0.4f);
}

// Animation manager
animation_manager_s* animation_manager_alloc(void) {
    animation_manager_s* manager = furi_alloc(sizeof(animation_manager_s));
    if(!manager) return NULL;
    
    manager->animations = furi_alloc(sizeof(animation_t*) * MAX_ANIMATIONS);
    manager->animation_count = 0;
    manager->last_update_time = furi_get_tick();
    
    for(uint32_t i = 0; i < MAX_ANIMATIONS; i++) {
        manager->animations[i] = NULL;
    }
    
    return manager;
}

void animation_manager_free(animation_manager_s* manager) {
    if(!manager) return;
    
    // Stop all animations
    for(uint32_t i = 0; i < MAX_ANIMATIONS; i++) {
        if(manager->animations[i]) {
            animation_free(manager->animations[i]);
        }
    }
    
    furi_free(manager->animations);
    furi_free(manager);
}

bool animation_manager_add(animation_manager_s* manager, animation_t* animation) {
    if(!manager || !animation || manager->animation_count >= MAX_ANIMATIONS) {
        return false;
    }
    
    // Find empty slot
    for(uint32_t i = 0; i < MAX_ANIMATIONS; i++) {
        if(!manager->animations[i]) {
            manager->animations[i] = animation;
            manager->animation_count++;
            animation->start_time = furi_get_tick();
            return true;
        }
    }
    
    return false;
}

void animation_manager_remove(animation_manager_s* manager, animation_t* animation) {
    if(!manager || !animation) return;
    
    for(uint32_t i = 0; i < MAX_ANIMATIONS; i++) {
        if(manager->animations[i] == animation) {
            manager->animations[i] = NULL;
            manager->animation_count--;
            break;
        }
    }
}

void animation_manager_update(animation_manager_s* manager) {
    if(!manager) return;
    
    uint32_t current_time = furi_get_tick();
    uint32_t delta_time = current_time - manager->last_update_time;
    manager->last_update_time = current_time;
    
    // Update all animations
    for(uint32_t i = 0; i < MAX_ANIMATIONS; i++) {
        animation_t* animation = manager->animations[i];
        if(!animation) continue;
        
        uint32_t elapsed = current_time - animation->start_time;
        
        if(elapsed >= animation->duration) {
            // Animation complete
            if(animation->update_callback) {
                animation->update_callback(animation, 1.0f);
            }
            
            if(animation->completion_callback) {
                animation->completion_callback(animation);
            }
            
            // Handle looping
            if(animation->loop) {
                animation->start_time = current_time;
            } else {
                animation_manager_remove(manager, animation);
                animation_free(animation);
            }
        } else {
            // Update animation progress
            float progress = (float)elapsed / (float)animation->duration;
            float eased_progress = animation->easing_function(progress);
            
            if(animation->update_callback) {
                animation->update_callback(animation, eased_progress);
            }
        }
    }
}

uint32_t animation_manager_get_count(animation_manager_s* manager) {
    return manager ? manager->animation_count : 0;
}

// Animation creation
animation_t* animation_alloc(uint32_t duration) {
    animation_t* animation = furi_alloc(sizeof(animation_t));
    if(!animation) return NULL;
    
    memset(animation, 0, sizeof(animation_t));
    animation->duration = duration;
    animation->easing_function = ease_linear;
    animation->loop = false;
    
    return animation;
}

void animation_free(animation_t* animation) {
    if(animation) {
        furi_free(animation);
    }
}

void animation_set_easing(animation_t* animation, easing_function_t easing) {
    if(animation) {
        animation->easing_function = easing;
    }
}

void animation_set_loop(animation_t* animation, bool loop) {
    if(animation) {
        animation->loop = loop;
    }
}

void animation_set_update_callback(animation_t* animation, animation_update_callback_t callback, void* context) {
    if(animation) {
        animation->update_callback = callback;
        animation->context = context;
    }
}

void animation_set_completion_callback(animation_t* animation, animation_completion_callback_t callback) {
    if(animation) {
        animation->completion_callback = callback;
    }
}

// Property animations
typedef struct {
    float* property;
    float start_value;
    float end_value;
} property_animation_context_t;

static void property_animation_update(animation_t* animation, float progress) {
    property_animation_context_t* ctx = (property_animation_context_t*)animation->context;
    *ctx->property = ctx->start_value + (ctx->end_value - ctx->start_value) * progress;
}

animation_t* animation_property_float(float* property, float start_value, float end_value, uint32_t duration) {
    animation_t* animation = animation_alloc(duration);
    if(!animation) return NULL;
    
    property_animation_context_t* ctx = furi_alloc(sizeof(property_animation_context_t));
    ctx->property = property;
    ctx->start_value = start_value;
    ctx->end_value = end_value;
    
    animation_set_update_callback(animation, property_animation_update, ctx);
    return animation;
}

// Position animations
typedef struct {
    uint16_t* x;
    uint16_t* y;
    uint16_t start_x;
    uint16_t start_y;
    uint16_t end_x;
    uint16_t end_y;
} position_animation_context_t;

static void position_animation_update(animation_t* animation, float progress) {
    position_animation_context_t* ctx = (position_animation_context_t*)animation->context;
    *ctx->x = ctx->start_x + (uint16_t)((ctx->end_x - ctx->start_x) * progress);
    *ctx->y = ctx->start_y + (uint16_t)((ctx->end_y - ctx->start_y) * progress);
}

animation_t* animation_position(uint16_t* x, uint16_t* y, uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y, uint32_t duration) {
    animation_t* animation = animation_alloc(duration);
    if(!animation) return NULL;
    
    position_animation_context_t* ctx = furi_alloc(sizeof(position_animation_context_t));
    ctx->x = x;
    ctx->y = y;
    ctx->start_x = start_x;
    ctx->start_y = start_y;
    ctx->end_x = end_x;
    ctx->end_y = end_y;
    
    animation_set_update_callback(animation, position_animation_update, ctx);
    return animation;
}

// Color animations
typedef struct {
    canvas_color_t* color;
    canvas_color_t start_color;
    canvas_color_t end_color;
} color_animation_context_t;

static void color_animation_update(animation_t* animation, float progress) {
    color_animation_context_t* ctx = (color_animation_context_t*)animation->context;
    // Simple interpolation between black and white
    if(ctx->start_color == CanvasColorWhite && ctx->end_color == CanvasColorBlack) {
        *ctx->color = progress > 0.5f ? CanvasColorBlack : CanvasColorWhite;
    } else if(ctx->start_color == CanvasColorBlack && ctx->end_color == CanvasColorWhite) {
        *ctx->color = progress > 0.5f ? CanvasColorWhite : CanvasColorBlack;
    }
}

animation_t* animation_color(canvas_color_t* color, canvas_color_t start_color, canvas_color_t end_color, uint32_t duration) {
    animation_t* animation = animation_alloc(duration);
    if(!animation) return NULL;
    
    color_animation_context_t* ctx = furi_alloc(sizeof(color_animation_context_t));
    ctx->color = color;
    ctx->start_color = start_color;
    ctx->end_color = end_color;
    
    animation_set_update_callback(animation, color_animation_update, ctx);
    return animation;
}

// Global animation manager access
animation_manager_s* animation_manager_get_global(void) {
    if(!g_animation_manager) {
        g_animation_manager = animation_manager_alloc();
    }
    return g_animation_manager;
}

void animation_manager_update_global(void) {
    if(g_animation_manager) {
        animation_manager_update(g_animation_manager);
    }
}
