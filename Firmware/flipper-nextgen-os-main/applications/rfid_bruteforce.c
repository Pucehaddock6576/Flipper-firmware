/* RFID/NFC Tag Bruteforce Tool - Door Access System */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/canvas.h>
#include <gui/animation.h>
#include <string.h>

#define UNUSED(x) (void)(x)

// Bruteforce state
typedef struct {
    uint64_t current_tag;
    uint64_t start_range;
    uint64_t end_range;
    uint32_t attempts;
    uint32_t successful_attempts;
    bool is_running;
    bool success_found;
    uint32_t last_attempt_time;
    uint32_t attempts_per_second;
} bruteforce_state_t;

// Common RFID tag formats
typedef enum {
    FORMAT_EM4100 = 0,    // 40-bit EM4100
    FORMAT_HID_PROX = 1,  // 26/32/40 bit HID Prox
    FORMAT_AWID = 2,      // 26/40 bit AWID
    FORMAT_INDALA = 3,    // 26/40 bit Indala
    FORMAT_PAC = 4,       // 32/64 bit PAC/Stanley
    FORMAT_NEXA = 5,      // 32/64 bit Nexa/Ansluta
    FORMAT_KERUI = 6,     // 32 bit Kerui
    FORMAT_CUSTOM = 7     // Custom range
} tag_format_t;

// Tag format definitions
typedef struct {
    const char* name;
    uint8_t bits;
    uint64_t min_value;
    uint64_t max_value;
    const char* description;
} tag_format_info_t;

static const tag_format_info_t tag_formats[] = {
    {"EM4100", 40, 0x0000000000, 0xFFFFFFFFFF, "Standard 40-bit EM4100 tags"},
    {"HID Prox", 26, 0x00000000, 0x03FFFFFF, "26-bit HID Prox cards"},
    {"HID Prox 40", 40, 0x0000000000, 0xFFFFFFFFFF, "40-bit HID Prox cards"},
    {"AWID", 26, 0x00000000, 0x03FFFFFF, "26-bit AWID access cards"},
    {"AWID 40", 40, 0x0000000000, 0xFFFFFFFFFF, "40-bit AWID cards"},
    {"Indala", 26, 0x00000000, 0x03FFFFFF, "26-bit Indala cards"},
    {"Indala 40", 40, 0x0000000000, 0xFFFFFFFFFF, "40-bit Indala cards"},
    {"PAC/Stanley", 32, 0x00000000, 0xFFFFFFFF, "32-bit PAC/Stanley tags"},
    {"PAC 64", 64, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, "64-bit PAC tags"},
    {"Nexa/Ansluta", 32, 0x00000000, 0xFFFFFFFF, "32-bit Nexa remote controls"},
    {"Nexa 64", 64, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, "64-bit Nexa remotes"},
    {"Kerui", 32, 0x00000000, 0xFFFFFFFF, "32-bit Kerui security sensors"},
    {"Custom", 64, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, "Custom user-defined range"}
};

static bruteforce_state_t g_bruteforce_state = {0};

// Convert tag ID to hex string
static void tag_to_hex_string(uint64_t tag, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%016llX", tag);
}

// Format tag for display based on format
static void format_tag_display(uint64_t tag, tag_format_t format, char* buffer, size_t buffer_size) {
    switch(format) {
        case FORMAT_EM4100:
            snprintf(buffer, buffer_size, "EM:%010llX", tag);
            break;
        case FORMAT_HID_PROX:
            snprintf(buffer, buffer_size, "HID:%08X", (uint32_t)(tag & 0xFFFFFFFF));
            break;
        case FORMAT_AWID:
            snprintf(buffer, buffer_size, "AW:%08X", (uint32_t)(tag & 0xFFFFFFFF));
            break;
        case FORMAT_INDALA:
            snprintf(buffer, buffer_size, "IND:%08X", (uint32_t)(tag & 0xFFFFFFFF));
            break;
        case FORMAT_PAC:
            snprintf(buffer, buffer_size, "PAC:%08X", (uint32_t)(tag & 0xFFFFFFFF));
            break;
        case FORMAT_NEXA:
            snprintf(buffer, buffer_size, "NX:%08X", (uint32_t)(tag & 0xFFFFFFFF));
            break;
        case FORMAT_KERUI:
            snprintf(buffer, buffer_size, "KR:%08X", (uint32_t)(tag & 0xFFFFFFFF));
            break;
        default:
            tag_to_hex_string(tag, buffer, buffer_size);
            break;
    }
}

