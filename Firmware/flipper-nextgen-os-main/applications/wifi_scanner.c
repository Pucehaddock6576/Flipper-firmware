/* WiFi Network Scanner & Analyzer Tool */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/canvas.h>
#include <gui/animation.h>
#include <string.h>

#define UNUSED(x) (void)(x)

// WiFi network structure
typedef struct {
    char ssid[33];
    uint8_t bssid[6];
    int8_t rssi;
    uint8_t channel;
    uint8_t security;
    uint8_t encryption;
    bool is_wps_enabled;
    uint8_t wps_version;
    bool is_hidden;
    uint32_t last_seen;
} wifi_network_t;

// Security types
typedef enum {
    SECURITY_OPEN = 0,
    SECURITY_WEP = 1,
    SECURITY_WPA = 2,
    SECURITY_WPA2 = 3,
    SECURITY_WPA3 = 4,
    SECURITY_WPA2_WPA3 = 5
} wifi_security_t;

// Encryption types
typedef enum {
    ENCRYPTION_NONE = 0,
    ENCRYPTION_WEP = 1,
    ENCRYPTION_TKIP = 2,
    ENCRYPTION_CCMP = 3,
    ENCRYPTION_GCMP = 4
} wifi_encryption_t;

// Scanner state
typedef struct {
    wifi_network_t* networks;
    uint8_t network_count;
    uint8_t max_networks;
    bool is_scanning;
    uint32_t scan_start_time;
    uint32_t networks_found;
    uint8_t current_channel;
    int8_t best_rssi;
    uint8_t selected_network;
    bool show_details;
} wifi_scanner_state_t;

static wifi_scanner_state_t g_scanner_state = {0};

// Security type to string
static const char* security_to_string(uint8_t security) {
    switch(security) {
        case SECURITY_OPEN: return "OPEN";
        case SECURITY_WEP: return "WEP";
        case SECURITY_WPA: return "WPA";
        case SECURITY_WPA2: return "WPA2";
        case SECURITY_WPA3: return "WPA3";
        case SECURITY_WPA2_WPA3: return "WPA2/3";
        default: return "UNKNOWN";
    }
}

// Format RSSI to signal strength
static const char* rssi_to_strength(int8_t rssi) {
    if(rssi >= -50) return "EXCELLENT";
    if(rssi >= -60) return "GOOD";
    if(rssi >= -70) return "FAIR";
    if(rssi >= -80) return "POOR";
    return "VERY POOR";
}

// Simulate WiFi scanning (placeholder for actual WiFi hardware)
static void simulate_wifi_scan(void) {
    if(!g_scanner_state.is_scanning) return;
    
    // Simulate finding networks
    if(rand() % 10 == 0 && g_scanner_state.network_count < g_scanner_state.max_networks) {
        wifi_network_t* network = &g_scanner_state.networks[g_scanner_state.network_count];
        
        // Generate random SSID
        const char* ssids[] = {
            "HomeNetwork", "OfficeWiFi", "GuestNetwork", "IoT_Devices",
            "SmartHome", "SecurityCam", "PrinterNet", "MediaServer",
            "MobileHotspot", "NeighborWiFi", "PublicWiFi", "Cafe_Free"
        };
        
        snprintf(network->ssid, sizeof(network->ssid), "%s_%d", 
                ssids[rand() % (sizeof(ssids)/sizeof(ssids[0]))], rand() % 100);
        
        // Generate random BSSID
        for(int i = 0; i < 6; i++) {
            network->bssid[i] = rand() % 256;
        }
        
        network->rssi = -30 - (rand() % 70); // -30 to -100 dBm
        network->channel = 1 + (rand() % 13); // Channels 1-13
        network->security = rand() % 6;
        network->encryption = rand() % 5;
        network->is_wps_enabled = (rand() % 3) == 0;
        network->wps_version = 1 + (rand() % 2);
        network->is_hidden = (rand() % 10) == 0;
        network->last_seen = furi_get_tick();
        
        g_scanner_state.network_count++;
        g_scanner_state.networks_found++;
        
        if(network->rssi > g_scanner_state.best_rssi) {
            g_scanner_state.best_rssi = network->rssi;
        }
        
        FURI_LOG_D("WIFI", "Found network: %s (%s, %d dBm)", 
                   network->ssid, security_to_string(network->security), network->rssi);
    }
    
    // Update scan progress
    g_scanner_state.current_channel++;
    if(g_scanner_state.current_channel > 13) {
        g_scanner_state.current_channel = 1;
        g_scanner_state.is_scanning = false;
        FURI_LOG_I("WIFI", "Scan completed. Found %d networks", g_scanner_state.network_count);
    }
}

