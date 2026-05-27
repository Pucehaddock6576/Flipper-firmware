/* Sub-GHz Protocol Analyzer - Advanced Signal Intelligence */

#include <protocols/subghz_analyzer.h>
#include <furi.h>
#include <furi_hal.h>
#include <string.h>
#include <math.h>

// Protocol database
static const subghz_protocol_entry_t protocol_database[] = {
    // Garage door protocols
    {"CAME", 433920000, SUBGHZ_MODULATION_ASK, 1000, subghz_decode_came},
    {"NICE FLO", 433920000, SUBGHZ_MODULATION_FSK, 2000, subghz_decode_nice_flo},
    {"SOMFY", 433420000, SUBGHZ_MODULATION_ASK, 4800, subghz_decode_somfy},
    {"BETT", 433920000, SUBGHZ_MODULATION_ASK, 1500, subghz_decode_bett},
    
    // Car remote protocols
    {"KEELOQ", 315000000, SUBGHZ_MODULATION_ASK, 4000, subghz_decode_keeloq},
    {"HCS300", 433920000, SUBGHZ_MODULATION_ASK, 4000, subghz_decode_hcs300},
    {"HCS200", 315000000, SUBGHZ_MODULATION_ASK, 4000, subghz_decode_hcs200},
    
    // Temperature/weather sensors
    {"OREGON", 433920000, SUBGHZ_MODULATION_ASK, 1024, subghz_decode_oregon},
    {"LACROSSE", 433920000, SUBGHZ_MODULATION_ASK, 2000, subghz_decode_lacrosse},
    {"ACURITE", 433920000, SUBGHZ_MODULATION_ASK, 2048, subghz_decode_acurite},
    
    // Doorbell protocols
    {"BYRON", 433920000, SUBGHZ_MODULATION_ASK, 1200, subghz_decode_byron},
    {"CHAMBERLAIN", 315000000, SUBGHZ_MODULATION_ASK, 2000, subghz_decode_chamberlain},
    {"HONEYWELL", 345000000, SUBGHZ_MODULATION_ASK, 1500, subghz_decode_honeywell},
    
    // Smart home protocols
    {"LIGHTWAVERF", 433920000, SUBGHZ_MODULATION_ASK, 2500, subghz_decode_lightwaverf},
    {"BLYNK", 433920000, SUBGHZ_MODULATION_ASK, 1000, subghz_decode_blynk},
    {"NEXA", 433920000, SUBGHZ_MODULATION_ASK, 1000, subghz_decode_nexa},
    {"ANSLUT", 433920000, SUBGHZ_MODULATION_ASK, 1200, subghz_decode_anslut},
    
    // Security systems
    {"ADEMCO", 345000000, SUBGHZ_MODULATION_ASK, 2000, subghz_decode_ademco},
    {"DSC", 433920000, SUBGHZ_MODULATION_ASK, 1500, subghz_decode_dsc},
    {"VISONIC", 433920000, SUBGHZ_MODULATION_ASK, 1000, subghz_decode_visonic},
    
    // Industrial protocols
    {"WIEGAND", 125000000, SUBGHZ_MODULATION_ASK, 4000, subghz_decode_wiegand},
    {"EM4100", 125000000, SUBGHZ_MODULATION_ASK, 4000, subghz_decode_em4100},
    {"HID", 125000000, SUBGHZ_MODULATION_FSK, 4000, subghz_decode_hid},
    
    // Custom/unknown
    {"UNKNOWN", 0, SUBGHZ_MODULATION_ASK, 0, subghz_decode_unknown}
};

#define PROTOCOL_COUNT (sizeof(protocol_database) / sizeof(subghz_protocol_entry_t))

// Signal processing
static float calculate_rssi(const uint16_t* samples, size_t count) {
    if(count == 0) return -100.0f;
    
    float sum = 0.0f;
    for(size_t i = 0; i < count; i++) {
        sum += 20.0f * log10f(fabsf((float)samples[i]) / 32768.0f);
    }
    
    return sum / count;
}

static float calculate_snr(const uint16_t* samples, size_t count) {
    if(count < 2) return 0.0f;
    
    float mean = 0.0f;
    for(size_t i = 0; i < count; i++) {
        mean += (float)samples[i];
    }
    mean /= count;
    
    float variance = 0.0f;
    for(size_t i = 0; i < count; i++) {
        float diff = (float)samples[i] - mean;
        variance += diff * diff;
    }
    variance /= count;
    
    float signal_power = mean * mean;
    float noise_power = variance;
    
    return noise_power > 0.0f ? 10.0f * log10f(signal_power / noise_power) : 0.0f;
}