// Simulate tag transmission (placeholder for actual RFID transmission)
static bool simulate_tag_transmission(uint64_t tag) {
    // In real implementation, this would:
    // 1. Configure RFID/NFC hardware
    // 2. Transmit the tag using appropriate protocol
    // 3. Listen for door access response
    // 4. Return true if access granted
    
    // For demo, simulate 1% success rate
    return (rand() % 100) == 0;
}

// Bruteforce engine
static void bruteforce_step(void) {
    if(!g_bruteforce_state.is_running || g_bruteforce_state.success_found) {
        return;
    }
    
    uint32_t current_time = furi_get_tick();
    
    // Calculate attempts per second
    if(g_bruteforce_state.last_attempt_time > 0) {
        uint32_t time_diff = current_time - g_bruteforce_state.last_attempt_time;
        if(time_diff > 0) {
            g_bruteforce_state.attempts_per_second = 1000 / time_diff;
        }
    }
    g_bruteforce_state.last_attempt_time = current_time;
    
    // Attempt current tag
    bool success = simulate_tag_transmission(g_bruteforce_state.current_tag);
    g_bruteforce_state.attempts++;
    
    if(success) {
        g_bruteforce_state.successful_attempts++;
        g_bruteforce_state.success_found = true;
        g_bruteforce_state.is_running = false;
        FURI_LOG_I("BRUTEFORCE", "SUCCESS! Tag found: %016llX", g_bruteforce_state.current_tag);
    }
    
    // Move to next tag
    if(g_bruteforce_state.current_tag < g_bruteforce_state.end_range) {
        g_bruteforce_state.current_tag++;
    } else {
        // Range completed
        g_bruteforce_state.is_running = false;
        FURI_LOG_I("BRUTEFORCE", "Range completed. Total attempts: %lu", g_bruteforce_state.attempts);
    }
}

// Initialize bruteforce for specific format
static void bruteforce_init(tag_format_t format, uint64_t custom_start, uint64_t custom_end) {
    if(format >= 0 && format < sizeof(tag_formats)/sizeof(tag_formats[0])) {
        g_bruteforce_state.start_range = tag_formats[format].min_value;
        g_bruteforce_state.end_range = tag_formats[format].max_value;
    } else {
        g_bruteforce_state.start_range = custom_start;
        g_bruteforce_state.end_range = custom_end;
    }
    
    g_bruteforce_state.current_tag = g_bruteforce_state.start_range;
    g_bruteforce_state.attempts = 0;
    g_bruteforce_state.successful_attempts = 0;
    g_bruteforce_state.is_running = false;
    g_bruteforce_state.success_found = false;
    g_bruteforce_state.last_attempt_time = 0;
    g_bruteforce_state.attempts_per_second = 0;
}