// Initialize scanner
static void wifi_scanner_init(void) {
    if(g_scanner_state.networks) {
        furi_free(g_scanner_state.networks);
    }
    
    g_scanner_state.max_networks = 50;
    g_scanner_state.networks = furi_alloc(sizeof(wifi_network_t) * g_scanner_state.max_networks);
    g_scanner_state.network_count = 0;
    g_scanner_state.is_scanning = false;
    g_scanner_state.networks_found = 0;
    g_scanner_state.current_channel = 1;
    g_scanner_state.best_rssi = -100;
    g_scanner_state.selected_network = 0;
    g_scanner_state.show_details = false;
    
    memset(g_scanner_state.networks, 0, sizeof(wifi_network_t) * g_scanner_state.max_networks);
}

// Start scanning
static void wifi_scanner_start(void) {
    g_scanner_state.network_count = 0;
    g_scanner_state.is_scanning = true;
    g_scanner_state.scan_start_time = furi_get_tick();
    g_scanner_state.current_channel = 1;
    g_scanner_state.best_rssi = -100;
    g_scanner_state.networks_found = 0;
    
    FURI_LOG_I("WIFI", "Starting WiFi scan");
}

// Format BSSID as string
static void format_bssid(const uint8_t* bssid, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%02X:%02X:%02X:%02X:%02X:%02X",
             bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
}