static uint32_t estimate_frequency(const uint16_t* samples, size_t count, uint32_t sample_rate) {
    if(count < 10) return 0;
    
    // Simple zero-crossing frequency estimation
    uint32_t crossings = 0;
    int16_t prev_sample = (int16_t)samples[0];
    
    for(size_t i = 1; i < count; i++) {
        int16_t current_sample = (int16_t)samples[i];
        if((prev_sample < 0 && current_sample >= 0) || (prev_sample >= 0 && current_sample < 0)) {
            crossings++;
        }
        prev_sample = current_sample;
    }
    
    return (crossings * sample_rate) / (2 * count);
}

static uint32_t estimate_period(const uint16_t* samples, size_t count, uint32_t sample_rate) {
    uint32_t frequency = estimate_frequency(samples, count, sample_rate);
    return frequency > 0 ? sample_rate / frequency : 0;
}

// Protocol detection
static bool detect_modulation(const uint16_t* samples, size_t count, subghz_modulation_t* modulation) {
    if(count < 100) return false;
    
    // Analyze amplitude characteristics
    uint32_t amplitude_changes = 0;
    int16_t prev_amplitude = abs((int16_t)samples[0]);
    
    for(size_t i = 1; i < count; i++) {
        int16_t current_amplitude = abs((int16_t)samples[i]);
        if(abs(current_amplitude - prev_amplitude) > prev_amplitude / 2) {
            amplitude_changes++;
        }
        prev_amplitude = current_amplitude;
    }
    
    float change_ratio = (float)amplitude_changes / count;
    
    if(change_ratio > 0.3f) {
        *modulation = SUBGHZ_MODULATION_ASK;
    } else if(change_ratio > 0.1f) {
        *modulation = SUBGHZ_MODULATION_FSK;
    } else {
        *modulation = SUBGHZ_MODULATION_PSK;
    }
    
    return true;
}

static const subghz_protocol_entry_t* identify_protocol(const subghz_signal_t* signal) {
    const subghz_protocol_entry_t* best_match = NULL;
    float best_score = 0.0f;
    
    for(size_t i = 0; i < PROTOCOL_COUNT; i++) {
        const subghz_protocol_entry_t* protocol = &protocol_database[i];
        
        // Skip frequency check for unknown protocol
        if(protocol->frequency > 0 && abs((int32_t)signal->frequency - (int32_t)protocol->frequency) > 1000000) {
            continue;
        }
        
        // Check modulation match
        if(protocol->modulation != SUBGHZ_MODULATION_UNKNOWN && 
           protocol->modulation != signal->modulation) {
            continue;
        }
        
        // Check data rate similarity
        if(protocol->data_rate > 0 && signal->data_rate > 0) {
            float rate_diff = fabsf((float)signal->data_rate - (float)protocol->data_rate) / protocol->data_rate;
            if(rate_diff > 0.2f) { // 20% tolerance
                continue;
            }
        }
        
        // Calculate match score
        float score = 1.0f;
        
        // Frequency score
        if(protocol->frequency > 0) {
            float freq_diff = fabsf((float)signal->frequency - (float)protocol->frequency) / protocol->frequency;
            score *= (1.0f - freq_diff);
        }
        
        // Modulation score
        if(protocol->modulation != SUBGHZ_MODULATION_UNKNOWN) {
            score *= (protocol->modulation == signal->modulation) ? 1.0f : 0.5f;
        }
        
        // Data rate score
        if(protocol->data_rate > 0 && signal->data_rate > 0) {
            float rate_diff = fabsf((float)signal->data_rate - (float)protocol->data_rate) / protocol->data_rate;
            score *= (1.0f - rate_diff);
        }
        
        if(score > best_score) {
            best_score = score;
            best_match = protocol;
        }
    }
    
    return best_match;
}

