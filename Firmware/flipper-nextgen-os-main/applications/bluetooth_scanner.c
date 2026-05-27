/* Bluetooth Device Scanner & Exploitation Tool */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/canvas.h>
#include <gui/animation.h>
#include <string.h>

#define UNUSED(x) (void)(x)

// Bluetooth device structure
typedef struct {
    char name[21];
    uint8_t mac[6];
    uint8_t device_class[3];
    int8_t rssi;
    uint8_t device_type;
    bool is_connectable;
    bool is_discoverable;
    uint8_t services_count;
    uint16_t services[8];
    bool is_paired;
    bool is_encrypted;
    uint32_t last_seen;
    uint8_t manufacturer_data_len;
    uint8_t manufacturer_data[16];
} bt_device_t;

// Device types
typedef enum {
    BT_DEVICE_UNKNOWN = 0,
    BT_DEVICE_PHONE = 1,
    BT_DEVICE_COMPUTER = 2,
    BT_DEVICE_HEADSET = 3,
    BT_DEVICE_SPEAKER = 4,
    BT_DEVICE_KEYBOARD = 5,
    BT_DEVICE_MOUSE = 6,
    BT_DEVICE_WATCH = 7,
    BT_DEVICE_FITNESS = 8,
    BT_DEVICE_CAR = 9,
    BT_DEVICE_IOT = 10
} bt_device_type_t;

// Scanner state
typedef struct {
    bt_device_t* devices;
    uint8_t device_count;
    uint8_t max_devices;
    bool is_scanning;
    uint32_t scan_start_time;
    uint32_t devices_found;
    uint8_t selected_device;
    bool show_details;
    bool show_exploits;
    uint8_t exploit_options;
} bt_scanner_state_t;

static bt_scanner_state_t g_bt_state = {0};

// Device type to string
static const char* device_type_to_string(uint8_t type) {
    switch(type) {
        case BT_DEVICE_PHONE: return "Phone";
        case BT_DEVICE_COMPUTER: return "Computer";
        case BT_DEVICE_HEADSET: return "Headset";
        case BT_DEVICE_SPEAKER: return "Speaker";
        case BT_DEVICE_KEYBOARD: return "Keyboard";
        case BT_DEVICE_MOUSE: return "Mouse";
        case BT_DEVICE_WATCH: return "Watch";
        case BT_DEVICE_FITNESS: return "Fitness";
        case BT_DEVICE_CAR: return "Car";
        case BT_DEVICE_IOT: return "IoT";
        default: return "Unknown";
    }
}