// WiFi Scanner Application
int32_t wifi_scanner_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I("WIFI", "Starting WiFi Network Scanner");
    
    // Initialize GUI
    gui_t* gui = gui_alloc();
    if(!gui) {
        FURI_LOG_E("WIFI", "Failed to allocate GUI");
        return 1;
    }
    
    // Create view port
    view_port_t* view_port = view_port_alloc();
    if(!view_port) {
        FURI_LOG_E("WIFI", "Failed to allocate view port");
        gui_free(gui);
        return 1;
    }
    
    // Add to GUI
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Create canvas
    canvas_t* canvas = canvas_alloc(128, 64);
    if(!canvas) {
        FURI_LOG_E("WIFI", "Failed to allocate canvas");
        view_port_free(view_port);
        gui_free(gui);
        return 1;
    }
    
    // Initialize scanner
    wifi_scanner_init();
    
    // Main application loop
    uint32_t frame_counter = 0;
    uint8_t scroll_offset = 0;
    
    while(1) {
        // Update scanner
        if(g_scanner_state.is_scanning) {
            simulate_wifi_scan();
        }
        
        // Update display every 5 frames
        if(frame_counter % 5 == 0) {
            canvas_clear(canvas, CanvasColorWhite);
            canvas_set_color(canvas, CanvasColorBlack);
            canvas_set_font(canvas, &font_8x11);
            
            if(g_scanner_state.show_details && g_scanner_state.selected_network < g_scanner_state.network_count) {
                // Show network details
                wifi_network_t* network = &g_scanner_state.networks[g_scanner_state.selected_network];
                
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Network Details");
                
                char line_buffer[32];
                uint8_t y = 14;
                
                // SSID
                canvas_draw_str(canvas, 2, y, "SSID:");
                canvas_draw_str(canvas, 30, y, network->ssid);
                y += 10;
                
                // BSSID
                format_bssid(network->bssid, line_buffer, sizeof(line_buffer));
                canvas_draw_str(canvas, 2, y, "BSSID:");
                canvas_draw_str(canvas, 35, y, line_buffer);
                y += 10;
                
                // Channel & RSSI
                snprintf(line_buffer, sizeof(line_buffer), "CH:%d RSSI:%d", network->channel, network->rssi);
                canvas_draw_str(canvas, 2, y, line_buffer);
                y += 10;
                
                // Security
                snprintf(line_buffer, sizeof(line_buffer), "SEC:%s", security_to_string(network->security));
                canvas_draw_str(canvas, 2, y, line_buffer);
                y += 10;
                
                // WPS
                if(network->is_wps_enabled) {
                    snprintf(line_buffer, sizeof(line_buffer), "WPS: v%d", network->wps_version);
                    canvas_draw_str(canvas, 2, y, line_buffer);
                    y += 10;
                }
                
                // Signal strength
                snprintf(line_buffer, sizeof(line_buffer), "Signal: %s", rssi_to_strength(network->rssi));
                canvas_draw_str(canvas, 2, y, line_buffer);
                
                canvas_draw_str(canvas, 2, 58, "BACK:Return OK:Scan");
                
            } else {
                // Main scanner screen
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "WiFi Network Scanner");
                
                // Scan status
                uint8_t y = 14;
                if(g_scanner_state.is_scanning) {
                    char status[32];
                    snprintf(status, sizeof(status), "Scanning CH:%d/13", g_scanner_state.current_channel);
                    canvas_draw_str(canvas, 2, y, status);
                    y += 10;
                    
                    // Animated scanning indicator
                    uint8_t anim_pos = (frame_counter / 2) % 16;
                    canvas_draw_box(canvas, 2 + anim_pos, y, 16 - anim_pos, 2);
                    y += 6;
                } else {
                    canvas_draw_str(canvas, 2, y, "Scan Complete");
                    y += 10;
                }
                
                // Network count
                char count_str[32];
                snprintf(count_str, sizeof(count_str), "Networks: %d/50", g_scanner_state.network_count);
                canvas_draw_str(canvas, 2, y, count_str);
                y += 10;
                
                // Best signal
                if(g_scanner_state.best_rssi > -100) {
                    char best_str[32];
                    snprintf(best_str, sizeof(best_str), "Best: %d dBm", g_scanner_state.best_rssi);
                    canvas_draw_str(canvas, 2, y, best_str);
                    y += 10;
                }
                
                // Network list (show top networks)
                if(g_scanner_state.network_count > 0) {
                    uint8_t max_display = 3;
                    uint8_t start_idx = (scroll_offset < g_scanner_state.network_count) ? scroll_offset : 0;
                    
                    for(uint8_t i = 0; i < max_display && (start_idx + i) < g_scanner_state.network_count; i++) {
                        wifi_network_t* network = &g_scanner_state.networks[start_idx + i];
                        
                        char net_line[32];
                        // Truncate SSID if too long
                        char short_ssid[13];
                        strncpy(short_ssid, network->ssid, 12);
                        short_ssid[12] = '\0';
                        
                        snprintf(net_line, sizeof(net_line), "%s %s", short_ssid, security_to_string(network->security));
                        canvas_draw_str(canvas, 2, y + i * 8, net_line);
                        
                        // RSSI indicator
                        uint8_t bars = 0;
                        if(network->rssi >= -50) bars = 4;
                        else if(network->rssi >= -60) bars = 3;
                        else if(network->rssi >= -70) bars = 2;
                        else if(network->rssi >= -80) bars = 1;
                        
                        for(uint8_t b = 0; b < bars; b++) {
                            canvas_draw_box(canvas, 110 + b * 4, y + i * 8 + 1, 2, 4);
                        }
                    }
                } else {
                    canvas_draw_str(canvas, 2, y, "No networks found");
                }
                
                // Controls
                if(g_scanner_state.is_scanning) {
                    canvas_draw_str(canvas, 2, 58, "BACK:Stop");
                } else {
                    canvas_draw_str(canvas, 2, 58, "OK:Scan UP:Select");
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
    if(g_scanner_state.networks) {
        furi_free(g_scanner_state.networks);
    }
    canvas_free(canvas);
    view_port_free(view_port);
    gui_free(gui);
    
    FURI_LOG_I("WIFI", "WiFi scanner completed");
    FURI_LOG_I("WIFI", "Total networks found: %d", g_scanner_state.networks_found);
    
    return 0;
}
