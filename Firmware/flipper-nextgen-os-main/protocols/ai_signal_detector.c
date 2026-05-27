/* AI-Driven Signal Detection - Machine Learning for Protocol Recognition */

#include <protocols/ai_signal_detector.h>
#include <protocols/subghz_analyzer.h>
#include <furi.h>
#include <math.h>
#include <string.h>

// Feature extraction
static void extract_features(const subghz_signal_t* signal, ai_signal_features_t* features) {
    if(!signal || !features) return;
    
    // Time domain features
    features->mean = 0.0f;
    features->std_dev = 0.0f;
    features->peak_to_peak = 0.0f;
    features->rms = 0.0f;
    
    if(signal->sample_count > 0) {
        // Calculate mean
        for(size_t i = 0; i < signal->sample_count; i++) {
            features->mean += (float)signal->samples[i];
        }
        features->mean /= signal->sample_count;
        
        // Calculate standard deviation and RMS
        float sum_sq = 0.0f;
        float min_val = (float)signal->samples[0];
        float max_val = (float)signal->samples[0];
        
        for(size_t i = 0; i < signal->sample_count; i++) {
            float sample = (float)signal->samples[i];
            float diff = sample - features->mean;
            sum_sq += diff * diff;
            
            if(sample < min_val) min_val = sample;
            if(sample > max_val) max_val = sample;
        }
        
        features->std_dev = sqrtf(sum_sq / signal->sample_count);
        features->peak_to_peak = max_val - min_val;
        features->rms = sqrtf(sum_sq / signal->sample_count + features->mean * features->mean);
    }
    
    // Frequency domain features (simplified)
    features->spectral_centroid = signal->frequency;
    features->spectral_bandwidth = signal->data_rate;
    features->spectral_rolloff = signal->frequency + signal->data_rate;
    
    // Modulation features
    features->modulation_index = (signal->modulation == SUBGHZ_MODULATION_FSK) ? 1.0f : 0.5f;
    features->zero_crossing_rate = 0.0f;
    
    // Calculate zero crossing rate
    if(signal->sample_count > 1) {
        uint32_t crossings = 0;
        for(size_t i = 1; i < signal->sample_count; i++) {
            if((signal->samples[i-1] >= 0 && signal->samples[i] < 0) ||
               (signal->samples[i-1] < 0 && signal->samples[i] >= 0)) {
                crossings++;
            }
        }
        features->zero_crossing_rate = (float)crossings / signal->sample_count;
    }
    
    // Signal quality features
    features->snr = signal->snr;
    features->rssi = signal->rssi;
    features->signal_quality = subghz_evaluate_signal_quality(signal->rssi, signal->snr);
    
    // Timing features
    features->pulse_width = signal->period > 0 ? signal->period / 2 : 0;
    features->pulse_period = signal->period;
    features->duty_cycle = 0.5f; // Approximation
    
    // Entropy (simplified)
    features->entropy = 0.0f;
    if(signal->sample_count > 0) {
        uint32_t histogram[16] = {0};
        for(size_t i = 0; i < signal->sample_count; i++) {
            uint8_t bin = (signal->samples[i] >> 12) & 0xF; // 4-bit bins
            histogram[bin]++;
        }
        
        for(int i = 0; i < 16; i++) {
            if(histogram[i] > 0) {
                float p = (float)histogram[i] / signal->sample_count;
                features->entropy -= p * log2f(p);
            }
        }
    }
}

// Neural network implementation (simplified)
typedef struct ai_neural_network_s {
    float weights[AI_FEATURE_COUNT][AI_HIDDEN_SIZE];
    float hidden_bias[AI_HIDDEN_SIZE];
    float output_weights[AI_HIDDEN_SIZE][AI_MAX_PROTOCOLS];
    float output_bias[AI_MAX_PROTOCOLS];
} ai_neural_network_s;

static float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

static float relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

