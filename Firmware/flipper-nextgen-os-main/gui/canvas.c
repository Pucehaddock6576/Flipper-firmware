/* Next-Gen GUI System - 60fps Rendering Engine */

#include <furi.h>
#include <gui/canvas.h>
#include <gui/view_port.h>
#include <gui/animation.h>
#include <string.h>
#include <math.h>

// Canvas implementation
struct canvas {
    uint8_t* buffer;
    uint16_t width;
    uint16_t height;
    uint16_t stride;
    canvas_color_t fg_color;
    canvas_color_t bg_color;
    font_t* current_font;
};

canvas_t* canvas_alloc(uint16_t width, uint16_t height) {
    canvas_t* canvas = furi_alloc(sizeof(canvas_t));
    if(!canvas) return NULL;
    
    canvas->width = width;
    canvas->height = height;
    canvas->stride = (width + 7) / 8;
    canvas->buffer = furi_alloc(canvas->stride * height);
    canvas->fg_color = CanvasColorBlack;
    canvas->bg_color = CanvasColorWhite;
    canvas->current_font = &font_8x11;
    
    if(!canvas->buffer) {
        furi_free(canvas);
        return NULL;
    }
    
    canvas_clear(canvas, canvas->bg_color);
    return canvas;
}

void canvas_free(canvas_t* canvas) {
    if(canvas) {
        furi_free(canvas->buffer);
        furi_free(canvas);
    }
}

void canvas_clear(canvas_t* canvas, canvas_color_t color) {
    if(!canvas) return;
    
    memset(canvas->buffer, color == CanvasColorWhite ? 0xFF : 0x00, 
           canvas->stride * canvas->height);
}

void canvas_set_color(canvas_t* canvas, canvas_color_t color) {
    if(canvas) canvas->fg_color = color;
}

void canvas_set_font(canvas_t* canvas, font_t* font) {
    if(canvas && font) canvas->current_font = font;
}

// Drawing primitives
void canvas_draw_pixel(canvas_t* canvas, uint16_t x, uint16_t y) {
    if(!canvas || x >= canvas->width || y >= canvas->height) return;
    
    uint16_t byte_offset = (y * canvas->stride) + (x / 8);
    uint8_t bit_mask = 1 << (7 - (x % 8));
    
    if(canvas->fg_color == CanvasColorBlack) {
        canvas->buffer[byte_offset] &= ~bit_mask;
    } else {
        canvas->buffer[byte_offset] |= bit_mask;
    }
}

