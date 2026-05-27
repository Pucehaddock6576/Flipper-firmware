/* GUI System Header - Main GUI Interface */

#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include <stdbool.h>
#include "view_port.h"

// Forward declaration
typedef struct gui gui_t;

// GUI management
gui_t* gui_alloc(void);
void gui_free(gui_t* gui);
void gui_add_view_port(gui_t* gui, view_port_t* view_port, GuiLayer layer);
void gui_remove_view_port(gui_t* gui, view_port_t* view_port);
void gui_update(gui_t* gui);

#endif // GUI_H
