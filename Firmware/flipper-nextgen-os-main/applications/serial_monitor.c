/* Serial Port Monitor & UART Sniffer */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/canvas.h>
#include <gui/animation.h>
#include <string.h>

#define UNUSED(x) (void)(x)

// Serial data structure
typedef struct {
    uint8_t data[256];
    uint16_t length;
    uint32_t timestamp;
    uint8_t port;
    uint32_t baudrate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    bool is_hex;
    bool is_ascii;
    bool is_binary;
} serial_data_t;

// UART configuration
typedef struct {
    uint32_t baudrate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    bool flow_control;
    uint8_t port;
} uart_config_t;

// Monitor state
typedef struct {
    serial_data_t* data_buffer;
    uint8_t buffer_count;
    uint8_t max_buffers;
    bool is_monitoring;
    bool is_sniffing;
    uint8_t selected_port;
    uart_config_t current_config;
    uint32_t bytes_received;
    uint32_t bytes_transmitted;
    uint32_t packets_captured;
    uint8_t display_mode;
    uint8_t scroll_offset;
    uint32_t last_activity;
    bool show_config;
    bool show_hex_dump;
    char search_pattern[17];
    bool search_active;
} serial_monitor_state_t;

static serial_monitor_state_t g_serial_state = {0};

// Common baudrates
static const uint32_t common_baudrates[] = {
    9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600
};

// Parity types
typedef enum {
    PARITY_NONE = 0,
    PARITY_EVEN = 1,
    PARITY_ODD = 2,
    PARITY_MARK = 3,
    PARITY_SPACE = 4
} uart_parity_t;

// Display modes
typedef enum {
    DISPLAY_ASCII = 0,
    DISPLAY_HEX = 1,
    DISPLAY_BINARY = 2,
    DISPLAY_MIXED = 3
} display_mode_t;

// Format baudrate
static void format_baudrate(uint32_t baudrate, char* buffer, size_t buffer_size) {
    if(baudrate >= 1000000) {
        snprintf(buffer, buffer_size, "%.1fM", (float)baudrate / 1000000.0);
    } else if(baudrate >= 1000) {
        snprintf(buffer, buffer_size, "%.0fk", (float)baudrate / 1000.0);
    } else {
        snprintf(buffer, buffer_size, "%lu", baudrate);
    }
}

// Format data for display
static void format_data_display(uint8_t* data, uint16_t length, display_mode_t mode, 
                              char* buffer, size_t buffer_size) {
    switch(mode) {
        case DISPLAY_ASCII:
            for(uint16_t i = 0; i < length && i < buffer_size - 1; i++) {
                if(data[i] >= 32 && data[i] <= 126) {
                    buffer[i] = data[i];
                } else {
                    buffer[i] = '.';
                }
            }
            buffer[length < buffer_size ? length : buffer_size - 1] = '\0';
            break;
            
        case DISPLAY_HEX:
            for(uint16_t i = 0; i < length && i * 3 < buffer_size - 1; i++) {
                snprintf(buffer + i * 3, buffer_size - i * 3, "%02X ", data[i]);
            }
            break;
            
        case DISPLAY_BINARY:
            for(uint16_t i = 0; i < length && i * 9 < buffer_size - 1; i++) {
                char binary_str[9];
                for(uint8_t b = 0; b < 8; b++) {
                    binary_str[b] = (data[i] & (1 << (7 - b))) ? '1' : '0';
                }
                binary_str[8] = '\0';
                snprintf(buffer + i * 9, buffer_size - i * 9, "%s ", binary_str);
            }
            break;
            
        case DISPLAY_MIXED:
            for(uint16_t i = 0; i < length && i < 4 < buffer_size - 1; i++) {
                if(data[i] >= 32 && data[i] <= 126) {
                    buffer[i] = data[i];
                } else {
                    snprintf(buffer + i, buffer_size - i, "[%02X]", data[i]);
                    if(i < 3) i += 2; // Skip extra characters for hex display
                }
            }
            break;
    }
}