static void neural_network_forward(const ai_neural_network_s* nn, const float* input, float* output) {
    float hidden[AI_HIDDEN_SIZE];
    
    // Hidden layer
    for(int i = 0; i < AI_HIDDEN_SIZE; i++) {
        hidden[i] = nn->hidden_bias[i];
        for(int j = 0; j < AI_FEATURE_COUNT; j++) {
            hidden[i] += input[j] * nn->weights[j][i];
        }
        hidden[i] = relu(hidden[i]);
    }
    
    // Output layer
    for(int i = 0; i < AI_MAX_PROTOCOLS; i++) {
        output[i] = nn->output_bias[i];
        for(int j = 0; j < AI_HIDDEN_SIZE; j++) {
            output[i] += hidden[j] * nn->output_weights[j][i];
        }
        output[i] = sigmoid(output[i]);
    }
}

// Pattern matching
static float calculate_pattern_similarity(const ai_signal_features_t* features, const ai_pattern_template_t* template) {
    if(!features || !template) return 0.0f;
    
    float similarity = 0.0f;
    float total_weight = 0.0f;
    
    // Compare mean
    if(template->mean_tolerance > 0.0f) {
        float diff = fabsf(features->mean - template->mean);
        float weight = 1.0f / template->mean_tolerance;
        similarity += weight * expf(-diff * diff / (2.0f * template->mean_tolerance * template->mean_tolerance));
        total_weight += weight;
    }
    
    // Compare standard deviation
    if(template->std_dev_tolerance > 0.0f) {
        float diff = fabsf(features->std_dev - template->std_dev);
        float weight = 1.0f / template->std_dev_tolerance;
        similarity += weight * expf(-diff * diff / (2.0f * template->std_dev_tolerance * template->std_dev_tolerance));
        total_weight += weight;
    }
    
    // Compare frequency
    if(template->frequency_tolerance > 0.0f) {
        float diff = fabsf(features->spectral_centroid - template->center_frequency);
        float weight = 1.0f / template->frequency_tolerance;
        similarity += weight * expf(-diff * diff / (2.0f * template->frequency_tolerance * template->frequency_tolerance));
        total_weight += weight;
    }
    
    // Compare modulation
    if(template->modulation != SUBGHZ_MODULATION_UNKNOWN) {
        float weight = 2.0f; // Higher weight for modulation match
        similarity += weight * (features->modulation_index > 0.5f ? 1.0f : 0.0f);
        total_weight += weight;
    }
    
    // Compare signal quality
    if(template->min_quality > 0) {
        float weight = 1.0f;
        similarity += weight * (features->signal_quality >= template->min_quality ? 1.0f : 0.0f);
        total_weight += weight;
    }
    
    return total_weight > 0.0f ? similarity / total_weight : 0.0f;
}

