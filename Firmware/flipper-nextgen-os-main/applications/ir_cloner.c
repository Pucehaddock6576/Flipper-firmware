/* IR Remote Control Cloner & Universal Remote */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/canvas.h>
#include <gui/animation.h>
#include <string.h>

#define UNUSED(x) (void)(x)

// IR signal structure
typedef struct {
    uint16_t* timings;
    uint16_t timing_count;
    uint32_t frequency;
    uint16_t carrier_duty;
    uint8_t protocol;
    uint32_t address;
    uint32_t command;
    char name[21];
    bool is_learned;
    uint32_t timestamp;
} ir_signal_t;

// IR protocols
typedef enum {
    IR_PROTOCOL_NEC = 0,
    IR_PROTOCOL_SONY = 1,
    IR_PROTOCOL_RC5 = 2,
    IR_PROTOCOL_RC6 = 3,
    IR_PROTOCOL_SAMSUNG = 4,
    IR_PROTOCOL_PANASONIC = 5,
    IR_PROTOCOL_JVC = 6,
    IR_PROTOCOL_SHARP = 7,
    IR_PROTOCOL_DENON = 8,
    IR_PROTOCOL_NEC42 = 9,
    IR_PROTOCOL_UNKNOWN = 255
} ir_protocol_t;

// Remote types
typedef enum {
    REMOTE_TV = 0,
    REMOTE_AC = 1,
    REMOTE_AUDIO = 2,
    REMOTE_DVD = 3,
    REMOTE_PROJECTOR = 4,
    REMOTE_FAN = 5,
    REMOTE_LIGHTS = 6,
    REMOTE_CUSTOM = 7
} remote_type_t;

// Cloner state
typedef struct {
    ir_signal_t* signals;
    uint8_t signal_count;
    uint8_t max_signals;
    bool is_learning;
    bool is_transmitting;
    uint8_t selected_signal;
    uint8_t selected_remote_type;
    uint32_t last_learn_time;
    uint8_t learn_attempts;
    bool show_signal_details;
    bool show_remote_menu;
    char current_remote_name[21];
} ir_cloner_state_t;

static ir_cloner_state_t g_ir_state = {0};

// Protocol to string
static const char* protocol_to_string(uint8_t protocol) {
    switch(protocol) {
        case IR_PROTOCOL_NEC: return "NEC";
        case IR_PROTOCOL_SONY: return "Sony";
        case IR_PROTOCOL_RC5: return "RC5";
        case IR_PROTOCOL_RC6: return "RC6";
        case IR_PROTOCOL_SAMSUNG: return "Samsung";
        case IR_PROTOCOL_PANASONIC: return "Panasonic";
        case IR_PROTOCOL_JVC: return "JVC";
        case IR_PROTOCOL_SHARP: return "Sharp";
        case IR_PROTOCOL_DENON: return "Denon";
        case IR_PROTOCOL_NEC42: return "NEC42";
        default: return "Unknown";
    }
}

