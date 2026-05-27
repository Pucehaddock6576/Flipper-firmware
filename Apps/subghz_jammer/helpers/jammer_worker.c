#include "jammer_worker.h"
#include "jammer_radio_device_loader.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_random.h>
#include <lib/subghz/devices/devices.h>

#define TAG "JammerWorker"

// Noise pattern parameters - aggressive continuous transmission
// Times are in microseconds
#define NOISE_PULSE_MIN 50
#define NOISE_PULSE_MAX 500
#define NOISE_GAP_MIN 50  
#define NOISE_GAP_MAX 500
#define SAMPLES_PER_BURST 512

struct JammerWorker {
    FuriThread* thread;
    const SubGhzDevice* radio_device;
    
    volatile bool worker_running;
    volatile bool tx_running;
    JammerWorkerState state;
    
    uint32_t frequency;
    
    // For noise generation
    volatile uint32_t sample_index;
    volatile bool is_high;
    
    JammerWorkerCallback callback;
    void* callback_context;
};

static void jammer_worker_send_callback(JammerWorker* worker) {
    if(worker->callback) {
        worker->callback(worker->callback_context, worker->state);
    }
}

// Generate random value in range [min, max]
static inline uint32_t random_in_range(uint32_t min, uint32_t max) {
    if(min >= max) return min;
    return min + (furi_hal_random_get() % (max - min + 1));
}

// Async TX callback - generates noise pulses directly
static LevelDuration jammer_worker_yield_callback(void* context) {
    JammerWorker* worker = context;
    
    if(!worker->tx_running || worker->sample_index >= SAMPLES_PER_BURST) {
        return level_duration_reset();
    }
    
    worker->sample_index++;
    worker->is_high = !worker->is_high;
    
    uint32_t duration;
    if(worker->is_high) {
        // High pulse (transmission on)
        duration = random_in_range(NOISE_PULSE_MIN, NOISE_PULSE_MAX);
    } else {
        // Low gap (transmission off)
        duration = random_in_range(NOISE_GAP_MIN, NOISE_GAP_MAX);
    }
    
    return level_duration_make(worker->is_high, duration);
}

static void jammer_worker_transmit_burst(JammerWorker* worker) {
    // Reset sample counter for new burst
    worker->sample_index = 0;
    worker->is_high = false;
    worker->tx_running = true;
    
    // Start async TX
    subghz_devices_start_async_tx(
        worker->radio_device,
        jammer_worker_yield_callback,
        worker);
    
    // Wait for transmission to complete
    while(!subghz_devices_is_async_complete_tx(worker->radio_device)) {
        if(!worker->worker_running) {
            break;
        }
        furi_delay_us(100);
    }
    
    subghz_devices_stop_async_tx(worker->radio_device);
    worker->tx_running = false;
}

static int32_t jammer_worker_thread(void* context) {
    JammerWorker* worker = context;
    
    FURI_LOG_I(TAG, "Worker started, frequency: %lu Hz", worker->frequency);
    
    worker->state = JammerWorkerStateRunning;
    jammer_worker_send_callback(worker);
    
    // Reset and configure radio
    subghz_devices_reset(worker->radio_device);
    subghz_devices_idle(worker->radio_device);
    
    // Check if frequency is valid
    if(!subghz_devices_is_frequency_valid(worker->radio_device, worker->frequency)) {
        FURI_LOG_E(TAG, "Invalid frequency: %lu", worker->frequency);
        worker->state = JammerWorkerStateStopped;
        worker->worker_running = false;
        jammer_worker_send_callback(worker);
        return -1;
    }
    
    // Load preset and set frequency
    subghz_devices_load_preset(worker->radio_device, FuriHalSubGhzPresetOok270Async, NULL);
    uint32_t real_freq = subghz_devices_set_frequency(worker->radio_device, worker->frequency);
    FURI_LOG_I(TAG, "Frequency set: requested=%lu, actual=%lu", worker->frequency, real_freq);
    
    // Enter TX mode
    if(!subghz_devices_set_tx(worker->radio_device)) {
        FURI_LOG_E(TAG, "Failed to enter TX mode");
        worker->state = JammerWorkerStateStopped;
        worker->worker_running = false;
        jammer_worker_send_callback(worker);
        return -1;
    }
    
    FURI_LOG_I(TAG, "TX mode enabled, starting jamming loop");
    
    // Continuous jamming loop
    while(worker->worker_running) {
        jammer_worker_transmit_burst(worker);
        
        // Need to re-enter TX mode for next burst
        if(worker->worker_running) {
            subghz_devices_idle(worker->radio_device);
            if(!subghz_devices_set_tx(worker->radio_device)) {
                FURI_LOG_E(TAG, "Failed to re-enter TX mode");
                break;
            }
        }
    }
    
    subghz_devices_idle(worker->radio_device);
    
    worker->state = JammerWorkerStateStopped;
    jammer_worker_send_callback(worker);
    
    FURI_LOG_I(TAG, "Worker stopped");
    
    return 0;
}

JammerWorker* jammer_worker_alloc(const SubGhzDevice* radio_device) {
    JammerWorker* worker = malloc(sizeof(JammerWorker));
    
    worker->radio_device = radio_device;
    worker->worker_running = false;
    worker->tx_running = false;
    worker->state = JammerWorkerStateIdle;
    worker->frequency = 433920000;
    worker->sample_index = 0;
    worker->is_high = false;
    worker->callback = NULL;
    worker->callback_context = NULL;
    
    worker->thread = furi_thread_alloc();
    furi_thread_set_name(worker->thread, "JammerWorker");
    furi_thread_set_stack_size(worker->thread, 2048);
    furi_thread_set_context(worker->thread, worker);
    furi_thread_set_callback(worker->thread, jammer_worker_thread);
    
    return worker;
}

void jammer_worker_free(JammerWorker* worker) {
    furi_assert(worker);
    
    jammer_worker_stop(worker);
    furi_thread_free(worker->thread);
    
    free(worker);
}

bool jammer_worker_start(JammerWorker* worker, uint32_t frequency) {
    furi_assert(worker);
    
    if(worker->worker_running) {
        FURI_LOG_W(TAG, "Worker already running");
        return false;
    }
    
    worker->frequency = frequency;
    worker->worker_running = true;
    furi_thread_start(worker->thread);
    
    return true;
}

void jammer_worker_stop(JammerWorker* worker) {
    furi_assert(worker);
    
    if(!worker->worker_running) {
        return;
    }
    
    worker->worker_running = false;
    worker->tx_running = false;
    furi_thread_join(worker->thread);
    
    subghz_devices_idle(worker->radio_device);
}

bool jammer_worker_is_running(JammerWorker* worker) {
    furi_assert(worker);
    return worker->worker_running;
}

void jammer_worker_set_callback(JammerWorker* worker, JammerWorkerCallback callback, void* context) {
    furi_assert(worker);
    worker->callback = callback;
    worker->callback_context = context;
}

bool jammer_worker_is_frequency_valid(JammerWorker* worker, uint32_t frequency) {
    furi_assert(worker);
    return subghz_devices_is_frequency_valid(worker->radio_device, frequency);
}
