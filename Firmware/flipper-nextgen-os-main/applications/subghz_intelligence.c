/* Sub-GHz Application - Advanced Signal Intelligence Interface */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/canvas.h>
#include <protocols/subghz_analyzer.h>
#include <protocols/ai_signal_detector.h>
#include <input/input.h>
#include <string.h>

// Forward declarations
static void subghz_app_draw_signal_info(canvas_t* canvas, subghz_app_t* app);
static void subghz_app_draw_protocol_list(canvas_t* canvas, subghz_app_t* app);
static void subghz_app_draw_ai_stats(canvas_t* canvas, subghz_app_t* app);
static void subghz_analyzer_callback(subghz_analyzer_t* analyzer, void* context);

// Application state
typedef struct {
    Gui* gui;
    ViewPort* view_port;
    subghz_analyzer_t* analyzer;
    ai_signal_detector_t* ai_detector;
    
    bool is_capturing;
    bool ai_enabled;
    uint32_t last_update_time;
    
    // UI state
    uint8_t current_screen; // 0: signal info, 1: protocol list, 2: AI stats
    uint8_t scroll_offset;
    uint8_t selected_protocol;
    
    // Display data
    char current_protocol[32];
    float current_rssi;
    float current_snr;
    uint32_t current_frequency;
    uint32_t signal_count;
    float ai_confidence;
    
    // Animation
    uint16_t scan_animation_offset;
    bool scan_animation_direction;
} subghz_app_t;

// Screen constants
#define SCREEN_SIGNAL_INFO 0
#define SCREEN_PROTOCOL_LIST 1
#define SCREEN_AI_STATS 2

// Protocol list for display
static const char* known_protocols[] = {
    "CAME", "NICE FLO", "SOMFY", "BETT", "KEELOQ", "HCS300", "HCS200",
    "OREGON", "LACROSSE", "ACURITE", "BYRON", "CHAMBERLAIN", "HONEYWELL",
    "LIGHTWAVERF", "BLYNK", "NEXA", "ANSLUT", "ADEMCO", "DSC", "VISONIC",
    "WIEGAND", "EM4100", "HID"
};
#define KNOWN_PROTOCOLS_COUNT (sizeof(known_protocols) / sizeof(known_protocols[0]))

// Forward declarations
static void subghz_app_draw_callback(Canvas* canvas, void* context);
static void subghz_app_input_callback(InputEvent* event, void* context);
static void subghz_analyzer_callback(subghz_analyzer_t* analyzer, void* context);

// Application initialization
int32_t subghz_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I("SUBGHZ_APP", "Starting Sub-GHz Signal Intelligence App");
    
    // Allocate application state
    subghz_app_t* app = furi_alloc(sizeof(subghz_app_t));
    if(!app) return 1;
    
    memset(app, 0, sizeof(subghz_app_t));
    
    // Initialize GUI
    app->gui = furi_record_open("gui");
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, subghz_app_draw_callback, app);
    view_port_input_callback_set(app->view_port, subghz_app_input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    
    // Initialize analyzer
    app->analyzer = subghz_analyzer_alloc();
    subghz_analyzer_set_callback(app->analyzer, subghz_analyzer_callback, app);
    
    // Initialize AI detector
    app->ai_detector = ai_signal_detector_alloc();
    ai_signal_detector_add_pattern(app->ai_detector, &AI_PATTERN_CAME);
    ai_signal_detector_add_pattern(app->ai_detector, &AI_PATTERN_NICE_FLO);
    ai_signal_detector_add_pattern(app->ai_detector, &AI_PATTERN_SOMFY);
    ai_signal_detector_add_pattern(app->ai_detector, &AI_PATTERN_KEELOQ);
    ai_signal_detector_enable_learning(app->ai_detector, true);
    ai_signal_detector_set_confidence_threshold(app->ai_detector, 0.7f);
    
    app->ai_enabled = true;
    app->last_update_time = furi_get_tick();
    
    FURI_LOG_I("SUBGHZ_APP", "Initialization complete");
    
    // Main application loop
    while(1) {
        // Update analyzer
        subghz_analyzer_update(app->analyzer);
        
        // Update AI detector if enabled
        if(app->ai_enabled) {
            subghz_signal_t* signal = subghz_analyzer_get_current_signal(app->analyzer);
            if(signal && signal->sample_count > MIN_SIGNAL_LENGTH) {
                ai_detection_result_t* result = ai_signal_detector_detect(app->ai_detector, signal);
                if(result) {
                    strncpy(app->current_protocol, result->protocol_name, sizeof(app->current_protocol) - 1);
                    app->ai_confidence = result->confidence;
                    ai_detection_result_free(result);
                }
            }
        }
        
        // Update display
        view_port_update(app->view_port);
        
        // Small delay
        furi_delay_ms(50);
    }
    
    return 0;
}

