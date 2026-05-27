/* GUI ViewPort Header - Window Management System */

#ifndef GUI_VIEW_PORT_H
#define GUI_VIEW_PORT_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct canvas canvas_t;
typedef struct view_port view_port_t;
typedef struct input_event InputEvent;

// GUI layers
typedef enum {
    GuiLayerDesktop = 0,
    GuiLayerWindow = 1,
    GuiLayerFullscreen = 2,
    GuiLayerConsole = 3
} GuiLayer;

// Drawing callback
typedef void (*view_port_draw_callback_t)(canvas_t* canvas, void* context);

// Input callback
typedef void (*view_port_input_callback_t)(InputEvent* event, void* context);

// ViewPort management
view_port_t* view_port_alloc(void);
void view_port_free(view_port_t* view_port);
void view_port_draw_callback_set(view_port_t* view_port, view_port_draw_callback_t callback, void* context);
void view_port_input_callback_set(view_port_t* view_port, view_port_input_callback_t callback, void* context);

// GUI system
typedef struct gui gui_t;

gui_t* gui_alloc(void);
void gui_free(gui_t* gui);
void gui_add_view_port(gui_t* gui, view_port_t* view_port, GuiLayer layer);
void gui_remove_view_port(gui_t* gui, view_port_t* view_port);
void gui_update(gui_t* gui);

// Input events
typedef enum {
    InputTypePress = 0,
    InputTypeRelease = 1,
    InputTypeShort = 2,
    InputTypeLong = 3,
    InputTypeRepeat = 4
} InputType;

typedef enum {
    InputKeyBack = 0x01,
    InputKeyUp = 0x02,
    InputKeyDown = 0x04,
    InputKeyLeft = 0x08,
    InputKeyRight = 0x10,
    InputKeyOk = 0x20,
    InputKeyMAX = 0x40
} InputKey;

struct input_event {
    InputType type;
    InputKey key;
    uint16_t sequence;
};

#endif // GUI_VIEW_PORT_H