// Simulate serial data reception
static void simulate_serial_reception(void) {
    if(!g_serial_state.is_monitoring && !g_serial_state.is_sniffing) return;
    
    // Simulate data reception with 20% probability
    if(rand() % 10 < 2 && g_serial_state.buffer_count < g_serial_state.max_buffers) {
        serial_data_t* data = &g_serial_state.data_buffer[g_serial_state.buffer_count];
        
        // Generate random data
        data->length = 1 + (rand() % 32); // 1-32 bytes
        data->timestamp = furi_get_tick();
        data->port = g_serial_state.selected_port;
        data->baudrate = g_serial_state.current_config.baudrate;
        data->data_bits = g_serial_state.current_config.data_bits;
        data->stop_bits = g_serial_state.current_config.stop_bits;
        data->parity = g_serial_state.current_config.parity;
        
        // Generate random data pattern
        for(uint16_t i = 0; i < data->length; i++) {
            if(g_serial_state.display_mode == DISPLAY_ASCII) {
                // Generate printable ASCII characters
                data->data[i] = 32 + (rand() % 95); // Space to ~
            } else {
                data->data[i] = rand() % 256; // Full byte range
            }
        }
        
        g_serial_state.buffer_count++;
        g_serial_state.bytes_received += data->length;
        g_serial_state.last_activity = furi_get_tick();
        
        if(g_serial_state.is_sniffing) {
            g_serial_state.packets_captured++;
        }
        
        FURI_LOG_D("SERIAL", "Received %d bytes on port %d", data->length, data->port);
    }
}

// Initialize monitor
static void serial_monitor_init(void) {
    if(g_serial_state.data_buffer) {
        furi_free(g_serial_state.data_buffer);
    }
    
    g_serial_state.max_buffers = 20;
    g_serial_state.data_buffer = furi_alloc(sizeof(serial_data_t) * g_serial_state.max_buffers);
    g_serial_state.buffer_count = 0;
    g_serial_state.is_monitoring = false;
    g_serial_state.is_sniffing = false;
    g_serial_state.selected_port = 0;
    g_serial_state.bytes_received = 0;
    g_serial_state.bytes_transmitted = 0;
    g_serial_state.packets_captured = 0;
    g_serial_state.display_mode = DISPLAY_ASCII;
    g_serial_state.scroll_offset = 0;
    g_serial_state.last_activity = 0;
    g_serial_state.show_config = false;
    g_serial_state.show_hex_dump = false;
    g_serial_state.search_active = false;
    
    // Default UART config
    g_serial_state.current_config.baudrate = 115200;
    g_serial_state.current_config.data_bits = 8;
    g_serial_state.current_config.stop_bits = 1;
    g_serial_state.current_config.parity = PARITY_NONE;
    g_serial_state.current_config.flow_control = false;
    g_serial_state.current_config.port = 0;
    
    memset(g_serial_state.data_buffer, 0, sizeof(serial_data_t) * g_serial_state.max_buffers);
    memset(g_serial_state.search_pattern, 0, sizeof(g_serial_state.search_pattern));
}

// Start monitoring
static void serial_start_monitoring(void) {
    g_serial_state.is_monitoring = true;
    g_serial_state.buffer_count = 0;
    g_serial_state.bytes_received = 0;
    g_serial_state.bytes_transmitted = 0;
    g_serial_state.last_activity = furi_get_tick();
    
    FURI_LOG_I("SERIAL", "Starting serial monitoring on port %d", g_serial_state.selected_port);
}

// Start sniffing
static void serial_start_sniffing(void) {
    g_serial_state.is_sniffing = true;
    g_serial_state.buffer_count = 0;
    g_serial_state.packets_captured = 0;
    g_serial_state.last_activity = furi_get_tick();
    
    FURI_LOG_I("SERIAL", "Starting UART sniffing on port %d", g_serial_state.selected_port);
}

// Stop monitoring/sniffing
static void serial_stop_all(void) {
    g_serial_state.is_monitoring = false;
    g_serial_state.is_sniffing = false;
    
    FURI_LOG_I("SERIAL", "Stopped serial monitoring/sniffing");
}

