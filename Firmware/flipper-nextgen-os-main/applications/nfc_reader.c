/* NFC/RFID Card Reader & Emulator */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/canvas.h>
#include <gui/animation.h>
#include <string.h>

#define UNUSED(x) (void)(x)

// Card structure
typedef struct {
    char uid[21];           // Card UID as hex string
    uint8_t uid_bytes[10];  // Raw UID bytes
    uint8_t uid_length;     // UID length in bytes
    uint8_t card_type;      // Card type
    char atqa[5];          // ATQA response
    char sak[3];           // SAK response
    uint8_t sectors;        // Number of sectors
    uint8_t blocks_per_sector; // Blocks per sector
    bool is_emulated;      // Card is being emulated
    uint32_t last_read;    // Last read timestamp
    uint8_t read_count;    // Number of successful reads
    uint8_t data[1024];    // Card data (simplified)
    uint16_t data_length;  // Data length
} nfc_card_t;

// Card types
typedef enum {
    CARD_TYPE_MIFARE_1K = 0,
    CARD_TYPE_MIFARE_4K = 1,
    CARD_TYPE_MIFARE_ULTRALIGHT = 2,
    CARD_TYPE_MIFARE_DESFIRE = 3,
    CARD_TYPE_NTAG_213 = 4,
    CARD_TYPE_NTAG_215 = 5,
    CARD_TYPE_NTAG_216 = 6,
    CARD_TYPE_ISO_15693 = 7,
    CARD_TYPE_FELICA = 8,
    CARD_TYPE_UNKNOWN = 255
} nfc_card_type_t;

// Reader state
typedef struct {
    nfc_card_t* cards;
    uint8_t card_count;
    uint8_t max_cards;
    bool is_reading;
    bool is_emulating;
    bool is_writing;
    uint8_t selected_card;
    uint32_t total_reads;
    uint32_t successful_reads;
    uint32_t failed_reads;
    uint8_t current_uid_length;
    bool show_card_details;
    bool show_data_dump;
    bool show_emulator_menu;
    uint8_t emulation_mode;
    char custom_uid[21];
} nfc_reader_state_t;

static nfc_reader_state_t g_nfc_state = {0};

// Card type to string
static const char* card_type_to_string(uint8_t type) {
    switch(type) {
        case CARD_TYPE_MIFARE_1K: return "Mifare 1K";
        case CARD_TYPE_MIFARE_4K: return "Mifare 4K";
        case CARD_TYPE_MIFARE_ULTRALIGHT: return "Mifare Ultralight";
        case CARD_TYPE_MIFARE_DESFIRE: return "Mifare DESFire";
        case CARD_TYPE_NTAG_213: return "NTAG213";
        case CARD_TYPE_NTAG_215: return "NTAG215";
        case CARD_TYPE_NTAG_216: return "NTAG216";
        case CARD_TYPE_ISO_15693: return "ISO15693";
        case CARD_TYPE_FELICA: return "Felica";
        default: return "Unknown";
    }
}

// Format UID as hex string
static void format_uid_hex(const uint8_t* uid, uint8_t length, char* buffer, size_t buffer_size) {
    for(uint8_t i = 0; i < length && i * 3 < buffer_size - 1; i++) {
        snprintf(buffer + i * 3, buffer_size - i * 3, "%02X:", uid[i]);
    }
    if(length > 0) {
        buffer[length * 3 - 1] = '\0'; // Remove trailing colon
    }
}

// Parse hex string to UID bytes
static bool parse_uid_hex(const char* hex_str, uint8_t* uid, uint8_t* length) {
    uint8_t len = 0;
    char temp_str[3] = {0};
    
    for(uint8_t i = 0; i < strlen(hex_str) && len < 10; i++) {
        if(hex_str[i] == ':') continue;
        
        temp_str[0] = hex_str[i];
        if(i + 1 < strlen(hex_str) && hex_str[i + 1] != ':') {
            temp_str[1] = hex_str[i + 1];
            i++;
        } else {
            temp_str[1] = '0';
        }
        
        uid[len] = (uint8_t)strtol(temp_str, NULL, 16);
        len++;
    }
    
    *length = len;
    return len > 0;
}

