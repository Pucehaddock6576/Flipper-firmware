/* AI Signal Detector Header - Machine Learning for Protocol Recognition */

#ifndef PROTOCOLS_AI_SIGNAL_DETECTOR_H
#define PROTOCOLS_AI_SIGNAL_DETECTOR_H

#include <stdint.h>
#include <stdbool.h>
#include <furi.h>
#include "subghz_analyzer.h"

// AI Constants
#define AI_FEATURE_COUNT 16
#define AI_HIDDEN_SIZE 32
#define AI_MAX_PROTOCOLS 64
#define AI_MAX_PATTERNS 128

// Signal features for AI processing
typedef struct {
    float mean;                    // Time domain mean
    float std_dev;                 // Standard deviation
    float peak_to_peak;            // Peak-to-peak amplitude
    float rms;                     // Root mean square
    float spectral_centroid;       // Frequency domain centroid
    float spectral_bandwidth;      // Spectral bandwidth
    float spectral_rolloff;        // Spectral rolloff
    float modulation_index;        // Modulation index
    float zero_crossing_rate;      // Zero crossing rate
    float snr;                     // Signal-to-noise ratio
    float rssi;                    // Signal strength
    signal_quality_t signal_quality; // Overall signal quality
    uint32_t pulse_width;          // Pulse width in microseconds
    uint32_t pulse_period;         // Pulse period in microseconds
    float duty_cycle;              // Duty cycle
    float entropy;                 // Signal entropy
} ai_signal_features_t;

// Pattern template for supervised learning
typedef struct {
    char protocol_name[32];
    float mean;
    float std_dev;
    float center_frequency;
    float bandwidth;
    subghz_modulation_t modulation;
    float mean_tolerance;
    float std_dev_tolerance;
    float frequency_tolerance;
    signal_quality_t min_quality;
    uint32_t min_pulse_width;
    uint32_t max_pulse_width;
    float typical_duty_cycle;
} ai_pattern_template_t;

// Detection result
typedef struct {
    char protocol_name[32];
    float confidence;              // Overall confidence (0.0-1.0)
    float pattern_similarity;      // Pattern matching confidence
    float neural_confidence;       // Neural network confidence
    ai_signal_features_t features; // Extracted features
    bool is_novel;                 // True if this appears to be a new protocol
    float novelty_score;           // How novel this signal is (0.0-1.0)
} ai_detection_result_t;

// Detector statistics
typedef struct {
    uint32_t signals_processed;
    uint32_t protocols_identified;
    uint32_t false_positives;
    uint32_t novel_signals_detected;
    float average_confidence;
    uint32_t learning_iterations;
    float model_accuracy;
    uint32_t patterns_learned;
} ai_detector_stats_t;

// Forward declarations
typedef struct ai_neural_network ai_neural_network_t;

// AI Signal Detector
typedef struct {
    ai_neural_network_t* neural_network;
    ai_pattern_template_t* patterns;
    int pattern_count;
    bool learning_enabled;
    float confidence_threshold;
    ai_detector_stats_t stats;
} ai_signal_detector_t;

// Detector management
ai_signal_detector_t* ai_signal_detector_alloc(void);
void ai_signal_detector_free(ai_signal_detector_t* detector);

// Pattern management
void ai_signal_detector_add_pattern(ai_signal_detector_t* detector, const ai_pattern_template_t* pattern);

// Detection
ai_detection_result_t* ai_signal_detector_detect(ai_signal_detector_t* detector, const subghz_signal_t* signal);

// Learning
void ai_signal_detector_update_learning(ai_signal_detector_t* detector, const subghz_signal_t* signal, int pattern_index, float confidence);
void ai_signal_detector_enable_learning(ai_signal_detector_t* detector, bool enabled);
void ai_signal_detector_set_confidence_threshold(ai_signal_detector_t* detector, float threshold);

// Statistics
ai_detector_stats_t* ai_signal_detector_get_stats(ai_signal_detector_t* detector);
void ai_signal_detector_reset_stats(ai_signal_detector_t* detector);

// Model persistence
bool ai_signal_detector_save_model(ai_signal_detector_t* detector, const char* filename);
bool ai_signal_detector_load_model(ai_signal_detector_t* detector, const char* filename);

// Utility functions
void ai_detection_result_free(ai_detection_result_t* result);

// Predefined pattern templates
static const ai_pattern_template_t AI_PATTERN_CAME = {
    .protocol_name = "CAME",
    .mean = 0.0f,
    .std_dev = 8192.0f,
    .center_frequency = 433920000.0f,
    .bandwidth = 1000.0f,
    .modulation = SUBGHZ_MODULATION_ASK,
    .mean_tolerance = 16384.0f,
    .std_dev_tolerance = 4096.0f,
    .frequency_tolerance = 50000.0f,
    .min_quality = SIGNAL_QUALITY_FAIR,
    .min_pulse_width = 300,
    .max_pulse_width = 600,
    .typical_duty_cycle = 0.5f
};