// Serial Monitor Application
int32_t serial_monitor_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I("SERIAL", "Starting Serial Port Monitor");
    
    // Initialize GUI
    gui_t* gui = gui_alloc();
    if(!gui) {
        FURI_LOG_E("SERIAL", "Failed to allocate GUI");
        return 1;
    }
    
    // Create view port
    view_port_t* view_port = view_port_alloc();
    if(!view_port) {
        FURI_LOG_E("SERIAL", "Failed to allocate view port");
        gui_free(gui);
        return 1;
    }
    
    // Add to GUI
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Create canvas
    canvas_t* canvas = canvas_alloc(128, 64);
    if(!canvas) {
        FURI_LOG_E("SERIAL", "Failed to allocate canvas");
        view_port_free(view_port);
        gui_free(gui);
        return 1;
    }
    
    // Initialize monitor
    serial_monitor_init();
    
    // Main application loop
    uint32_t frame_counter = 0;
    uint8_t selected_baudrate = 4; // 115200 default
    
    while(1) {
        // Update monitoring
        simulate_serial_reception();
        
        // Update display every 5 frames
        if(frame_counter % 5 == 0) {
            canvas_clear(canvas, CanvasColorWhite);
            canvas_set_color(canvas, CanvasColorBlack);
            canvas_set_font(canvas, &font_8x11);
            
            if(g_serial_state.show_config) {
                // Show configuration screen
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "UART Configuration");
                
                uint8_t y = 14;
                
                // Port selection
                char port_str[32];
                snprintf(port_str, sizeof(port_str), "Port: %d", g_serial_state.selected_port);
                canvas_draw_str(canvas, 2, y, port_str);
                y += 8;
                
                // Baudrate
                char baud_str[32];
                format_baudrate(g_serial_state.current_config.baudrate, baud_str, sizeof(baud_str));
                snprintf(port_str, sizeof(port_str), "Baud: %s", baud_str);
                canvas_draw_str(canvas, 2, y, port_str);
                y += 8;
                
                // Data bits
                snprintf(port_str, sizeof(port_str), "Data: %d bits", g_serial_state.current_config.data_bits);
                canvas_draw_str(canvas, 2, y, port_str);
                y += 8;
                
                // Stop bits
                snprintf(port_str, sizeof(port_str), "Stop: %d bits", g_serial_state.current_config.stop_bits);
                canvas_draw_str(canvas, 2, y, port_str);
                y += 8;
                
                // Parity
                const char* parity_str = "None";
                switch(g_serial_state.current_config.parity) {
                    case PARITY_EVEN: parity_str = "Even"; break;
                    case PARITY_ODD: parity_str = "Odd"; break;
                    case PARITY_MARK: parity_str = "Mark"; break;
                    case PARITY_SPACE: parity_str = "Space"; break;
                }
                snprintf(port_str, sizeof(port_str), "Parity: %s", parity_str);
                canvas_draw_str(canvas, 2, y, port_str);
                
                canvas_draw_str(canvas, 2, 58, "BACK:Return OK:Apply");
                
            } else if(g_serial_state.show_hex_dump && g_serial_state.buffer_count > 0) {
                // Show detailed hex dump
                serial_data_t* data = &g_serial_state.data_buffer[g_serial_state.buffer_count - 1];
                
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Hex Dump");
                
                uint8_t y = 14;
                
                // Show first 16 bytes in hex
                char hex_line[48];
                for(uint8_t i = 0; i < 4 && i < data->length; i++) {
                    uint8_t start = i * 4;
                    uint8_t end = start + 4;
                    if(end > data->length) end = data->length;
                    
                    char line_prefix[16];
                    snprintf(line_prefix, sizeof(line_prefix), "%04X: ", start);
                    strcpy(hex_line, line_prefix);
                    
                    for(uint8_t j = start; j < end; j++) {
                        char byte_str[4];
                        snprintf(byte_str, sizeof(byte_str), "%02X ", data->data[j]);
                        strcat(hex_line, byte_str);
                    }
                    
                    canvas_draw_str(canvas, 2, y, hex_line);
                    y += 8;
                }
                
                // Statistics
                char stats_str[32];
                snprintf(stats_str, sizeof(stats_str), "Total: %d bytes", data->length);
                canvas_draw_str(canvas, 2, y, stats_str);
                
                canvas_draw_str(canvas, 2, 58, "BACK:Return");
                
            } else {
                // Main monitor screen
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Serial Monitor");
                
                // Status
                uint8_t y = 14;
                if(g_serial_state.is_monitoring) {
                    canvas_draw_str(canvas, 2, y, "Monitoring");
                    y += 8;
                    
                    // Activity indicator
                    uint32_t time_since_activity = furi_get_tick() - g_serial_state.last_activity;
                    if(time_since_activity < 1000) {
                        canvas_draw_box(canvas, 2, y, 20, 2);
                    } else {
                        canvas_draw_box(canvas, 2, y, 5, 2);
                    }
                    y += 6;
                } else if(g_serial_state.is_sniffing) {
                    canvas_draw_str(canvas, 2, y, "Sniffing");
                    y += 8;
                    
                    // Sniffing indicator
                    uint8_t anim_pos = (frame_counter / 2) % 8;
                    for(uint8_t i = 0; i < 3; i++) {
                        uint8_t pos = (anim_pos + i * 3) % 8;
                        canvas_draw_box(canvas, 2 + pos * 3, y, 2, 2);
                    }
                    y += 6;
                } else {
                    canvas_draw_str(canvas, 2, y, "Idle");
                    y += 8;
                }
                
                // Port and baudrate
                char config_str[32];
                format_baudrate(g_serial_state.current_config.baudrate, config_str, sizeof(config_str));
                snprintf(config_str, sizeof(config_str), "P%d @ %s", g_serial_state.selected_port, config_str);
                canvas_draw_str(canvas, 2, y, config_str);
                y += 10;
                
                // Statistics
                char stats_str[32];
                if(g_serial_state.is_monitoring) {
                    snprintf(stats_str, sizeof(stats_str), "RX: %lu", g_serial_state.bytes_received);
                } else if(g_serial_state.is_sniffing) {
                    snprintf(stats_str, sizeof(stats_str), "PKT: %lu", g_serial_state.packets_captured);
                } else {
                    snprintf(stats_str, sizeof(stats_str), "Buffers: %d", g_serial_state.buffer_count);
                }
                canvas_draw_str(canvas, 2, y, stats_str);
                y += 10;
                
                // Data display
                if(g_serial_state.buffer_count > 0) {
                    uint8_t start_idx = (g_serial_state.scroll_offset < g_serial_state.buffer_count) ? 
                                      g_serial_state.scroll_offset : 0;
                    
                    for(uint8_t i = 0; i < 2 && (start_idx + i) < g_serial_state.buffer_count; i++) {
                        serial_data_t* data = &g_serial_state.data_buffer[start_idx + i];
                        
                        char display_line[32];
                        format_data_display(data->data, 
                                         (data->length > 8) ? 8 : data->length,
                                         g_serial_state.display_mode, 
                                         display_line, sizeof(display_line));
                        
                        canvas_draw_str(canvas, 2, y + i * 8, display_line);
                    }
                } else {
                    canvas_draw_str(canvas, 2, y, "No data received");
                }
                
                // Display mode indicator
                const char* mode_str = "ASCII";
                switch(g_serial_state.display_mode) {
                    case DISPLAY_HEX: mode_str = "HEX"; break;
                    case DISPLAY_BINARY: mode_str = "BIN"; break;
                    case DISPLAY_MIXED: mode_str = "MIX"; break;
                }
                canvas_draw_str(canvas, 100, 58, mode_str);
                
                // Controls
                if(g_serial_state.is_monitoring || g_serial_state.is_sniffing) {
                    canvas_draw_str(canvas, 2, 58, "BACK:Stop");
                } else {
                    canvas_draw_str(canvas, 2, 58, "OK:Monitor UP:Config");
                }
            }
            
            gui_update(gui);
        }
        
        frame_counter++;
        furi_delay_ms(50); // 20 FPS
        
        // Exit condition (simplified)
        if(frame_counter > 2000) break; // Exit after ~100 seconds for demo
    }
    
    // Cleanup
    if(g_serial_state.data_buffer) {
        furi_free(g_serial_state.data_buffer);
    }
    canvas_free(canvas);
    view_port_free(view_port);
    gui_free(gui);
    
    FURI_LOG_I("SERIAL", "Serial monitor completed");
    FURI_LOG_I("SERIAL", "Total bytes received: %lu", g_serial_state.bytes_received);
    FURI_LOG_I("SERIAL", "Total packets captured: %lu", g_serial_state.packets_captured);
    
    return 0;
}