// Simulate NFC card reading
static bool simulate_nfc_read(void) {
    if(!g_nfc_state.is_reading) return false;
    
    // Simulate successful read with 60% probability
    if(rand() % 10 < 6 && g_nfc_state.card_count < g_nfc_state.max_cards) {
        nfc_card_t* card = &g_nfc_state.cards[g_nfc_state.card_count];
        
        // Generate random card type
        card->card_type = rand() % 8;
        
        // Generate random UID based on card type
        switch(card->card_type) {
            case CARD_TYPE_MIFARE_1K:
            case CARD_TYPE_MIFARE_4K:
                card->uid_length = 4;
                break;
            case CARD_TYPE_MIFARE_ULTRALIGHT:
            case CARD_TYPE_NTAG_213:
            case CARD_TYPE_NTAG_215:
            case CARD_TYPE_NTAG_216:
                card->uid_length = 7;
                break;
            case CARD_TYPE_MIFARE_DESFIRE:
                card->uid_length = 4 + (rand() % 4); // 4-7 bytes
                break;
            case CARD_TYPE_ISO_15693:
                card->uid_length = 8;
                break;
            case CARD_TYPE_FELICA:
                card->uid_length = 8;
                break;
            default:
                card->uid_length = 4;
                break;
        }
        
        // Generate random UID
        for(uint8_t i = 0; i < card->uid_length; i++) {
            card->uid_bytes[i] = rand() % 256;
        }
        
        format_uid_hex(card->uid_bytes, card->uid_length, card->uid, sizeof(card->uid));
        
        // Generate ATQA and SAK (simplified)
        snprintf(card->atqa, sizeof(card->atqa), "%02X%02X", rand() % 256, rand() % 256);
        snprintf(card->sak, sizeof(card->sak), "%02X", rand() % 256);
        
        // Set sectors based on card type
        switch(card->card_type) {
            case CARD_TYPE_MIFARE_1K: card->sectors = 16; card->blocks_per_sector = 4; break;
            case CARD_TYPE_MIFARE_4K: card->sectors = 40; card->blocks_per_sector = 4; break;
            case CARD_TYPE_MIFARE_ULTRALIGHT: card->sectors = 16; card->blocks_per_sector = 4; break;
            case CARD_TYPE_NTAG_213: card->sectors = 45; card->blocks_per_sector = 4; break;
            case CARD_TYPE_NTAG_215: card->sectors = 135; card->blocks_per_sector = 4; break;
            case CARD_TYPE_NTAG_216: card->sectors = 231; card->blocks_per_sector = 4; break;
            default: card->sectors = 16; card->blocks_per_sector = 4; break;
        }
        
        card->is_emulated = false;
        card->last_read = furi_get_tick();
        card->read_count = 1;
        
        // Generate some sample data
        card->data_length = 16 + (rand() % 1008); // 16-1024 bytes
        for(uint16_t i = 0; i < card->data_length; i++) {
            card->data[i] = rand() % 256;
        }
        
        g_nfc_state.card_count++;
        g_nfc_state.successful_reads++;
        g_nfc_state.total_reads++;
        
        FURI_LOG_I("NFC", "Read card: %s (%s)", card->uid, card_type_to_string(card->card_type));
        return true;
    }
    
    g_nfc_state.total_reads++;
    g_nfc_state.failed_reads++;
    return false;
}

// Simulate card emulation
static bool simulate_card_emulation(nfc_card_t* card) {
    if(!card || !g_nfc_state.is_emulating) return false;
    
    // In real implementation, this would:
    // 1. Configure NFC hardware for card emulation
    // 2. Respond to reader commands with card data
    // 3. Handle authentication challenges
    // 4. Provide access to card data as requested
    
    FURI_LOG_D("NFC", "Emulating card: %s", card->uid);
    return true;
}

// Initialize reader
static void nfc_reader_init(void) {
    if(g_nfc_state.cards) {
        furi_free(g_nfc_state.cards);
    }
    
    g_nfc_state.max_cards = 20;
    g_nfc_state.cards = furi_alloc(sizeof(nfc_card_t) * g_nfc_state.max_cards);
    g_nfc_state.card_count = 0;
    g_nfc_state.is_reading = false;
    g_nfc_state.is_emulating = false;
    g_nfc_state.is_writing = false;
    g_nfc_state.selected_card = 0;
    g_nfc_state.total_reads = 0;
    g_nfc_state.successful_reads = 0;
    g_nfc_state.failed_reads = 0;
    g_nfc_state.current_uid_length = 4;
    g_nfc_state.show_card_details = false;
    g_nfc_state.show_data_dump = false;
    g_nfc_state.show_emulator_menu = false;
    g_nfc_state.emulation_mode = 0;
    
    memset(g_nfc_state.cards, 0, sizeof(nfc_card_t) * g_nfc_state.max_cards);
    memset(g_nfc_state.custom_uid, 0, sizeof(g_nfc_state.custom_uid));
}

// Start reading
static void nfc_start_reading(void) {
    g_nfc_state.is_reading = true;
    g_nfc_state.total_reads = 0;
    g_nfc_state.successful_reads = 0;
    g_nfc_state.failed_reads = 0;
    
    FURI_LOG_I("NFC", "Starting NFC card reading");
}

// Stop reading
static void nfc_stop_reading(void) {
    g_nfc_state.is_reading = false;
    FURI_LOG_I("NFC", "Stopped NFC card reading");
}

