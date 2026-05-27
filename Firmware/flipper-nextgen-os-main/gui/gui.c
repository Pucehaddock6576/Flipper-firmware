/* Basic GUI Implementation for Testing */

#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/canvas.h>
#include <furi.h>
#include <string.h>

// GUI structure
struct gui {
    view_port_t* view_ports[16]; // Max 16 view ports
    uint8_t view_port_count;
    uint32_t last_update_time;
};

// ViewPort structure
struct view_port {
    view_port_draw_callback_t draw_callback;
    view_port_input_callback_t input_callback;
    void* context;
    uint8_t width;
    uint8_t height;
    bool visible;
};

// GUI implementation
gui_t* gui_alloc(void) {
    gui_t* gui = furi_alloc(sizeof(gui_t));
    if(!gui) return NULL;
    
    memset(gui, 0, sizeof(gui_t));
    gui->last_update_time = furi_get_tick();
    
    return gui;
}

void gui_free(gui_t* gui) {
    if(gui) {
        for(uint8_t i = 0; i < gui->view_port_count; i++) {
            view_port_free(gui->view_ports[i]);
        }
        furi_free(gui);
    }
}

void gui_add_view_port(gui_t* gui, view_port_t* view_port, GuiLayer layer) {
    if(!gui || !view_port || gui->view_port_count >= 16) return;
    
    gui->view_ports[gui->view_port_count++] = view_port;
    view_port->visible = true;
    
    FURI_LOG_D("GUI", "Added view port at layer %d", layer);
}

void gui_remove_view_port(gui_t* gui, view_port_t* view_port) {
    if(!gui || !view_port) return;
    
    for(uint8_t i = 0; i < gui->view_port_count; i++) {
        if(gui->view_ports[i] == view_port) {
            // Remove and shift remaining
            for(uint8_t j = i; j < gui->view_port_count - 1; j++) {
                gui->view_ports[j] = gui->view_ports[j + 1];
            }
            gui->view_port_count--;
            view_port->visible = false;
            break;
        }
    }
}

void gui_update(gui_t* gui) {
    if(!gui) return;
    
    // Create a test canvas
    canvas_t* canvas = canvas_alloc(128, 64);
    if(!canvas) return;
    
    // Draw all view ports
    for(uint8_t i = 0; i < gui->view_port_count; i++) {
        view_port_t* vp = gui->view_ports[i];
        if(vp && vp->visible && vp->draw_callback) {
            vp->draw_callback(canvas, vp->context);
        }
    }
    
    canvas_free(canvas);
    
    gui->last_update_time = furi_get_tick();
}

// ViewPort implementation
view_port_t* view_port_alloc(void) {
    view_port_t* vp = furi_alloc(sizeof(view_port_t));
    if(!vp) return NULL;
    
    memset(vp, 0, sizeof(view_port_t));
    vp->width = 128;
    vp->height = 64;
    vp->visible = true;
    
    return vp;
}

void view_port_free(view_port_t* view_port) {
    if(view_port) {
        furi_free(view_port);
    }
}

void view_port_draw_callback_set(view_port_t* view_port, view_port_draw_callback_t callback, void* context) {
    if(view_port) {
        view_port->draw_callback = callback;
        view_port->context = context;
    }
}

void view_port_input_callback_set(view_port_t* view_port, view_port_input_callback_t callback, void* context) {
    if(view_port) {
        view_port->input_callback = callback;
        view_port->context = context;
    }
}