// Drawing callback
static void subghz_app_draw_callback(canvas_t* canvas, void* context) {
    subghz_app_t* app = (subghz_app_t*)context;
    if(!app) return;
    
    canvas_clear(canvas, CanvasColorWhite);
    canvas_set_color(canvas, CanvasColorBlack);
    canvas_set_font(canvas, &font_8x11);
    
    switch(app->current_screen) {
        case SCREEN_SIGNAL_INFO:
            subghz_app_draw_signal_info(canvas, app);
            break;
        case SCREEN_PROTOCOL_LIST:
            subghz_app_draw_protocol_list(canvas, app);
            break;
        case SCREEN_AI_STATS:
            subghz_app_draw_ai_stats(canvas, app);
            break;
    }
}

// Draw signal information screen
static void subghz_app_draw_signal_info(canvas_t* canvas, subghz_app_t* app) {
    const uint8_t width = 128;
    const uint8_t height = 64;
    uint8_t y = 2;
    
    // Title
    canvas_draw_str_aligned(canvas, width / 2, y, AlignCenter, AlignTop, "Sub-GHz Intelligence");
    y += 12;
    
    // Draw scanning animation
    if(app->is_capturing) {
        canvas_draw_frame(canvas, 2, y, width - 4, 8);
        canvas_draw_box(canvas, 4, y + 2, app->scan_animation_offset, 4);
        
        // Update animation
        app->scan_animation_offset += app->scan_animation_direction ? 2 : -2;
        if(app->scan_animation_offset >= width - 10 || app->scan_animation_offset <= 0) {
            app->scan_animation_direction = !app->scan_animation_direction;
        }
    } else {
        canvas_draw_str(canvas, 2, y, "Press OK to start capture");
    }
    y += 12;
    
    // Current signal info
    if(app->signal_count > 0) {
        char buffer[32];
        
        // Protocol
        canvas_draw_str(canvas, 2, y, "Protocol:");
        canvas_draw_str(canvas, 50, y, app->current_protocol);
        y += 10;
        
        // Frequency
        snprintf(buffer, sizeof(buffer), "Freq:%luHz", app->current_frequency);
        canvas_draw_str(canvas, 2, y, buffer);
        y += 10;
        
        // RSSI
        snprintf(buffer, sizeof(buffer), "RSSI:%.1fdB", app->current_rssi);
        canvas_draw_str(canvas, 2, y, buffer);
        y += 10;
        
        // SNR
        snprintf(buffer, sizeof(buffer), "SNR:%.1fdB", app->current_snr);
        canvas_draw_str(canvas, 2, y, buffer);
        y += 10;
        
        // AI confidence
        if(app->ai_enabled) {
            snprintf(buffer, sizeof(buffer), "AI:%.0f%%", app->ai_confidence * 100.0f);
            canvas_draw_str(canvas, 2, y, buffer);
            y += 10;
        }
        
        // Signal count
        snprintf(buffer, sizeof(buffer), "Signals:%lu", app->signal_count);
        canvas_draw_str(canvas, 2, y, buffer);
    } else {
        canvas_draw_str(canvas, 2, y, "No signals captured");
    }
    
    // Navigation hint
    canvas_draw_str_aligned(canvas, width / 2, height - 10, AlignCenter, AlignTop, 
                          "OK:Capture | UP:Protocols | DOWN:AI");
}