// AI Detector implementation
ai_signal_detector_t* ai_signal_detector_alloc(void) {
    ai_signal_detector_t* detector = furi_alloc(sizeof(ai_signal_detector_t));
    if(!detector) return NULL;
    
    detector->neural_network = furi_alloc(sizeof(ai_neural_network_s));
    detector->patterns = furi_alloc(sizeof(ai_pattern_template_t) * AI_MAX_PATTERNS);
    detector->pattern_count = 0;
    detector->learning_enabled = true;
    detector->confidence_threshold = 0.7f;
    
    // Initialize neural network with random weights
    ai_neural_network_s* nn = (ai_neural_network_s*)detector->neural_network;
    for(int i = 0; i < AI_FEATURE_COUNT; i++) {
        for(int j = 0; j < AI_HIDDEN_SIZE; j++) {
            nn->weights[i][j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
        }
    }
    
    for(int i = 0; i < AI_HIDDEN_SIZE; i++) {
        nn->hidden_bias[i] = ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
        for(int j = 0; j < AI_MAX_PROTOCOLS; j++) {
            nn->output_weights[i][j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
        }
    }
    
    for(int i = 0; i < AI_MAX_PROTOCOLS; i++) {
        nn->output_bias[i] = ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
    }
    
    // Initialize statistics
    detector->stats.signals_processed = 0;
    detector->stats.protocols_identified = 0;
    detector->stats.false_positives = 0;
    detector->stats.average_confidence = 0.0f;
    detector->stats.learning_iterations = 0;
    
    return detector;
}

void ai_signal_detector_free(ai_signal_detector_t* detector) {
    if(!detector) return;
    
    furi_free(detector->neural_network);
    furi_free(detector->patterns);
    furi_free(detector);
}

void ai_signal_detector_add_pattern(ai_signal_detector_t* detector, const ai_pattern_template_t* pattern) {
    if(!detector || !pattern || detector->pattern_count >= AI_MAX_PATTERNS) return;
    
    detector->patterns[detector->pattern_count] = *pattern;
    detector->pattern_count++;
    
    FURI_LOG_I("AI_DETECTOR", "Added pattern for protocol: %s", pattern->protocol_name);
}

ai_detection_result_t* ai_signal_detector_detect(ai_signal_detector_t* detector, const subghz_signal_t* signal) {
    if(!detector || !signal) return NULL;
    
    ai_detection_result_t* result = furi_alloc(sizeof(ai_detection_result_t));
    memset(result, 0, sizeof(ai_detection_result_t));
    
    // Extract features
    ai_signal_features_t features;
    extract_features(signal, &features);
    
    // Pattern matching
    float best_similarity = 0.0f;
    int best_pattern_index = -1;
    
    for(int i = 0; i < detector->pattern_count; i++) {
        float similarity = calculate_pattern_similarity(&features, &detector->patterns[i]);
        if(similarity > best_similarity) {
            best_similarity = similarity;
            best_pattern_index = i;
        }
    }
    
    // Neural network prediction
    float nn_output[AI_MAX_PROTOCOLS];
    float nn_input[AI_FEATURE_COUNT];
    
    // Normalize features for neural network
    nn_input[0] = features.mean / 32768.0f;
    nn_input[1] = features.std_dev / 32768.0f;
    nn_input[2] = features.peak_to_peak / 65536.0f;
    nn_input[3] = features.rms / 32768.0f;
    nn_input[4] = features.spectral_centroid / 1000000.0f;
    nn_input[5] = features.spectral_bandwidth / 100000.0f;
    nn_input[6] = features.spectral_rolloff / 1000000.0f;
    nn_input[7] = features.modulation_index;
    nn_input[8] = features.zero_crossing_rate;
    nn_input[9] = features.snr / 50.0f; // Normalize to 0-1 range
    nn_input[10] = (features.rssi + 100.0f) / 100.0f; // Normalize -100 to 0 dB
    nn_input[11] = features.signal_quality / 4.0f;
    nn_input[12] = features.pulse_width / 10000.0f;
    nn_input[13] = features.pulse_period / 10000.0f;
    nn_input[14] = features.duty_cycle;
    nn_input[15] = features.entropy / 4.0f; // Max entropy for 4-bit bins
    
    neural_network_forward((const ai_neural_network_s*)detector->neural_network, nn_input, nn_output);
    
    // Find best neural network prediction
    float best_nn_output = 0.0f;
    int best_nn_index = -1;
    
    for(int i = 0; i < AI_MAX_PROTOCOLS; i++) {
        if(nn_output[i] > best_nn_output) {
            best_nn_output = nn_output[i];
            best_nn_index = i;
        }
    }
    
    // Combine pattern matching and neural network results
    float combined_confidence = 0.0f;
    
    if(best_pattern_index >= 0 && best_nn_index >= 0) {
        // Both methods agree
        if(best_pattern_index == best_nn_index) {
            combined_confidence = (best_similarity + best_nn_output) / 2.0f;
            strncpy(result->protocol_name, detector->patterns[best_pattern_index].protocol_name, sizeof(result->protocol_name) - 1);
        } else {
            // Methods disagree, use higher confidence
            if(best_similarity > best_nn_output) {
                combined_confidence = best_similarity;
                strncpy(result->protocol_name, detector->patterns[best_pattern_index].protocol_name, sizeof(result->protocol_name) - 1);
            } else {
                combined_confidence = best_nn_output;
                snprintf(result->protocol_name, sizeof(result->protocol_name), "AI_Protocol_%d", best_nn_index);
            }
        }
    } else if(best_pattern_index >= 0) {
        combined_confidence = best_similarity;
        strncpy(result->protocol_name, detector->patterns[best_pattern_index].protocol_name, sizeof(result->protocol_name) - 1);
    } else if(best_nn_index >= 0) {
        combined_confidence = best_nn_output;
        snprintf(result->protocol_name, sizeof(result->protocol_name), "AI_Protocol_%d", best_nn_index);
    } else {
        combined_confidence = 0.0f;
        strncpy(result->protocol_name, "UNKNOWN", sizeof(result->protocol_name) - 1);
    }
    
    result->confidence = combined_confidence;
    result->pattern_similarity = best_similarity;
    result->neural_confidence = best_nn_output;
    result->features = features;
    
    // Update statistics
    detector->stats.signals_processed++;
    detector->stats.average_confidence = (detector->stats.average_confidence * (detector->stats.signals_processed - 1) + 
                                         combined_confidence) / detector->stats.signals_processed;
    
    if(combined_confidence >= detector->confidence_threshold) {
        detector->stats.protocols_identified++;
    } else {
        detector->stats.false_positives++;
    }
    
    // Learning update
    if(detector->learning_enabled && best_pattern_index >= 0) {
        ai_signal_detector_update_learning(detector, signal, best_pattern_index, combined_confidence);
    }
    
    return result;
}

void ai_signal_detector_update_learning(ai_signal_detector_t* detector, const subghz_signal_t* signal, int pattern_index, float confidence) {
    if(!detector || !signal || pattern_index < 0 || pattern_index >= detector->pattern_count) return;
    
    // Extract features for learning
    ai_signal_features_t features;
    extract_features(signal, &features);
    
    // Update pattern template with moving average
    ai_pattern_template_t* pattern = &detector->patterns[pattern_index];
    float learning_rate = 0.1f;
    
    pattern->mean = pattern->mean * (1.0f - learning_rate) + features.mean * learning_rate;
    pattern->std_dev = pattern->std_dev * (1.0f - learning_rate) + features.std_dev * learning_rate;
    pattern->center_frequency = pattern->center_frequency * (1.0f - learning_rate) + features.spectral_centroid * learning_rate;
    
    // Update tolerances based on variance
    pattern->mean_tolerance = pattern->std_dev * 2.0f;
    pattern->std_dev_tolerance = pattern->std_dev * 0.5f;
    
    detector->stats.learning_iterations++;
    
    FURI_LOG_D("AI_DETECTOR", "Updated learning for pattern %s (confidence: %.2f)", 
               pattern->protocol_name, confidence);
}

void ai_signal_detector_enable_learning(ai_signal_detector_t* detector, bool enabled) {
    if(detector) {
        detector->learning_enabled = enabled;
        FURI_LOG_I("AI_DETECTOR", "Learning %s", enabled ? "enabled" : "disabled");
    }
}

void ai_signal_detector_set_confidence_threshold(ai_signal_detector_t* detector, float threshold) {
    if(detector && threshold >= 0.0f && threshold <= 1.0f) {
        detector->confidence_threshold = threshold;
        FURI_LOG_I("AI_DETECTOR", "Confidence threshold set to %.2f", threshold);
    }
}

ai_detector_stats_t* ai_signal_detector_get_stats(ai_signal_detector_t* detector) {
    return detector ? &detector->stats : NULL;
}

void ai_signal_detector_reset_stats(ai_signal_detector_t* detector) {
    if(!detector) return;
    
    detector->stats.signals_processed = 0;
    detector->stats.protocols_identified = 0;
    detector->stats.false_positives = 0;
    detector->stats.average_confidence = 0.0f;
    detector->stats.learning_iterations = 0;
}

// Utility functions
void ai_detection_result_free(ai_detection_result_t* result) {
    if(result) {
        furi_free(result);
    }
}

bool ai_signal_detector_save_model(ai_signal_detector_t* detector, const char* filename) {
    if(!detector || !filename) return false;
    
    // TODO: Implement storage when available
    FURI_LOG_W("AI_DETECTOR", "Model saving not yet implemented");
    return false;
}

bool ai_signal_detector_load_model(ai_signal_detector_t* detector, const char* filename) {
    if(!detector || !filename) return false;
    
    // TODO: Implement storage when available
    FURI_LOG_W("AI_DETECTOR", "Model loading not yet implemented");
    return false;
}
