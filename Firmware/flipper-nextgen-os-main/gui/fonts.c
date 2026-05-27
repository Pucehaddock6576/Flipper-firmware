/* Font definitions for GUI */

#include <gui/canvas.h>

// 8x11 font (simplified)
static const uint8_t font_8x11_data[] = {
    // ... font bitmap data would go here
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ... more font data
};

font_t font_8x11 = {
    .data = font_8x11_data,
    .width = 8,
    .height = 11,
    .space_width = 4,
    .first_char = ' ',
    .last_char = '~'
};

// 16x21 font (placeholder)
font_t font_16x21 = {
    .data = font_8x11_data,
    .width = 16,
    .height = 21,
    .space_width = 8,
    .first_char = ' ',
    .last_char = '~'
};

// 18x26 font (placeholder)
font_t font_18x26 = {
    .data = font_8x11_data,
    .width = 18,
    .height = 26,
    .space_width = 9,
    .first_char = ' ',
    .last_char = '~'
};

// 24x35 font (placeholder)
font_t font_24x35 = {
    .data = font_8x11_data,
    .width = 24,
    .height = 35,
    .space_width = 12,
    .first_char = ' ',
    .last_char = '~'
};

// Common icons (simplified)
static const uint8_t icon_button_center[] = {
    0x3C, 0x42, 0x42, 0x42, 0x3C
};

Icon I_ButtonCenter = {
    .data = icon_button_center,
    .width = 8,
    .height = 5
};

static const uint8_t icon_button_up[] = {
    0x18, 0x3C, 0x7E, 0x18, 0x18
};

Icon I_ButtonUp = {
    .data = icon_button_up,
    .width = 8,
    .height = 5
};

static const uint8_t icon_button_down[] = {
    0x18, 0x18, 0x7E, 0x3C, 0x18
};

Icon I_ButtonDown = {
    .data = icon_button_down,
    .width = 8,
    .height = 5
};

static const uint8_t icon_button_left[] = {
    0x18, 0x3C, 0x7E, 0x3C, 0x18
};

Icon I_ButtonLeft = {
    .data = icon_button_left,
    .width = 8,
    .height = 5
};

static const uint8_t icon_button_right[] = {
    0x18, 0x3C, 0x7E, 0x3C, 0x18
};

Icon I_ButtonRight = {
    .data = icon_button_right,
    .width = 8,
    .height = 5
};

static const uint8_t icon_button_up_left[] = {
    0x3C, 0x42, 0x42, 0x42, 0x3C
};

Icon I_ButtonUpLeft = {
    .data = icon_button_up_left,
    .width = 8,
    .height = 5
};

static const uint8_t icon_button_up_right[] = {
    0x3C, 0x42, 0x42, 0x42, 0x3C
};

Icon I_ButtonUpRight = {
    .data = icon_button_up_right,
    .width = 8,
    .height = 5
};

static const uint8_t icon_button_down_left[] = {
    0x3C, 0x42, 0x42, 0x42, 0x3C
};

Icon I_ButtonDownLeft = {
    .data = icon_button_down_left,
    .width = 8,
    .height = 5
};

static const uint8_t icon_button_down_right[] = {
    0x3C, 0x42, 0x42, 0x42, 0x3C
};

Icon I_ButtonDownRight = {
    .data = icon_button_down_right,
    .width = 8,
    .height = 5
};

static const uint8_t icon_ok_btn[] = {
    0x3C, 0x42, 0x42, 0x42, 0x3C
};

Icon I_Ok_btn = {
    .data = icon_ok_btn,
    .width = 8,
    .height = 5
};

static const uint8_t icon_back_btn[] = {
    0x3C, 0x42, 0x42, 0x42, 0x3C
};

Icon I_Back_btn = {
    .data = icon_back_btn,
    .width = 8,
    .height = 5
};

static const uint8_t icon_pinboard_arrow_up_10x8[] = {
    0x18, 0x3C, 0x7E, 0x18, 0x18
};

Icon I_Pinboard_arrow_up_10x8 = {
    .data = icon_pinboard_arrow_up_10x8,
    .width = 8,
    .height = 5
};

static const uint8_t icon_pinboard_arrow_down_10x8[] = {
    0x18, 0x18, 0x7E, 0x3C, 0x18
};

Icon I_Pinboard_arrow_down_10x8 = {
    .data = icon_pinboard_arrow_down_10x8,
    .width = 8,
    .height = 5
};

static const uint8_t icon_pinboard_arrow_left_10x8[] = {
    0x18, 0x3C, 0x7E, 0x3C, 0x18
};

Icon I_Pinboard_arrow_left_10x8 = {
    .data = icon_pinboard_arrow_left_10x8,
    .width = 8,
    .height = 5
};

static const uint8_t icon_pinboard_arrow_right_10x8[] = {
    0x18, 0x3C, 0x7E, 0x3C, 0x18
};

Icon I_Pinboard_arrow_right_10x8 = {
    .data = icon_pinboard_arrow_right_10x8,
    .width = 8,
    .height = 5
};
