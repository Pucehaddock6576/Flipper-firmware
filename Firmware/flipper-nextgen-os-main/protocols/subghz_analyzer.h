/* Sub-GHz Protocol Analyzer Header - Advanced Signal Intelligence */

#ifndef PROTOCOLS_SUBGHZ_ANALYZER_H
#define PROTOCOLS_SUBGHZ_ANALYZER_H

#include <stdint.h>
#include <stdbool.h>
#include <furi.h>

// Constants
#define MAX_CAPTURE_SIZE 8192
#define MIN_SIGNAL_LENGTH 100
#define SAMPLE_RATE 1000000 // 1MHz
#define MAX_PROTOCOLS 64

// Modulation types
typedef enum {
    SUBGHZ_MODULATION_UNKNOWN = 0,
    SUBGHZ_MODULATION_ASK = 1,
    SUBGHZ_MODULATION_FSK = 2,
    SUBGHZ_MODULATION_PSK = 3,
    SUBGHZ_MODULATION_OOK = 4
} subghz_modulation_t;

// Signal structure
typedef struct {
    float rssi;           // Signal strength in dB
    float snr;            // Signal-to-noise ratio
    uint32_t frequency;   // Center frequency in Hz
    uint32_t period;      // Signal period in microseconds
    uint32_t data_rate;   // Data rate in Hz
    subghz_modulation_t modulation;
    uint16_t* samples;    // Raw sample data
    size_t sample_count;  // Number of samples
} subghz_signal_t;

// Decoded data structure
typedef struct {
    uint64_t data;        // Decoded data
    uint32_t data_length; // Number of bits
    uint32_t rolling_code; // Rolling code if applicable
    uint8_t button_id;    // Button ID
    bool battery_ok;      // Battery status
    char protocol_name[16]; // Protocol name
    uint8_t raw_data[256]; // Raw decoded bytes
    size_t raw_data_length; // Raw data length
} subghz_decoded_data_t;

// Protocol entry
typedef struct {
    const char* name;
    uint32_t frequency;
    subghz_modulation_t modulation;
    uint32_t data_rate;
    bool (*decode_function)(const subghz_signal_t* signal, subghz_decoded_data_t* data);
} subghz_protocol_entry_t;

// Statistics
typedef struct {
    uint32_t signals_captured;
    uint32_t protocols_detected;
    uint32_t false_positives;
    float average_rssi;
    float average_snr;
    uint32_t total_capture_time;
} subghz_stats_t;

// Callback type
typedef struct subghz_analyzer subghz_analyzer_t;
typedef void (*subghz_analyzer_callback_t)(subghz_analyzer_t* analyzer, void* context);

// Analyzer structure
typedef struct subghz_analyzer {
    uint16_t* capture_buffer;
    size_t capture_size;
    bool is_capturing;
    
    subghz_signal_t* current_signal;
    const subghz_protocol_entry_t* detected_protocol;
    subghz_decoded_data_t decoded_data;
    
    subghz_stats_t stats;
    
    subghz_analyzer_callback_t callback;
    void* context;
} subghz_analyzer_t;

// Analyzer management
subghz_analyzer_t* subghz_analyzer_alloc(void);
void subghz_analyzer_free(subghz_analyzer_t* analyzer);

// Capture control
void subghz_analyzer_start_capture(subghz_analyzer_t* analyzer);
void subghz_analyzer_stop_capture(subghz_analyzer_t* analyzer);
void subghz_analyzer_update(subghz_analyzer_t* analyzer);

// Analysis
void subghz_analyzer_analyze_signal(subghz_analyzer_t* analyzer);

// Callbacks
void subghz_analyzer_set_callback(subghz_analyzer_t* analyzer, subghz_analyzer_callback_t callback, void* context);

// Data access
subghz_signal_t* subghz_analyzer_get_current_signal(subghz_analyzer_t* analyzer);
const subghz_protocol_entry_t* subghz_analyzer_get_detected_protocol(subghz_analyzer_t* analyzer);
subghz_decoded_data_t* subghz_analyzer_get_decoded_data(subghz_analyzer_t* analyzer);
subghz_stats_t* subghz_analyzer_get_stats(subghz_analyzer_t* analyzer);

// Statistics
void subghz_analyzer_reset_stats(subghz_analyzer_t* analyzer);

// Protocol decode functions
bool subghz_decode_came(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_nice_flo(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_somfy(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_bett(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_keeloq(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_hcs300(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_hcs200(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_oregon(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_lacrosse(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_acurite(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_byron(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_chamberlain(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_honeywell(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_lightwaverf(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_blynk(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_nexa(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_anslut(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_ademco(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_dsc(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_visonic(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_wiegand(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_em4100(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_hid(const subghz_signal_t* signal, subghz_decoded_data_t* data);
bool subghz_decode_unknown(const subghz_signal_t* signal, subghz_decoded_data_t* data);

// Frequency bands
typedef enum {
    SUBGHZ_BAND_315MHZ = 315000000,
    SUBGHZ_BAND_433MHZ = 433920000,
    SUBGHZ_BAND_868MHZ = 868000000,
    SUBGHZ_BAND_915MHZ = 915000000
} subghz_frequency_band_t;

// Common frequency ranges
static const struct {
    uint32_t start_freq;
    uint32_t end_freq;
    const char* name;
} frequency_bands[] = {
    {300000000, 350000000, "300-350MHz"},
    {380000000, 450000000, "380-450MHz"},
    {779000000, 928000000, "779-928MHz"},
    {2400000000, 2500000000, "2.4GHz"}
};

// Signal quality metrics
typedef enum {
    SIGNAL_QUALITY_EXCELLENT = 4,
    SIGNAL_QUALITY_GOOD = 3,
    SIGNAL_QUALITY_FAIR = 2,
    SIGNAL_QUALITY_POOR = 1,
    SIGNAL_QUALITY_UNUSABLE = 0
} signal_quality_t;

static inline signal_quality_t subghz_evaluate_signal_quality(float rssi, float snr) {
    if(rssi > -50.0f && snr > 20.0f) return SIGNAL_QUALITY_EXCELLENT;
    if(rssi > -70.0f && snr > 10.0f) return SIGNAL_QUALITY_GOOD;
    if(rssi > -85.0f && snr > 5.0f) return SIGNAL_QUALITY_FAIR;
    if(rssi > -95.0f && snr > 0.0f) return SIGNAL_QUALITY_POOR;
    return SIGNAL_QUALITY_UNUSABLE;
}

#endif // PROTOCOLS_SUBGHZ_ANALYZER_H