// Bruteforce application
int32_t bruteforce_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I("BRUTEFORCE", "Starting RFID/NFC Bruteforce Tool");
    
    // Initialize GUI
    gui_t* gui = gui_alloc();
    if(!gui) {
        FURI_LOG_E("BRUTEFORCE", "Failed to allocate GUI");
        return 1;
    }
    
    // Create view port
    view_port_t* view_port = view_port_alloc();
    if(!view_port) {
        FURI_LOG_E("BRUTEFORCE", "Failed to allocate view port");
        gui_free(gui);
        return 1;
    }
    
    // Add to GUI
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Create canvas
    canvas_t* canvas = canvas_alloc(128, 64);
    if(!canvas) {
        FURI_LOG_E("BRUTEFORCE", "Failed to allocate canvas");
        view_port_free(view_port);
        gui_free(gui);
        return 1;
    }
    
    // Initialize with EM4100 format
    bruteforce_init(FORMAT_EM4100, 0, 0);
    
    // Main application loop
    uint32_t screen_update_counter = 0;
    tag_format_t current_format = FORMAT_EM4100;
    bool show_format_list = false;
    uint8_t format_scroll = 0;
    
    while(1) {
        // Update bruteforce engine
        if(g_bruteforce_state.is_running) {
            for(int i = 0; i < 10; i++) { // Multiple attempts per frame for speed
                bruteforce_step();
                if(!g_bruteforce_state.is_running) break;
            }
        }
        
        // Update display every 10 frames
        if(screen_update_counter % 10 == 0) {
            canvas_clear(canvas, CanvasColorWhite);
            canvas_set_color(canvas, CanvasColorBlack);
            canvas_set_font(canvas, &font_8x11);
            
            if(show_format_list) {
                // Show format selection screen
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Select Format:");
                
                uint8_t y = 14;
                for(uint8_t i = format_scroll; i < format_scroll + 4 && i < sizeof(tag_formats)/sizeof(tag_formats[0]); i++) {
                    char format_line[32];
                    snprintf(format_line, sizeof(format_line), "%d. %s", i, tag_formats[i].name);
                    canvas_draw_str(canvas, 4, y, format_line);
                    y += 11;
                }
                
                canvas_draw_str(canvas, 2, 58, "UP/DOWN:Scroll OK:Select");
                
            } else {
                // Main bruteforce screen
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "RFID Bruteforce Tool");
                
                // Current format
                char format_str[32];
                snprintf(format_str, sizeof(format_str), "Format: %s", tag_formats[current_format].name);
                canvas_draw_str(canvas, 2, 14, format_str);
                
                // Current tag
                char tag_str[32];
                format_tag_display(g_bruteforce_state.current_tag, current_format, tag_str, sizeof(tag_str));
                canvas_draw_str(canvas, 2, 25, tag_str);
                
                // Progress
                char progress_str[32];
                uint64_t total_range = g_bruteforce_state.end_range - g_bruteforce_state.start_range + 1;
                uint64_t current_progress = g_bruteforce_state.current_tag - g_bruteforce_state.start_range + 1;
                uint8_t progress_percent = (current_progress * 100) / total_range;
                snprintf(progress_str, sizeof(progress_str), "Progress: %d%% (%lu/%lu)", 
                        progress_percent, (uint32_t)current_progress, (uint32_t)total_range);
                canvas_draw_str(canvas, 2, 36, progress_str);
                
                // Statistics
                char stats_str[32];
                snprintf(stats_str, sizeof(stats_str), "Attempts: %lu (%lu/s)", 
                        g_bruteforce_state.attempts, g_bruteforce_state.attempts_per_second);
                canvas_draw_str(canvas, 2, 47, stats_str);
                
                // Status
                if(g_bruteforce_state.success_found) {
                    canvas_set_color(canvas, CanvasColorBlack);
                    canvas_draw_box(canvas, 2, 56, 124, 6);
                    canvas_set_color(canvas, CanvasColorWhite);
                    canvas_draw_str_aligned(canvas, 64, 58, AlignCenter, AlignTop, "SUCCESS FOUND!");
                    canvas_set_color(canvas, CanvasColorBlack);
                } else if(g_bruteforce_state.is_running) {
                    canvas_draw_str(canvas, 2, 58, "RUNNING... BACK:Stop");
                } else {
                    canvas_draw_str(canvas, 2, 58, "OK:Start BACK:Format");
                }
            }
            
            gui_update(gui);
        }
        
        screen_update_counter++;
        furi_delay_ms(50); // 20 FPS
        
        // Exit condition (simplified - in real app would handle button input)
        if(screen_update_counter > 1000) break; // Exit after ~50 seconds for demo
    }
    
    // Cleanup
    canvas_free(canvas);
    view_port_free(view_port);
    gui_free(gui);
    
    FURI_LOG_I("BRUTEFORCE", "Bruteforce tool completed");
    FURI_LOG_I("BRUTEFORCE", "Total attempts: %lu, Successful: %lu", 
               g_bruteforce_state.attempts, g_bruteforce_state.successful_attempts);
    
    return 0;
}