// Remote type to string
static const char* remote_type_to_string(uint8_t type) {
    switch(type) {
        case REMOTE_TV: return "TV";
        case REMOTE_AC: return "Air Conditioner";
        case REMOTE_AUDIO: return "Audio System";
        case REMOTE_DVD: return "DVD/Blu-ray";
        case REMOTE_PROJECTOR: return "Projector";
        case REMOTE_FAN: return "Fan";
        case REMOTE_LIGHTS: return "Lights";
        case REMOTE_CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

// Simulate IR learning
static bool simulate_ir_learning(void) {
    if(!g_ir_state.is_learning) return false;
    
    // Simulate successful learning with 70% probability
    if(rand() % 10 < 7) {
        ir_signal_t* signal = &g_ir_state.signals[g_ir_state.signal_count];
        
        // Generate random signal parameters
        signal->protocol = rand() % 10;
        signal->frequency = 38000 + (rand() % 4000); // 38-42 kHz typical
        signal->carrier_duty = 30 + (rand() % 20); // 30-50% duty cycle
        signal->address = rand() % 0xFFFF;
        signal->command = rand() % 0xFFFF;
        signal->timing_count = 10 + (rand() % 50); // 10-60 timing entries
        signal->is_learned = true;
        signal->timestamp = furi_get_tick();
        
        // Generate random signal name based on remote type
        const char* button_names[] = {
            "Power", "Volume Up", "Volume Down", "Channel Up", "Channel Down",
            "Mute", "Menu", "Back", "Home", "Input", "Settings", "OK", "Up", "Down", "Left", "Right",
            "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Play", "Pause", "Stop", "Record"
        };
        
        snprintf(signal->name, sizeof(signal->name), "%s_%d", 
                button_names[rand() % (sizeof(button_names)/sizeof(button_names[0]))], 
                g_ir_state.signal_count + 1);
        
        // Allocate timing array
        signal->timings = furi_alloc(sizeof(uint16_t) * signal->timing_count);
        for(uint16_t i = 0; i < signal->timing_count; i++) {
            signal->timings[i] = 100 + (rand() % 2000); // 100-2100 microseconds
        }
        
        g_ir_state.signal_count++;
        g_ir_state.last_learn_time = furi_get_tick();
        g_ir_state.learn_attempts++;
        
        FURI_LOG_I("IR", "Learned signal: %s (%s)", signal->name, protocol_to_string(signal->protocol));
        return true;
    }
    
    g_ir_state.learn_attempts++;
    return false;
}

// Simulate IR transmission
static bool simulate_ir_transmission(ir_signal_t* signal) {
    if(!signal) return false;
    
    // Simulate transmission success
    g_ir_state.is_transmitting = true;
    
    // In real implementation, this would:
    // 1. Configure IR hardware for the specific frequency
    // 2. Generate carrier wave with correct duty cycle
    // 3. Modulate according to protocol timing
    // 4. Transmit the complete signal
    
    furi_delay_ms(50); // Simulate transmission time
    g_ir_state.is_transmitting = false;
    
    FURI_LOG_D("IR", "Transmitted signal: %s", signal->name);
    return true;
}

// Initialize cloner
static void ir_cloner_init(void) {
    if(g_ir_state.signals) {
        for(uint8_t i = 0; i < g_ir_state.signal_count; i++) {
            if(g_ir_state.signals[i].timings) {
                furi_free(g_ir_state.signals[i].timings);
            }
        }
        furi_free(g_ir_state.signals);
    }
    
    g_ir_state.max_signals = 50;
    g_ir_state.signals = furi_alloc(sizeof(ir_signal_t) * g_ir_state.max_signals);
    g_ir_state.signal_count = 0;
    g_ir_state.is_learning = false;
    g_ir_state.is_transmitting = false;
    g_ir_state.selected_signal = 0;
    g_ir_state.selected_remote_type = REMOTE_TV;
    g_ir_state.last_learn_time = 0;
    g_ir_state.learn_attempts = 0;
    g_ir_state.show_signal_details = false;
    g_ir_state.show_remote_menu = false;
    
    strncpy(g_ir_state.current_remote_name, "My Remote", sizeof(g_ir_state.current_remote_name));
    
    memset(g_ir_state.signals, 0, sizeof(ir_signal_t) * g_ir_state.max_signals);
}

// Start learning
static void ir_start_learning(void) {
    g_ir_state.is_learning = true;
    g_ir_state.learn_attempts = 0;
    FURI_LOG_I("IR", "Starting IR learning mode");
}

// Stop learning
static void ir_stop_learning(void) {
    g_ir_state.is_learning = false;
    FURI_LOG_I("IR", "Stopped IR learning mode");
}

// IR Cloner Application
int32_t ir_cloner_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I("IR", "Starting IR Remote Cloner");
    
    // Initialize GUI
    gui_t* gui = gui_alloc();
    if(!gui) {
        FURI_LOG_E("IR", "Failed to allocate GUI");
        return 1;
    }
    
    // Create view port
    view_port_t* view_port = view_port_alloc();
    if(!view_port) {
        FURI_LOG_E("IR", "Failed to allocate view port");
        gui_free(gui);
        return 1;
    }
    
    // Add to GUI
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Create canvas
    canvas_t* canvas = canvas_alloc(128, 64);
    if(!canvas) {
        FURI_LOG_E("IR", "Failed to allocate canvas");
        view_port_free(view_port);
        gui_free(gui);
        return 1;
    }
    
    // Initialize cloner
    ir_cloner_init();
    
    // Main application loop
    uint32_t frame_counter = 0;
    uint8_t scroll_offset = 0;
    
    while(1) {
        // Update learning
        if(g_ir_state.is_learning) {
            simulate_ir_learning();
        }
        
        // Update display every 5 frames
        if(frame_counter % 5 == 0) {
            canvas_clear(canvas, CanvasColorWhite);
            canvas_set_color(canvas, CanvasColorBlack);
            canvas_set_font(canvas, &font_8x11);
            
            if(g_ir_state.show_signal_details && g_ir_state.selected_signal < g_ir_state.signal_count) {
                // Show signal details
                ir_signal_t* signal = &g_ir_state.signals[g_ir_state.selected_signal];
                
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Signal Details");
                
                uint8_t y = 14;
                canvas_draw_str(canvas, 2, y, signal->name);
                y += 10;
                
                char info_line[32];
                snprintf(info_line, sizeof(info_line), "Protocol: %s", protocol_to_string(signal->protocol));
                canvas_draw_str(canvas, 2, y, info_line);
                y += 8;
                
                snprintf(info_line, sizeof(info_line), "Freq: %lu Hz", signal->frequency);
                canvas_draw_str(canvas, 2, y, info_line);
                y += 8;
                
                snprintf(info_line, sizeof(info_line), "Duty: %d%%", signal->carrier_duty);
                canvas_draw_str(canvas, 2, y, info_line);
                y += 8;
                
                snprintf(info_line, sizeof(info_line), "Addr: 0x%04lX", signal->address);
                canvas_draw_str(canvas, 2, y, info_line);
                y += 8;
                
                snprintf(info_line, sizeof(info_line), "Cmd: 0x%04lX", signal->command);
                canvas_draw_str(canvas, 2, y, info_line);
                y += 8;
                
                snprintf(info_line, sizeof(info_line), "Timings: %d", signal->timing_count);
                canvas_draw_str(canvas, 2, y, info_line);
                
                canvas_draw_str(canvas, 2, 58, "BACK:Return OK:Transmit");
                
            } else if(g_ir_state.show_remote_menu) {
                // Show remote menu
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Remote Control");
                
                uint8_t y = 14;
                
                // Remote name
                canvas_draw_str(canvas, 2, y, "Name:");
                canvas_draw_str(canvas, 30, y, g_ir_state.current_remote_name);
                y += 10;
                
                // Remote type
                char type_str[32];
                snprintf(type_str, sizeof(type_str), "Type: %s", remote_type_to_string(g_ir_state.selected_remote_type));
                canvas_draw_str(canvas, 2, y, type_str);
                y += 10;
                
                // Signal count
                char count_str[32];
                snprintf(count_str, sizeof(count_str), "Signals: %d/50", g_ir_state.signal_count);
                canvas_draw_str(canvas, 2, y, count_str);
                y += 10;
                
                // Quick actions
                canvas_draw_str(canvas, 2, y, "Quick Actions:");
                y += 8;
                canvas_draw_str(canvas, 2, y, "1. Learn Signal");
                y += 8;
                canvas_draw_str(canvas, 2, y, "2. View Signals");
                y += 8;
                canvas_draw_str(canvas, 2, y, "3. Transmit All");
                y += 8;
                canvas_draw_str(canvas, 2, y, "4. Clear All");
                
                canvas_draw_str(canvas, 2, 58, "BACK:Return");
                
            } else {
                // Main cloner screen
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "IR Remote Cloner");
                
                // Learning status
                uint8_t y = 14;
                if(g_ir_state.is_learning) {
                    canvas_draw_str(canvas, 2, y, "Learning Mode");
                    y += 10;
                    
                    // Animated learning indicator
                    uint8_t anim_pos = (frame_counter / 3) % 16;
                    for(uint8_t i = 0; i < 4; i++) {
                        uint8_t pos = (anim_pos + i * 4) % 16;
                        canvas_draw_box(canvas, 2 + pos * 7, y, 5, 3);
                    }
                    y += 6;
                    
                    // Learning attempts
                    char attempts_str[32];
                    snprintf(attempts_str, sizeof(attempts_str), "Attempts: %d", g_ir_state.learn_attempts);
                    canvas_draw_str(canvas, 2, y, attempts_str);
                    y += 10;
                } else if(g_ir_state.is_transmitting) {
                    canvas_draw_str(canvas, 2, y, "Transmitting...");
                    y += 10;
                } else {
                    canvas_draw_str(canvas, 2, y, "Ready");
                    y += 10;
                }
                
                // Signal count
                char count_str[32];
                snprintf(count_str, sizeof(count_str), "Signals: %d/50", g_ir_state.signal_count);
                canvas_draw_str(canvas, 2, y, count_str);
                y += 10;
                
                // Current remote
                canvas_draw_str(canvas, 2, y, "Remote:");
                canvas_draw_str(canvas, 40, y, g_ir_state.current_remote_name);
                y += 10;
                
                // Signal list
                if(g_ir_state.signal_count > 0) {
                    uint8_t max_display = 2;
                    uint8_t start_idx = (scroll_offset < g_ir_state.signal_count) ? scroll_offset : 0;
                    
                    for(uint8_t i = 0; i < max_display && (start_idx + i) < g_ir_state.signal_count; i++) {
                        ir_signal_t* signal = &g_ir_state.signals[start_idx + i];
                        
                        // Truncate name if too long
                        char short_name[16];
                        strncpy(short_name, signal->name, 15);
                        short_name[15] = '\0';
                        
                        canvas_draw_str(canvas, 2, y + i * 8, short_name);
                        
                        // Protocol indicator
                        char proto_char = protocol_to_string(signal->protocol)[0];
                        char proto_str[2] = {proto_char, '\0'};
                        canvas_draw_str(canvas, 70, y + i * 8, proto_str);
                        
                        // Frequency indicator
                        char freq_str[8];
                        snprintf(freq_str, sizeof(freq_str), "%luk", signal->frequency / 1000);
                        canvas_draw_str(canvas, 80, y + i * 8, freq_str);
                    }
                } else {
                    canvas_draw_str(canvas, 2, y, "No signals learned");
                }
                
                // Controls
                if(g_ir_state.is_learning) {
                    canvas_draw_str(canvas, 2, 58, "BACK:Stop");
                } else {
                    canvas_draw_str(canvas, 2, 58, "OK:Learn UP:Select");
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
    if(g_ir_state.signals) {
        for(uint8_t i = 0; i < g_ir_state.signal_count; i++) {
            if(g_ir_state.signals[i].timings) {
                furi_free(g_ir_state.signals[i].timings);
            }
        }
        furi_free(g_ir_state.signals);
    }
    canvas_free(canvas);
    view_port_free(view_port);
    gui_free(gui);
    
    FURI_LOG_I("IR", "IR cloner completed");
    FURI_LOG_I("IR", "Total signals learned: %d", g_ir_state.signal_count);
    
    return 0;
}