// NFC Reader Application
int32_t nfc_reader_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I("NFC", "Starting NFC/RFID Reader & Emulator");
    
    // Initialize GUI
    gui_t* gui = gui_alloc();
    if(!gui) {
        FURI_LOG_E("NFC", "Failed to allocate GUI");
        return 1;
    }
    
    // Create view port
    view_port_t* view_port = view_port_alloc();
    if(!view_port) {
        FURI_LOG_E("NFC", "Failed to allocate view port");
        gui_free(gui);
        return 1;
    }
    
    // Add to GUI
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Create canvas
    canvas_t* canvas = canvas_alloc(128, 64);
    if(!canvas) {
        FURI_LOG_E("NFC", "Failed to allocate canvas");
        view_port_free(view_port);
        gui_free(gui);
        return 1;
    }
    
    // Initialize reader
    nfc_reader_init();
    
    // Main application loop
    uint32_t frame_counter = 0;
    uint8_t scroll_offset = 0;
    
    while(1) {
        // Update reading
        if(g_nfc_state.is_reading) {
            simulate_nfc_read();
        }
        
        // Update emulation
        if(g_nfc_state.is_emulating && g_nfc_state.selected_card < g_nfc_state.card_count) {
            simulate_card_emulation(&g_nfc_state.cards[g_nfc_state.selected_card]);
        }
        
        // Update display every 5 frames
        if(frame_counter % 5 == 0) {
            canvas_clear(canvas, CanvasColorWhite);
            canvas_set_color(canvas, CanvasColorBlack);
            canvas_set_font(canvas, &font_8x11);
            
            if(g_nfc_state.show_emulator_menu) {
                // Show emulator menu
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Card Emulator");
                
                uint8_t y = 14;
                
                // Emulation mode
                const char* mode_str = "Static";
                switch(g_nfc_state.emulation_mode) {
                    case 0: mode_str = "Static"; break;
                    case 1: mode_str = "Dynamic"; break;
                    case 2: mode_str = "Clone"; break;
                }
                char mode_line[32];
                snprintf(mode_line, sizeof(mode_line), "Mode: %s", mode_str);
                canvas_draw_str(canvas, 2, y, mode_line);
                y += 10;
                
                // Custom UID
                if(strlen(g_nfc_state.custom_uid) > 0) {
                    canvas_draw_str(canvas, 2, y, "Custom UID:");
                    canvas_draw_str(canvas, 2, y + 8, g_nfc_state.custom_uid);
                    y += 18;
                }
                
                // Options
                canvas_draw_str(canvas, 2, y, "1. Static Emulation");
                y += 8;
                canvas_draw_str(canvas, 2, y, "2. Dynamic Response");
                y += 8;
                canvas_draw_str(canvas, 2, y, "3. Clone Card");
                y += 8;
                canvas_draw_str(canvas, 2, y, "4. Custom UID");
                
                canvas_draw_str(canvas, 2, 58, "BACK:Return");
                
            } else if(g_nfc_state.show_data_dump && g_nfc_state.selected_card < g_nfc_state.card_count) {
                // Show data dump
                nfc_card_t* card = &g_nfc_state.cards[g_nfc_state.selected_card];
                
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Card Data Dump");
                
                uint8_t y = 14;
                
                // Show first few bytes of data in hex
                char hex_line[32];
                for(uint8_t i = 0; i < 4 && i * 8 < card->data_length; i++) {
                    uint8_t start = i * 8;
                    uint8_t end = start + 8;
                    if(end > card->data_length) end = card->data_length;
                    
                    char line_prefix[8];
                    snprintf(line_prefix, sizeof(line_prefix), "%04X: ", start);
                    strcpy(hex_line, line_prefix);
                    
                    for(uint8_t j = start; j < end && strlen(hex_line) < 30; j++) {
                        char byte_str[4];
                        snprintf(byte_str, sizeof(byte_str), "%02X ", card->data[j]);
                        strcat(hex_line, byte_str);
                    }
                    
                    canvas_draw_str(canvas, 2, y, hex_line);
                    y += 8;
                }
                
                // Statistics
                char stats_str[32];
                snprintf(stats_str, sizeof(stats_str), "Size: %d bytes", card->data_length);
                canvas_draw_str(canvas, 2, y, stats_str);
                
                canvas_draw_str(canvas, 2, 58, "BACK:Return");
                
            } else if(g_nfc_state.show_card_details && g_nfc_state.selected_card < g_nfc_state.card_count) {
                // Show card details
                nfc_card_t* card = &g_nfc_state.cards[g_nfc_state.selected_card];
                
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "Card Details");
                
                uint8_t y = 14;
                
                // Card type and UID
                canvas_draw_str(canvas, 2, y, card_type_to_string(card->card_type));
                y += 10;
                canvas_draw_str(canvas, 2, y, "UID:");
                canvas_draw_str(canvas, 25, y, card->uid);
                y += 10;
                
                // ATQA and SAK
                char info_line[32];
                snprintf(info_line, sizeof(info_line), "ATQA:%s SAK:%s", card->atqa, card->sak);
                canvas_draw_str(canvas, 2, y, info_line);
                y += 10;
                
                // Sectors and blocks
                snprintf(info_line, sizeof(info_line), "Sectors:%d Blocks:%d", card->sectors, card->blocks_per_sector);
                canvas_draw_str(canvas, 2, y, info_line);
                y += 10;
                
                // Read count
                snprintf(info_line, sizeof(info_line), "Reads: %d", card->read_count);
                canvas_draw_str(canvas, 2, y, info_line);
                y += 10;
                
                // Data size
                snprintf(info_line, sizeof(info_line), "Data: %d bytes", card->data_length);
                canvas_draw_str(canvas, 2, y, info_line);
                
                canvas_draw_str(canvas, 2, 58, "BACK:Return OK:Data");
                
            } else {
                // Main reader screen
                canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "NFC/RFID Reader");
                
                // Status
                uint8_t y = 14;
                if(g_nfc_state.is_reading) {
                    canvas_draw_str(canvas, 2, y, "Reading...");
                    y += 10;
                    
                    // Animated reading indicator
                    uint8_t anim_pos = (frame_counter / 2) % 12;
                    for(uint8_t i = 0; i < 3; i++) {
                        uint8_t pos = (anim_pos + i * 4) % 12;
                        canvas_draw_box(canvas, 2 + pos * 10, y, 8, 3);
                    }
                    y += 6;
                } else if(g_nfc_state.is_emulating) {
                    canvas_draw_str(canvas, 2, y, "Emulating");
                    y += 10;
                    
                    // Emulation indicator
                    uint8_t anim_pos = (frame_counter / 3) % 8;
                    canvas_draw_box(canvas, 2 + anim_pos * 15, y, 12, 3);
                    y += 6;
                } else {
                    canvas_draw_str(canvas, 2, y, "Ready");
                    y += 10;
                }
                
                // Statistics
                char stats_str[32];
                if(g_nfc_state.is_reading) {
                    snprintf(stats_str, sizeof(stats_str), "Success: %lu/%lu", 
                            g_nfc_state.successful_reads, g_nfc_state.total_reads);
                } else {
                    snprintf(stats_str, sizeof(stats_str), "Cards: %d/20", g_nfc_state.card_count);
                }
                canvas_draw_str(canvas, 2, y, stats_str);
                y += 10;
                
                // Card list
                if(g_nfc_state.card_count > 0) {
                    uint8_t max_display = 2;
                    uint8_t start_idx = (scroll_offset < g_nfc_state.card_count) ? scroll_offset : 0;
                    
                    for(uint8_t i = 0; i < max_display && (start_idx + i) < g_nfc_state.card_count; i++) {
                        nfc_card_t* card = &g_nfc_state.cards[start_idx + i];
                        
                        // Show UID (truncated)
                        char short_uid[13];
                        strncpy(short_uid, card->uid, 12);
                        short_uid[12] = '\0';
                        
                        canvas_draw_str(canvas, 2, y + i * 8, short_uid);
                        
                        // Card type indicator
                        char type_char = card_type_to_string(card->card_type)[0];
                        char type_str[2] = {type_char, '\0'};
                        canvas_draw_str(canvas, 70, y + i * 8, type_str);
                        
                        // Read count indicator
                        if(card->read_count > 1) {
                            char count_str[8];
                            snprintf(count_str, sizeof(count_str), "x%d", card->read_count);
                            canvas_draw_str(canvas, 80, y + i * 8, count_str);
                        }
                    }
                } else {
                    canvas_draw_str(canvas, 2, y, "No cards detected");
                }
                
                // Controls
                if(g_nfc_state.is_reading) {
                    canvas_draw_str(canvas, 2, 58, "BACK:Stop");
                } else if(g_nfc_state.is_emulating) {
                    canvas_draw_str(canvas, 2, 58, "BACK:Stop");
                } else {
                    canvas_draw_str(canvas, 2, 58, "OK:Read UP:Select");
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
    if(g_nfc_state.cards) {
        furi_free(g_nfc_state.cards);
    }
    canvas_free(canvas);
    view_port_free(view_port);
    gui_free(gui);
    
    FURI_LOG_I("NFC", "NFC reader completed");
    FURI_LOG_I("NFC", "Total cards read: %d", g_nfc_state.card_count);
    FURI_LOG_I("NFC", "Successful reads: %lu", g_nfc_state.successful_reads);
    
    return 0;
}
