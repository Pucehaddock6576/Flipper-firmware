#pragma once

#include <furi.h>
#include <lib/subghz/devices/devices.h>

typedef enum {
    JammerWorkerStateIdle,
    JammerWorkerStateRunning,
    JammerWorkerStateStopped,
} JammerWorkerState;

typedef void (*JammerWorkerCallback)(void* context, JammerWorkerState state);

typedef struct JammerWorker JammerWorker;

/**
 * @brief Allocate a new JammerWorker instance.
 *
 * @param radio_device Pointer to the SubGhz radio device.
 * @return JammerWorker* Pointer to the new worker instance.
 */
JammerWorker* jammer_worker_alloc(const SubGhzDevice* radio_device);

/**
 * @brief Free a JammerWorker instance.
 *
 * @param worker Pointer to the worker instance.
 */
void jammer_worker_free(JammerWorker* worker);

/**
 * @brief Start jamming on a specific frequency.
 *
 * @param worker Pointer to the worker instance.
 * @param frequency Frequency in Hz (e.g., 433920000 for 433.92 MHz).
 * @return true if started successfully, false otherwise.
 */
bool jammer_worker_start(JammerWorker* worker, uint32_t frequency);

/**
 * @brief Stop jamming.
 *
 * @param worker Pointer to the worker instance.
 */
void jammer_worker_stop(JammerWorker* worker);

/**
 * @brief Check if the worker is running.
 *
 * @param worker Pointer to the worker instance.
 * @return true if running, false otherwise.
 */
bool jammer_worker_is_running(JammerWorker* worker);

/**
 * @brief Set callback for worker state changes.
 *
 * @param worker Pointer to the worker instance.
 * @param callback Callback function.
 * @param context Callback context.
 */
void jammer_worker_set_callback(JammerWorker* worker, JammerWorkerCallback callback, void* context);

/**
 * @brief Check if a frequency is valid for transmission.
 *
 * @param worker Pointer to the worker instance.
 * @param frequency Frequency in Hz.
 * @return true if valid, false otherwise.
 */
bool jammer_worker_is_frequency_valid(JammerWorker* worker, uint32_t frequency);