// Draw protocol list screen
static void subghz_app_draw_protocol_list(canvas_t* canvas, subghz_app_t* app) {
    const uint8_t width = 128;
    const uint8_t height = 64;
    uint8_t y = 2;
    
    // Title
    canvas_draw_str_aligned(canvas, width / 2, y, AlignCenter, AlignTop, "Known Protocols");
    y += 12;
    
    // Draw protocol list
    uint8_t max_items = 4; // Max items visible
    uint8_t start_idx = app->scroll_offset;
    uint8_t end_idx = start_idx + max_items;
    if(end_idx > KNOWN_PROTOCOLS_COUNT) end_idx = KNOWN_PROTOCOLS_COUNT;
    
    for(uint8_t i = start_idx; i < end_idx; i++) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%c %s", 
                (i == app->selected_protocol) ? '>' : ' ', known_protocols[i]);
        
        if(i == app->selected_protocol) {
            canvas_draw_box(canvas, 0, y, width, 9);
            canvas_set_color(canvas, CanvasColorWhite);
            canvas_draw_str(canvas, 2, y + 1, buffer);
            canvas_set_color(canvas, CanvasColorBlack);
        } else {
            canvas_draw_str(canvas, 2, y + 1, buffer);
        }
        
        y += 9;
    }
    
    // Scroll indicator
    if(KNOWN_PROTOCOLS_COUNT > max_items) {
        uint8_t scroll_height = (height - 30) * max_items / KNOWN_PROTOCOLS_COUNT;
        uint8_t scroll_pos = (height - 30) * app->scroll_offset / KNOWN_PROTOCOLS_COUNT;
        canvas_draw_box(canvas, width - 2, 15 + scroll_pos, 2, scroll_height);
    }
    
    // Navigation hint
    canvas_draw_str_aligned(canvas, width / 2, height - 10, AlignCenter, AlignTop, 
                          "UP/DOWN:Scroll | OK:Select | BACK:Return");
}

// Draw AI statistics screen
static void subghz_app_draw_ai_stats(canvas_t* canvas, subghz_app_t* app) {
    const uint8_t width = 128;
    const uint8_t height = 64;
    uint8_t y = 2;
    
    // Title
    canvas_draw_str_aligned(canvas, width / 2, y, AlignCenter, AlignTop, "AI Statistics");
    y += 12;
    
    if(app->ai_detector) {
        ai_detector_stats_t* stats = ai_signal_detector_get_stats(app->ai_detector);
        if(stats) {
            char buffer[32];
            
            // Signals processed
            snprintf(buffer, sizeof(buffer), "Processed: %lu", stats->signals_processed);
            canvas_draw_str(canvas, 2, y, buffer);
            y += 10;
            
            // Protocols identified
            snprintf(buffer, sizeof(buffer), "Identified: %lu", stats->protocols_identified);
            canvas_draw_str(canvas, 2, y, buffer);
            y += 10;
            
            // False positives
            snprintf(buffer, sizeof(buffer), "False Pos: %lu", stats->false_positives);
            canvas_draw_str(canvas, 2, y, buffer);
            y += 10;
            
            // Average confidence
            snprintf(buffer, sizeof(buffer), "Avg Conf: %.1f%%", stats->average_confidence * 100.0f);
            canvas_draw_str(canvas, 2, y, buffer);
            y += 10;
            
            // Learning iterations
            snprintf(buffer, sizeof(buffer), "Learning: %lu", stats->learning_iterations);
            canvas_draw_str(canvas, 2, y, buffer);
            y += 10;
            
            // Model accuracy
            float accuracy = ai_calculate_accuracy(stats);
            snprintf(buffer, sizeof(buffer), "Accuracy: %.1f%%", accuracy * 100.0f);
            canvas_draw_str(canvas, 2, y, buffer);
        }
    } else {
        canvas_draw_str(canvas, 2, y, "AI Detector not available");
    }
    
    // AI status
    y += 12;
    canvas_draw_str(canvas, 2, y, app->ai_enabled ? "AI: ENABLED" : "AI: DISABLED");
    
    // Navigation hint
    canvas_draw_str_aligned(canvas, width / 2, height - 10, AlignCenter, AlignTop, 
                          "OK:Toggle AI | BACK:Return");
}