// Analyzer implementation
subghz_analyzer_t* subghz_analyzer_alloc(void) {
    subghz_analyzer_t* analyzer = furi_alloc(sizeof(subghz_analyzer_t));
    if(!analyzer) return NULL;
    
    memset(analyzer, 0, sizeof(subghz_analyzer_t));
    
    analyzer->capture_buffer = furi_alloc(sizeof(uint16_t) * MAX_CAPTURE_SIZE);
    analyzer->capture_size = 0;
    analyzer->is_capturing = false;
    analyzer->current_signal = furi_alloc(sizeof(subghz_signal_t));
    analyzer->detected_protocol = NULL;
    analyzer->callback = NULL;
    analyzer->context = NULL;
    
    // Initialize statistics
    analyzer->stats.signals_captured = 0;
    analyzer->stats.protocols_detected = 0;
    analyzer->stats.false_positives = 0;
    analyzer->stats.average_rssi = -100.0f;
    analyzer->stats.average_snr = 0.0f;
    
    return analyzer;
}

void subghz_analyzer_free(subghz_analyzer_t* analyzer) {
    if(!analyzer) return;
    
    furi_free(analyzer->capture_buffer);
    furi_free(analyzer->current_signal);
    furi_free(analyzer);
}

void subghz_analyzer_start_capture(subghz_analyzer_t* analyzer) {
    if(!analyzer) return;
    
    analyzer->is_capturing = true;
    analyzer->capture_size = 0;
    
    FURI_LOG_I("SUBGHZ", "Starting signal capture");
    
    // Start hardware capture
    FuriHalSubGhz* subghz = furi_hal_subghz_init();
    furi_hal_subghz_receive_start(subghz);
}

void subghz_analyzer_stop_capture(subghz_analyzer_t* analyzer) {
    if(!analyzer) return;
    
    analyzer->is_capturing = false;
    
    FURI_LOG_I("SUBGHZ", "Stopping signal capture");
    
    // Stop hardware capture
    FuriHalSubGhz* subghz = furi_hal_subghz_init();
    furi_hal_subghz_receive_data(subghz, analyzer->capture_buffer, &analyzer->capture_size);
}

void subghz_analyzer_update(subghz_analyzer_t* analyzer) {
    if(!analyzer || !analyzer->is_capturing) return;
    
    // Read more data from hardware
    FuriHalSubGhz* subghz = furi_hal_subghz_init();
    size_t new_samples = furi_hal_subghz_receive_data(subghz, 
                                                      &analyzer->capture_buffer[analyzer->capture_size], 
                                                      MAX_CAPTURE_SIZE - analyzer->capture_size);
    
    analyzer->capture_size += new_samples;
    
    // If buffer is full or we have enough data, analyze
    if(analyzer->capture_size >= MIN_SIGNAL_LENGTH || analyzer->capture_size >= MAX_CAPTURE_SIZE) {
        subghz_analyzer_analyze_signal(analyzer);
        analyzer->capture_size = 0; // Reset for next capture
    }
}

void subghz_analyzer_analyze_signal(subghz_analyzer_t* analyzer) {
    if(!analyzer || analyzer->capture_size < MIN_SIGNAL_LENGTH) return;
    
    // Analyze signal characteristics
    analyzer->current_signal->rssi = calculate_rssi(analyzer->capture_buffer, analyzer->capture_size);
    analyzer->current_signal->snr = calculate_snr(analyzer->capture_buffer, analyzer->capture_size);
    analyzer->current_signal->frequency = estimate_frequency(analyzer->capture_buffer, analyzer->capture_size, SAMPLE_RATE);
    analyzer->current_signal->period = estimate_period(analyzer->capture_buffer, analyzer->capture_size, SAMPLE_RATE);
    analyzer->current_signal->data_rate = analyzer->current_signal->frequency > 0 ? 
                                           analyzer->current_signal->frequency * 2 : 0; // Approximation
    
    detect_modulation(analyzer->capture_buffer, analyzer->capture_size, &analyzer->current_signal->modulation);
    
    // Identify protocol
    analyzer->detected_protocol = identify_protocol(analyzer->current_signal);
    
    // Update statistics
    analyzer->stats.signals_captured++;
    analyzer->stats.average_rssi = (analyzer->stats.average_rssi * (analyzer->stats.signals_captured - 1) + 
                                   analyzer->current_signal->rssi) / analyzer->stats.signals_captured;
    analyzer->stats.average_snr = (analyzer->stats.average_snr * (analyzer->stats.signals_captured - 1) + 
                                  analyzer->current_signal->snr) / analyzer->stats.signals_captured;
    
    if(analyzer->detected_protocol && analyzer->detected_protocol->decode_function) {
        analyzer->stats.protocols_detected++;
        
        // Decode the signal
        bool decoded = analyzer->detected_protocol->decode_function(
            analyzer->current_signal, 
            &analyzer->decoded_data
        );
        
        if(decoded) {
            FURI_LOG_I("SUBGHZ", "Detected protocol: %s", analyzer->detected_protocol->name);
            
            // Call callback if set
            if(analyzer->callback) {
                analyzer->callback(analyzer, analyzer->context);
            }
        } else {
            analyzer->stats.false_positives++;
        }
    }
    
    // Log signal info
    FURI_LOG_D("SUBGHZ", "Signal: RSSI=%.1fdB, SNR=%.1fdB, Freq=%luHz, Mod=%d", 
               analyzer->current_signal->rssi, analyzer->current_signal->snr,
               analyzer->current_signal->frequency, analyzer->current_signal->modulation);
}

