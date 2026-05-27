/* GUI Canvas Header - High-Performance Rendering */

#ifndef GUI_CANVAS_H
#define GUI_CANVAS_H

#include <stdint.h>
#include <stdbool.h>
#include <furi.h>

// Color definitions
typedef enum {
    CanvasColorWhite = 0,
    CanvasColorBlack = 1
} canvas_color_t;

// Alignment options
typedef enum {
    AlignLeft = 0,
    AlignCenter = 1,
    AlignRight = 2,
    AlignTop = 0,
    AlignBottom = 2
} Align;

// Font structure
typedef struct {
    const uint8_t* data;
    uint8_t width;
    uint8_t height;
    uint8_t space_width;
    char first_char;
    char last_char;
} font_t;

// Icon structure
typedef struct {
    const uint8_t* data;
    uint16_t width;
    uint16_t height;
} Icon;

// Canvas structure (opaque)
typedef struct canvas canvas_t;

// Canvas management
canvas_t* canvas_alloc(uint16_t width, uint16_t height);
void canvas_free(canvas_t* canvas);

// Drawing state
void canvas_clear(canvas_t* canvas, canvas_color_t color);
void canvas_set_color(canvas_t* canvas, canvas_color_t color);
void canvas_set_font(canvas_t* canvas, font_t* font);

// Basic primitives
void canvas_draw_pixel(canvas_t* canvas, uint16_t x, uint16_t y);
void canvas_draw_line(canvas_t* canvas, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void canvas_draw_circle(canvas_t* canvas, uint16_t cx, uint16_t cy, uint16_t radius);
void canvas_draw_disc(canvas_t* canvas, uint16_t cx, uint16_t cy, uint16_t radius);

// Shapes
void canvas_draw_frame(canvas_t* canvas, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void canvas_draw_box(canvas_t* canvas, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void canvas_draw_triangle(canvas_t* canvas, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3);
void canvas_draw_round_frame(canvas_t* canvas, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius);

// Text rendering
void canvas_draw_str(canvas_t* canvas, uint16_t x, uint16_t y, const char* str);
void canvas_draw_str_aligned(canvas_t* canvas, uint16_t x, uint16_t y, Align horizontal, Align vertical, const char* str);
uint16_t canvas_string_width(canvas_t* canvas, const char* str);

// Graphics rendering
void canvas_draw_icon(canvas_t* canvas, uint16_t x, uint16_t y, const Icon* icon);
void canvas_draw_bitmap(canvas_t* canvas, uint16_t x, uint16_t y, const uint8_t* bitmap, uint16_t width, uint16_t height);

// Built-in fonts
extern font_t font_8x11;
extern font_t font_16x21;
extern font_t font_18x26;
extern font_t font_24x35;

// Common icons
extern Icon I_ButtonCenter;
extern Icon I_ButtonUp;
extern Icon I_ButtonDown;
extern Icon I_ButtonLeft;
extern Icon I_ButtonRight;
extern Icon I_ButtonUpLeft;
extern Icon I_ButtonUpRight;
extern Icon I_ButtonDownLeft;
extern Icon I_ButtonDownRight;
extern Icon I_Ok_btn;
extern Icon I_Back_btn;
extern Icon I_Pinboard_arrow_up_10x8;
extern Icon I_Pinboard_arrow_down_10x8;
extern Icon I_Pinboard_arrow_left_10x8;
extern Icon I_Pinboard_arrow_right_10x8;

#endif // GUI_CANVAS_H