// Format MAC address
static void format_mac(const uint8_t* mac, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// Simulate Bluetooth scanning
static void simulate_bt_scan(void) {
    if(!g_bt_state.is_scanning) return;
    
    // Simulate finding devices
    if(rand() % 8 == 0 && g_bt_state.device_count < g_bt_state.max_devices) {
        bt_device_t* device = &g_bt_state.devices[g_bt_state.device_count];
        
        // Generate random device names
        const char* names[] = {
            "iPhone", "Galaxy S23", "AirPods Pro", "MacBook Pro",
            "Sony WH-1000", "Logitech MX", "Apple Watch", "Fitbit",
            "Tesla Model 3", "Smart Lock", "Bluetooth Speaker", "Wireless Mouse",
            "iPad Pro", "Dell Laptop", "JBL Speaker", "Garmin Watch"
        };
        
        snprintf(device->name, sizeof(device->name), "%s_%d", 
                names[rand() % (sizeof(names)/sizeof(names[0]))], rand() % 100);
        
        // Generate random MAC
        for(int i = 0; i < 6; i++) {
            device->mac[i] = rand() % 256;
        }
        
        device->rssi = -40 - (rand() % 60); // -40 to -100 dBm
        device->device_type = 1 + (rand() % 10);
        device->is_connectable = (rand() % 3) != 0;
        device->is_discoverable = (rand() % 2) == 0;
        device->services_count = rand() % 8;
        device->is_paired = false;
        device->is_encrypted = (rand() % 2) == 0;
        device->last_seen = furi_get_tick();
        device->manufacturer_data_len = rand() % 16;
        
        for(int i = 0; i < device->manufacturer_data_len; i++) {
            device->manufacturer_data[i] = rand() % 256;
        }
        
        g_bt_state.device_count++;
        g_bt_state.devices_found++;
        
        FURI_LOG_D("BT", "Found device: %s (%s, %d dBm)", 
                   device->name, device_type_to_string(device->device_type), device->rssi);
    }
}

// Initialize scanner
static void bt_scanner_init(void) {
    if(g_bt_state.devices) {
        furi_free(g_bt_state.devices);
    }
    
    g_bt_state.max_devices = 30;
    g_bt_state.devices = furi_alloc(sizeof(bt_device_t) * g_bt_state.max_devices);
    g_bt_state.device_count = 0;
    g_bt_state.is_scanning = false;
    g_bt_state.devices_found = 0;
    g_bt_state.selected_device = 0;
    g_bt_state.show_details = false;
    g_bt_state.show_exploits = false;
    g_bt_state.exploit_options = 0;
    
    memset(g_bt_state.devices, 0, sizeof(bt_device_t) * g_bt_state.max_devices);
}

// Start scanning
static void bt_scanner_start(void) {
    g_bt_state.device_count = 0;
    g_bt_state.is_scanning = true;
    g_bt_state.scan_start_time = furi_get_tick();
    g_bt_state.devices_found = 0;
    
    FURI_LOG_I("BT", "Starting Bluetooth scan");
}

// Check for vulnerabilities
static bool check_vulnerabilities(bt_device_t* device) {
    // Check for common vulnerabilities
    if(!device->is_encrypted) return true; // Unencrypted connection
    if(device->manufacturer_data_len > 0) return true; // Custom data potential
    if(device->services_count > 5) return true; // Many services = larger attack surface
    if(device->device_type == BT_DEVICE_IOT) return true; // IoT devices often vulnerable
    return false;
}

// Bluetooth Scanner Application
int32_t bt_scanner_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I("BT", "Starting Bluetooth Scanner & Exploiter");
    
    // Initialize GUI
    gui_t* gui = gui_alloc();
    if(!gui) {
        FURI_LOG_E("BT", "Failed to allocate GUI");
        return 1;
    }
    
    // Create view port
    view_port_t* view_port = view_port_alloc();
    if(!view_port) {
        FURI_LOG_E("BT", "Failed to allocate view port");
        gui_free(gui);
        return 1;
    }
    
    // Add to GUI
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Create canvas
    canvas_t* canvas = canvas_alloc(128, 64);
    if(!canvas) {
        FURI_LOG_E("BT", "Failed to allocate canvas");
        view_port_free(view_port);
        gui_free(gui);
        return 1;
    }
    
    // Initialize scanner
    bt_scanner_init();
    
    // Main application loop
    uint32_t frame_counter = 0;
    uint8_t scroll_offset = 0;
    
    while(1) {
        // Update scanner
        if(g_bt_state.is_scanning) {
            simulate_bt_scan();
        }
        
        // Update display every 5 frames
        if(frame_counter % 5 == 0) {
            canvas_clear(canvas, CanvasColorWhite);
            canvas_set_color(canvas, CanvasColorBlack);
            canvas_set_font(canvas, &font_8x11);
            
            if(g_bt_state.show_exploits && g_bt_state.selected_device < g_bt_state.device_count) {
                // Show exploit options
                bt_device_t* device = &g_bt_state.devices[g_bt_state.selected_device];
                
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Exploit Options");
                
                char mac_buffer[18];
                format_mac(device->mac, mac_buffer, sizeof(mac_buffer));
                
                uint8_t y = 14;
                canvas_draw_str(canvas, 2, y, "Target:");
                canvas_draw_str(canvas, 40, y, device->name);
                y += 10;
                
                canvas_draw_str(canvas, 2, y, "MAC:");
                canvas_draw_str(canvas, 30, y, mac_buffer);
                y += 10;
                
                // Exploit options based on device type and vulnerabilities
                bool vulnerable = check_vulnerabilities(device);
                
                if(vulnerable) {
                    canvas_draw_str(canvas, 2, y, "VULNERABLE!");
                    canvas_set_color(canvas, CanvasColorBlack);
                    canvas_draw_box(canvas, 2, y + 1, 60, 6);
                    canvas_set_color(canvas, CanvasColorWhite);
                    canvas_draw_str(canvas, 4, y + 2, "VULN");
                    canvas_set_color(canvas, CanvasColorBlack);
                    y += 10;
                    
                    // Specific exploits
                    if(!device->is_encrypted) {
                        canvas_draw_str(canvas, 2, y, "1. MITM Attack");
                        y += 8;
                    }
                    if(device->manufacturer_data_len > 0) {
                        canvas_draw_str(canvas, 2, y, "2. Data Injection");
                        y += 8;
                    }
                    if(device->device_type == BT_DEVICE_IOT) {
                        canvas_draw_str(canvas, 2, y, "3. IoT Takeover");
                        y += 8;
                    }
                    canvas_draw_str(canvas, 2, y, "4. DoS Attack");
                } else {
                    canvas_draw_str(canvas, 2, y, "SECURE DEVICE");
                    y += 10;
                    canvas_draw_str(canvas, 2, y, "Limited options:");
                    y += 8;
                    canvas_draw_str(canvas, 2, y, "1. DoS Attack");
                    y += 8;
                    canvas_draw_str(canvas, 2, y, "2. Jam Signal");
                }
                
                canvas_draw_str(canvas, 2, 58, "BACK:Return OK:Execute");
                
            } else if(g_bt_state.show_details && g_bt_state.selected_device < g_bt_state.device_count) {
                // Show device details
                bt_device_t* device = &g_bt_state.devices[g_bt_state.selected_device];
                
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Device Details");
                
                char line_buffer[32];
                uint8_t y = 14;
                
                // Name and type
                canvas_draw_str(canvas, 2, y, device->name);
                y += 8;
                canvas_draw_str(canvas, 2, y, device_type_to_string(device->device_type));
                y += 10;
                
                // MAC and RSSI
                format_mac(device->mac, line_buffer, sizeof(line_buffer));
                canvas_draw_str(canvas, 2, y, line_buffer);
                y += 8;
                snprintf(line_buffer, sizeof(line_buffer), "RSSI: %d dBm", device->rssi);
                canvas_draw_str(canvas, 2, y, line_buffer);
                y += 10;
                
                // Connection status
                if(device->is_connectable) canvas_draw_str(canvas, 2, y, "Connectable: YES");
                else canvas_draw_str(canvas, 2, y, "Connectable: NO");
                y += 8;
                
                if(device->is_discoverable) canvas_draw_str(canvas, 2, y, "Discoverable: YES");
                else canvas_draw_str(canvas, 2, y, "Discoverable: NO");
                y += 8;
                
                if(device->is_encrypted) canvas_draw_str(canvas, 2, y, "Encrypted: YES");
                else canvas_draw_str(canvas, 2, y, "Encrypted: NO");
                y += 8;
                
                // Services
                snprintf(line_buffer, sizeof(line_buffer), "Services: %d", device->services_count);
                canvas_draw_str(canvas, 2, y, line_buffer);
                
                canvas_draw_str(canvas, 2, 58, "BACK:Return OK:Exploit");
                
            } else {
                // Main scanner screen
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Bluetooth Scanner");
                
                // Scan status
                uint8_t y = 14;
                if(g_bt_state.is_scanning) {
                    canvas_draw_str(canvas, 2, y, "Scanning...");
                    y += 10;
                    
                    // Animated scanning indicator
                    uint8_t anim_pos = (frame_counter / 2) % 20;
                    for(uint8_t i = 0; i < 4; i++) {
                        uint8_t pos = (anim_pos + i * 5) % 20;
                        canvas_draw_box(canvas, 2 + pos, y, 3, 3);
                    }
                    y += 6;
                } else {
                    canvas_draw_str(canvas, 2, y, "Scan Complete");
                    y += 10;
                }
                
                // Device count
                char count_str[32];
                snprintf(count_str, sizeof(count_str), "Devices: %d/30", g_bt_state.device_count);
                canvas_draw_str(canvas, 2, y, count_str);
                y += 10;
                
                // Device list
                if(g_bt_state.device_count > 0) {
                    uint8_t max_display = 3;
                    uint8_t start_idx = (scroll_offset < g_bt_state.device_count) ? scroll_offset : 0;
                    
                    for(uint8_t i = 0; i < max_display && (start_idx + i) < g_bt_state.device_count; i++) {
                        bt_device_t* device = &g_bt_state.devices[start_idx + i];
                        
                        // Truncate name if too long
                        char short_name[13];
                        strncpy(short_name, device->name, 12);
                        short_name[12] = '\0';
                        
                        canvas_draw_str(canvas, 2, y + i * 8, short_name);
                        
                        // Device type indicator
                        char type_char = device_type_to_string(device->device_type)[0];
                        char type_str[2] = {type_char, '\0'};
                        canvas_draw_str(canvas, 70, y + i * 8, type_str);
                        
                        // RSSI bars
                        uint8_t bars = 0;
                        if(device->rssi >= -50) bars = 4;
                        else if(device->rssi >= -60) bars = 3;
                        else if(device->rssi >= -70) bars = 2;
                        else if(device->rssi >= -80) bars = 1;
                        
                        for(uint8_t b = 0; b < bars; b++) {
                            canvas_draw_box(canvas, 110 + b * 3, y + i * 8 + 1, 2, 4);
                        }
                        
                        // Vulnerability indicator
                        if(check_vulnerabilities(device)) {
                            canvas_draw_str(canvas, 80, y + i * 8, "!");
                        }
                    }
                } else {
                    canvas_draw_str(canvas, 2, y, "No devices found");
                }
                
                // Controls
                if(g_bt_state.is_scanning) {
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
    if(g_bt_state.devices) {
        furi_free(g_bt_state.devices);
    }
    canvas_free(canvas);
    view_port_free(view_port);
    gui_free(gui);
    
    FURI_LOG_I("BT", "Bluetooth scanner completed");
    FURI_LOG_I("BT", "Total devices found: %d", g_bt_state.devices_found);
    
    return 0;
}