void subghz_analyzer_set_callback(subghz_analyzer_t* analyzer, subghz_analyzer_callback_t callback, void* context) {
    if(analyzer) {
        analyzer->callback = callback;
        analyzer->context = context;
    }
}

subghz_signal_t* subghz_analyzer_get_current_signal(subghz_analyzer_t* analyzer) {
    return analyzer ? analyzer->current_signal : NULL;
}

const subghz_protocol_entry_t* subghz_analyzer_get_detected_protocol(subghz_analyzer_t* analyzer) {
    return analyzer ? analyzer->detected_protocol : NULL;
}

subghz_decoded_data_t* subghz_analyzer_get_decoded_data(subghz_analyzer_t* analyzer) {
    return analyzer ? &analyzer->decoded_data : NULL;
}

subghz_stats_t* subghz_analyzer_get_stats(subghz_analyzer_t* analyzer) {
    return analyzer ? &analyzer->stats : NULL;
}

void subghz_analyzer_reset_stats(subghz_analyzer_t* analyzer) {
    if(!analyzer) return;
    
    analyzer->stats.signals_captured = 0;
    analyzer->stats.protocols_detected = 0;
    analyzer->stats.false_positives = 0;
    analyzer->stats.average_rssi = -100.0f;
    analyzer->stats.average_snr = 0.0f;
}

// Protocol decode functions (simplified implementations)
bool subghz_decode_came(const subghz_signal_t* signal, subghz_decoded_data_t* data) {
    if(!signal || !data) return false;
    
    // CAME protocol: 13-bit rolling code
    if(signal->data_rate < 800 || signal->data_rate > 1200) return false;
    
    // Simplified CAME decoding
    data->data_length = 13;
    data->data = 0x1234; // Dummy data
    data->rolling_code = 0x1234;
    data->button_id = 1;
    data->battery_ok = true;
    
    return true;
}

bool subghz_decode_nice_flo(const subghz_signal_t* signal, subghz_decoded_data_t* data) {
    if(!signal || !data) return false;
    
    // Nice Flo protocol: 64-bit encrypted
    if(signal->data_rate < 1800 || signal->data_rate > 2200) return false;
    
    data->data_length = 64;
    data->data = 0x123456789ABCDEF0ULL; // Dummy encrypted data
    data->rolling_code = 0x12345678;
    data->button_id = 2;
    data->battery_ok = true;
    
    return true;
}

bool subghz_decode_somfy(const subghz_signal_t* signal, subghz_decoded_data_t* data) {
    if(!signal || !data) return false;
    
    // Somfy protocol: 80-bit
    if(signal->data_rate < 4600 || signal->data_rate > 5000) return false;
    
    data->data_length = 80;
    data->data = 0x123456789ABCDEF01234ULL; // Dummy data
    data->rolling_code = 0x123456;
    data->button_id = 3;
    data->battery_ok = false; // Somfy often reports battery
    
    return true;
}

bool subghz_decode_unknown(const subghz_signal_t* signal, subghz_decoded_data_t* data) {
    if(!signal || !data) return false;
    
    // Store raw signal data
    data->data_length = signal->period > 0 ? signal->period * 8 : 256;
    data->data = 0; // Would store actual bits in real implementation
    data->rolling_code = 0;
    data->button_id = 0;
    data->battery_ok = true;
    
    return true;
}

// Add other decode functions...
bool subghz_decode_bett(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_keeloq(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_hcs300(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_hcs200(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_oregon(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_lacrosse(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_acurite(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_byron(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_chamberlain(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_honeywell(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_lightwaverf(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_blynk(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_nexa(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_anslut(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_ademco(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_dsc(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_visonic(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_wiegand(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_em4100(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
bool subghz_decode_hid(const subghz_signal_t* signal, subghz_decoded_data_t* data) { return subghz_decode_unknown(signal, data); }
