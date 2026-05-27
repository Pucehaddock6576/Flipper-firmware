/* Simple Sub-GHz Test Application */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/canvas.h>
#include <string.h>

#define UNUSED(x) (void)(x)

// Simple test app
int32_t subghz_test_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I("SUBGHZ_TEST", "Starting Sub-GHz Test App");
    
    // Initialize GUI
    gui_t* gui = gui_alloc();
    if(!gui) {
        FURI_LOG_E("SUBGHZ_TEST", "Failed to allocate GUI");
        return 1;
    }
    
    // Create view port
    view_port_t* view_port = view_port_alloc();
    if(!view_port) {
        FURI_LOG_E("SUBGHZ_TEST", "Failed to allocate view port");
        gui_free(gui);
        return 1;
    }
    
    // Add to GUI
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Test canvas
    canvas_t* canvas = canvas_alloc(128, 64);
    if(!canvas) {
        FURI_LOG_E("SUBGHZ_TEST", "Failed to allocate canvas");
        view_port_free(view_port);
        gui_free(gui);
        return 1;
    }
    
    // Test drawing
    canvas_clear(canvas, CanvasColorWhite);
    canvas_set_color(canvas, CanvasColorBlack);
    canvas_set_font(canvas, &font_8x11);
    
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "Sub-GHz Test");
    canvas_draw_str(canvas, 2, 50, "Press BACK to exit");
    
    FURI_LOG_I("SUBGHZ_TEST", "Test app initialized successfully");
    
    // Simple loop
    uint32_t counter = 0;
    while(counter < 10) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Count: %lu", counter++);
        
        canvas_clear(canvas, CanvasColorWhite);
        canvas_set_color(canvas, CanvasColorBlack);
        canvas_set_font(canvas, &font_8x11);
        
        canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignTop, "Flipper Next-Gen OS");
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, buffer);
        canvas_draw_str(canvas, 2, 50, "Press BACK to exit");
        
        furi_delay_ms(1000);
        
        // Update display
        gui_update(gui);
    }
    
    // Cleanup
    canvas_free(canvas);
    view_port_free(view_port);
    gui_free(gui);
    
    FURI_LOG_I("SUBGHZ_TEST", "Test app completed");
    return 0;
}