void canvas_draw_line(canvas_t* canvas, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    if(!canvas) return;
    
    int16_t dx = abs(x2 - x1);
    int16_t dy = abs(y2 - y1);
    int16_t sx = x1 < x2 ? 1 : -1;
    int16_t sy = y1 < y2 ? 1 : -1;
    int16_t err = dx - dy;
    
    while(1) {
        canvas_draw_pixel(canvas, x1, y1);
        
        if(x1 == x2 && y1 == y2) break;
        
        int16_t e2 = 2 * err;
        if(e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if(e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void canvas_draw_circle(canvas_t* canvas, uint16_t cx, uint16_t cy, uint16_t radius) {
    if(!canvas) return;
    
    int16_t x = radius;
    int16_t y = 0;
    int16_t err = 0;
    
    while(x >= y) {
        canvas_draw_pixel(canvas, cx + x, cy + y);
        canvas_draw_pixel(canvas, cx + y, cy + x);
        canvas_draw_pixel(canvas, cx - y, cy + x);
        canvas_draw_pixel(canvas, cx - x, cy + y);
        canvas_draw_pixel(canvas, cx - x, cy - y);
        canvas_draw_pixel(canvas, cx - y, cy - x);
        canvas_draw_pixel(canvas, cx + y, cy - x);
        canvas_draw_pixel(canvas, cx + x, cy - y);
        
        if(err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if(err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void canvas_draw_frame(canvas_t* canvas, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if(!canvas) return;
    
    canvas_draw_line(canvas, x, y, x + width - 1, y);
    canvas_draw_line(canvas, x + width - 1, y, x + width - 1, y + height - 1);
    canvas_draw_line(canvas, x + width - 1, y + height - 1, x, y + height - 1);
    canvas_draw_line(canvas, x, y + height - 1, x, y);
}

void canvas_draw_box(canvas_t* canvas, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if(!canvas) return;
    
    for(uint16_t i = 0; i < height; i++) {
        canvas_draw_line(canvas, x, y + i, x + width - 1, y + i);
    }
}

// Text rendering
void canvas_draw_str(canvas_t* canvas, uint16_t x, uint16_t y, const char* str) {
    if(!canvas || !str || !canvas->current_font) return;
    
    font_t* font = canvas->current_font;
    uint16_t current_x = x;
    
    while(*str) {
        char c = *str++;
        if(c == ' ') {
            current_x += font->space_width;
            continue;
        }
        
        if(c >= font->first_char && c <= font->last_char) {
            uint16_t char_index = c - font->first_char;
            const uint8_t* char_data = &font->data[char_index * font->height];
            
            for(uint8_t row = 0; row < font->height; row++) {
                uint8_t byte = char_data[row];
                for(uint8_t col = 0; col < font->width; col++) {
                    if(byte & (0x80 >> col)) {
                        canvas_draw_pixel(canvas, current_x + col, y + row);
                    }
                }
            }
        }
        
        current_x += font->width + 1;
    }
}

void canvas_draw_str_aligned(canvas_t* canvas, uint16_t x, uint16_t y, Align horizontal, Align vertical, const char* str) {
    if(!canvas || !str || !canvas->current_font) return;
    
    font_t* font = canvas->current_font;
    uint16_t str_width = canvas_string_width(canvas, str);
    uint16_t str_height = font->height;
    
    // Adjust horizontal position
    switch(horizontal) {
        case AlignCenter:
            x = x > str_width / 2 ? x - str_width / 2 : 0;
            break;
        case AlignRight:
            x = x > str_width ? x - str_width : 0;
            break;
        case AlignLeft:
        default:
            break;
    }
    
    // Adjust vertical position
    switch(vertical) {
        case AlignCenter:
            y = y > str_height / 2 ? y - str_height / 2 : 0;
            break;
        case AlignBottom:
            y = y > str_height ? y - str_height : 0;
            break;
        case AlignTop:
        default:
            break;
    }
    
    canvas_draw_str(canvas, x, y, str);
}

uint16_t canvas_string_width(canvas_t* canvas, const char* str) {
    if(!canvas || !str || !canvas->current_font) return 0;
    
    font_t* font = canvas->current_font;
    uint16_t width = 0;
    
    while(*str) {
        char c = *str++;
        if(c == ' ') {
            width += font->space_width;
        } else if(c >= font->first_char && c <= font->last_char) {
            width += font->width + 1;
        }
    }
    
    return width > 0 ? width - 1 : 0; // Remove last space
}

// Icon rendering
void canvas_draw_icon(canvas_t* canvas, uint16_t x, uint16_t y, const Icon* icon) {
    if(!canvas || !icon) return;
    
    for(uint16_t row = 0; row < icon->height; row++) {
        for(uint16_t col = 0; col < icon->width; col++) {
            uint16_t byte_index = (row * icon->width + col) / 8;
            uint8_t bit_index = 7 - ((row * icon->width + col) % 8);
            
            if(icon->data[byte_index] & (1 << bit_index)) {
                canvas_draw_pixel(canvas, x + col, y + row);
            }
        }
    }
}

// Bitmap rendering
void canvas_draw_bitmap(canvas_t* canvas, uint16_t x, uint16_t y, const uint8_t* bitmap, uint16_t width, uint16_t height) {
    if(!canvas || !bitmap) return;
    
    for(uint16_t row = 0; row < height; row++) {
        for(uint16_t col = 0; col < width; col++) {
            uint16_t byte_index = (row * width + col) / 8;
            uint8_t bit_index = 7 - ((row * width + col) % 8);
            
            if(bitmap[byte_index] & (1 << bit_index)) {
                canvas_draw_pixel(canvas, x + col, y + row);
            }
        }
    }
}

// Advanced drawing functions
void canvas_draw_disc(canvas_t* canvas, uint16_t cx, uint16_t cy, uint16_t radius) {
    if(!canvas) return;
    
    canvas_draw_circle(canvas, cx, cy, radius);
    
    // Fill the circle
    for(int16_t y = -radius; y <= radius; y++) {
        for(int16_t x = -radius; x <= radius; x++) {
            if(x*x + y*y <= radius*radius) {
                canvas_draw_pixel(canvas, cx + x, cy + y);
            }
        }
    }
}

void canvas_draw_triangle(canvas_t* canvas, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3) {
    if(!canvas) return;
    
    canvas_draw_line(canvas, x1, y1, x2, y2);
    canvas_draw_line(canvas, x2, y2, x3, y3);
    canvas_draw_line(canvas, x3, y3, x1, y1);
}

void canvas_draw_round_frame(canvas_t* canvas, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius) {
    if(!canvas) return;
    
    // Draw corners
    canvas_draw_circle(canvas, x + radius, y + radius, radius);
    canvas_draw_circle(canvas, x + width - radius - 1, y + radius, radius);
    canvas_draw_circle(canvas, x + radius, y + height - radius - 1, radius);
    canvas_draw_circle(canvas, x + width - radius - 1, y + height - radius - 1, radius);
    
    // Draw edges
    canvas_draw_line(canvas, x + radius, y, x + width - radius - 1, y);
    canvas_draw_line(canvas, x + width - 1, y + radius, x + width - 1, y + height - radius - 1);
    canvas_draw_line(canvas, x + radius, y + height - 1, x + width - radius - 1, y + height - 1);
    canvas_draw_line(canvas, x, y + radius, x, y + height - radius - 1);
}