// Input callback
static void subghz_app_input_callback(InputEvent* event, void* context) {
    subghz_app_t* app = (subghz_app_t*)context;
    if(!app || !event) return;
    
    if(event->type != InputTypePress) return;
    
    switch(event->key) {
        case InputKeyOk:
            switch(app->current_screen) {
                case SCREEN_SIGNAL_INFO:
                    // Toggle capture
                    if(app->is_capturing) {
                        subghz_analyzer_stop_capture(app->analyzer);
                        app->is_capturing = false;
                        FURI_LOG_I("SUBGHZ_APP", "Capture stopped");
                    } else {
                        subghz_analyzer_start_capture(app->analyzer);
                        app->is_capturing = true;
                        app->signal_count = 0;
                        FURI_LOG_I("SUBGHZ_APP", "Capture started");
                    }
                    break;
                    
                case SCREEN_PROTOCOL_LIST:
                    // Select protocol (placeholder)
                    FURI_LOG_I("SUBGHZ_APP", "Selected protocol: %s", 
                               known_protocols[app->selected_protocol]);
                    break;
                    
                case SCREEN_AI_STATS:
                    // Toggle AI
                    app->ai_enabled = !app->ai_enabled;
                    ai_signal_detector_enable_learning(app->ai_detector, app->ai_enabled);
                    break;
            }
            break;
            
        case InputKeyBack:
            if(app->is_capturing) {
                subghz_analyzer_stop_capture(app->analyzer);
                app->is_capturing = false;
            } else if(app->current_screen != SCREEN_SIGNAL_INFO) {
                app->current_screen = SCREEN_SIGNAL_INFO;
            }
            break;
            
        case InputKeyUp:
            if(app->current_screen == SCREEN_SIGNAL_INFO) {
                app->current_screen = SCREEN_PROTOCOL_LIST;
                app->scroll_offset = 0;
            } else if(app->current_screen == SCREEN_PROTOCOL_LIST) {
                if(app->scroll_offset > 0) {
                    app->scroll_offset--;
                } else if(app->selected_protocol > 0) {
                    app->selected_protocol--;
                }
            }
            break;
            
        case InputKeyDown:
            if(app->current_screen == SCREEN_SIGNAL_INFO) {
                app->current_screen = SCREEN_AI_STATS;
            } else if(app->current_screen == SCREEN_PROTOCOL_LIST) {
                if(app->scroll_offset + 4 < KNOWN_PROTOCOLS_COUNT) {
                    app->scroll_offset++;
                } else if(app->selected_protocol < KNOWN_PROTOCOLS_COUNT - 1) {
                    app->selected_protocol++;
                }
            }
            break;
            
        case InputKeyLeft:
            if(app->current_screen == SCREEN_PROTOCOL_LIST) {
                if(app->selected_protocol > 0) {
                    app->selected_protocol--;
                    if(app->selected_protocol < app->scroll_offset) {
                        app->scroll_offset = app->selected_protocol;
                    }
                }
            }
            break;
            
        case InputKeyRight:
            if(app->current_screen == SCREEN_PROTOCOL_LIST) {
                if(app->selected_protocol < KNOWN_PROTOCOLS_COUNT - 1) {
                    app->selected_protocol++;
                    if(app->selected_protocol >= app->scroll_offset + 4) {
                        app->scroll_offset = app->selected_protocol - 3;
                    }
                }
            }
            break;
            
        default:
            break;
    }
}

// Analyzer callback
static void subghz_analyzer_callback(subghz_analyzer_t* analyzer, void* context) {
    subghz_app_t* app = (subghz_app_t*)context;
    if(!app || !analyzer) return;
    
    // Update display data
    subghz_signal_t* signal = subghz_analyzer_get_current_signal(analyzer);
    const subghz_protocol_entry_t* protocol = subghz_analyzer_get_detected_protocol(analyzer);
    
    if(signal) {
        app->current_rssi = signal->rssi;
        app->current_snr = signal->snr;
        app->current_frequency = signal->frequency;
        app->signal_count++;
        
        if(protocol) {
            strncpy(app->current_protocol, protocol->name, sizeof(app->current_protocol) - 1);
        } else {
            strncpy(app->current_protocol, "UNKNOWN", sizeof(app->current_protocol) - 1);
        }
        
        FURI_LOG_D("SUBGHZ_APP", "Signal detected: %s, RSSI: %.1fdB, SNR: %.1fdB", 
                   app->current_protocol, app->current_rssi, app->current_snr);
    }
}
