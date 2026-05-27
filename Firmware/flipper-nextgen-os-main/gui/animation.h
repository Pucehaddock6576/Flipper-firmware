/* Animation System Header - 60fps Smooth Transitions */

#ifndef GUI_ANIMATION_H
#define GUI_ANIMATION_H

#include <stdint.h>
#include <stdbool.h>
#include <furi.h>
#include "canvas.h"

// Forward declarations
typedef struct animation animation_t;

// Animation constants
#define MAX_ANIMATIONS 32
#define M_PI 3.14159265358979323846f

// Easing function types
typedef float (*easing_function_t)(float t);

// Animation callbacks
typedef void (*animation_update_callback_t)(animation_t* animation, float progress);
typedef void (*animation_completion_callback_t)(animation_t* animation);

// Animation structure
typedef struct animation {
    uint32_t duration;
    uint32_t start_time;
    easing_function_t easing_function;
    bool loop;
    animation_update_callback_t update_callback;
    animation_completion_callback_t completion_callback;
    void* context;
} animation_t;

// Animation manager
typedef struct animation_manager_s {
    animation_t** animations;
    uint32_t animation_count;
    uint32_t last_update_time;
} animation_manager_s;

// Easing functions
float ease_linear(float t);
float ease_in_quad(float t);
float ease_out_quad(float t);
float ease_in_out_quad(float t);
float ease_in_cubic(float t);
float ease_out_cubic(float t);
float ease_in_out_cubic(float t);
float ease_in_sine(float t);
float ease_out_sine(float t);
float ease_in_out_sine(float t);
float ease_bounce(float t);
float ease_elastic(float t);

// Animation manager
animation_manager_s* animation_manager_alloc(void);
void animation_manager_free(animation_manager_s* manager);
bool animation_manager_add(animation_manager_s* manager, animation_t* animation);
void animation_manager_remove(animation_manager_s* manager, animation_t* animation);
void animation_manager_update(animation_manager_s* manager);
uint32_t animation_manager_get_count(animation_manager_s* manager);

// Global animation manager
animation_manager_s* animation_manager_get_global(void);
void animation_manager_update_global(void);

// Animation creation and management
animation_t* animation_alloc(uint32_t duration);
void animation_free(animation_t* animation);
void animation_set_easing(animation_t* animation, easing_function_t easing);
void animation_set_loop(animation_t* animation, bool loop);
void animation_set_update_callback(animation_t* animation, animation_update_callback_t callback, void* context);
void animation_set_completion_callback(animation_t* animation, animation_completion_callback_t callback);

// Property animations
animation_t* animation_property_float(float* property, float start_value, float end_value, uint32_t duration);
animation_t* animation_position(uint16_t* x, uint16_t* y, uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y, uint32_t duration);
animation_t* animation_color(canvas_color_t* color, canvas_color_t start_color, canvas_color_t end_color, uint32_t duration);

// Common animation presets
static inline animation_t* animation_fade_in(uint32_t duration) {
    static float opacity = 0.0f;
    return animation_property_float(&opacity, 0.0f, 1.0f, duration);
}

static inline animation_t* animation_fade_out(uint32_t duration) {
    static float opacity = 1.0f;
    return animation_property_float(&opacity, 1.0f, 0.0f, duration);
}

static inline animation_t* animation_slide_in_left(uint16_t* x, uint16_t target_x, uint32_t duration) {
    return animation_position(x, NULL, -128, 0, target_x, 0, duration);
}

static inline animation_t* animation_slide_in_right(uint16_t* x, uint16_t target_x, uint32_t duration) {
    return animation_position(x, NULL, 128, 0, target_x, 0, duration);
}

static inline animation_t* animation_slide_in_up(uint16_t* y, uint16_t target_y, uint32_t duration) {
    return animation_position(NULL, y, 0, -64, 0, target_y, duration);
}

static inline animation_t* animation_slide_in_down(uint16_t* y, uint16_t target_y, uint32_t duration) {
    return animation_position(NULL, y, 0, 64, 0, target_y, duration);
}

static inline animation_t* animation_scale(float* scale, float start_scale, float end_scale, uint32_t duration) {
    return animation_property_float(scale, start_scale, end_scale, duration);
}

static inline animation_t* animation_rotate(float* angle, float start_angle, float end_angle, uint32_t duration) {
    return animation_property_float(angle, start_angle, end_angle, duration);
}

#endif // GUI_ANIMATION_H