static const ai_pattern_template_t AI_PATTERN_NICE_FLO = {
    .protocol_name = "NICE_FLO",
    .mean = 0.0f,
    .std_dev = 12288.0f,
    .center_frequency = 433920000.0f,
    .bandwidth = 2000.0f,
    .modulation = SUBGHZ_MODULATION_FSK,
    .mean_tolerance = 24576.0f,
    .std_dev_tolerance = 6144.0f,
    .frequency_tolerance = 100000.0f,
    .min_quality = SIGNAL_QUALITY_GOOD,
    .min_pulse_width = 200,
    .max_pulse_width = 400,
    .typical_duty_cycle = 0.5f
};

static const ai_pattern_template_t AI_PATTERN_SOMFY = {
    .protocol_name = "SOMFY",
    .mean = 0.0f,
    .std_dev = 16384.0f,
    .center_frequency = 433420000.0f,
    .bandwidth = 4800.0f,
    .modulation = SUBGHZ_MODULATION_ASK,
    .mean_tolerance = 32768.0f,
    .std_dev_tolerance = 8192.0f,
    .frequency_tolerance = 100000.0f,
    .min_quality = SIGNAL_QUALITY_GOOD,
    .min_pulse_width = 100,
    .max_pulse_width = 300,
    .typical_duty_cycle = 0.3f
};

static const ai_pattern_template_t AI_PATTERN_KEELOQ = {
    .protocol_name = "KEELOQ",
    .mean = 0.0f,
    .std_dev = 10240.0f,
    .center_frequency = 315000000.0f,
    .bandwidth = 4000.0f,
    .modulation = SUBGHZ_MODULATION_ASK,
    .mean_tolerance = 20480.0f,
    .std_dev_tolerance = 5120.0f,
    .frequency_tolerance = 50000.0f,
    .min_quality = SIGNAL_QUALITY_FAIR,
    .min_pulse_width = 200,
    .max_pulse_width = 500,
    .typical_duty_cycle = 0.4f
};

// Novelty detection
static inline bool ai_is_novel_signal(const ai_detection_result_t* result) {
    return result && (result->confidence < 0.5f || result->novelty_score > 0.7f);
}

// Confidence evaluation
static inline bool ai_is_high_confidence(const ai_detection_result_t* result) {
    return result && result->confidence >= 0.8f;
}

static inline bool ai_is_medium_confidence(const ai_detection_result_t* result) {
    return result && result->confidence >= 0.6f && result->confidence < 0.8f;
}

static inline bool ai_is_low_confidence(const ai_detection_result_t* result) {
    return result && result->confidence < 0.6f;
}

// Feature normalization utilities
static inline float ai_normalize_feature(float value, float min_val, float max_val) {
    if(max_val <= min_val) return 0.0f;
    float normalized = (value - min_val) / (max_val - min_val);
    return normalized < 0.0f ? 0.0f : (normalized > 1.0f ? 1.0f : normalized);
}

static inline float ai_normalize_rssi(float rssi) {
    return ai_normalize_feature(rssi, -100.0f, 0.0f);
}

static inline float ai_normalize_snr(float snr) {
    return ai_normalize_feature(snr, 0.0f, 50.0f);
}

static inline float ai_normalize_frequency(float freq) {
    return ai_normalize_feature(freq, 300000000.0f, 1000000000.0f);
}

// Learning rate adaptation
static inline float ai_adapt_learning_rate(float base_rate, uint32_t iteration_count, uint32_t max_iterations) {
    if(iteration_count >= max_iterations) return 0.01f;
    return base_rate * (1.0f - (float)iteration_count / max_iterations);
}

// Model quality metrics
static inline float ai_calculate_accuracy(const ai_detector_stats_t* stats) {
    if(!stats || stats->signals_processed == 0) return 0.0f;
    return (float)stats->protocols_identified / stats->signals_processed;
}

static inline float ai_calculate_precision(const ai_detector_stats_t* stats) {
    if(!stats || stats->protocols_identified + stats->false_positives == 0) return 0.0f;
    return (float)stats->protocols_identified / (stats->protocols_identified + stats->false_positives);
}

static inline float ai_calculate_false_positive_rate(const ai_detector_stats_t* stats) {
    if(!stats || stats->signals_processed == 0) return 0.0f;
    return (float)stats->false_positives / stats->signals_processed;
}

#endif // PROTOCOLS_AI_SIGNAL_DETECTOR_H
