#pragma once

#include <lib/subghz/devices/devices.h>

typedef enum {
    SubGhzRadioDeviceTypeInternal,
    SubGhzRadioDeviceTypeExternalCC1101,
} SubGhzRadioDeviceType;

/**
 * @brief Sets the SubGhz radio device type.
 *
 * @param current_radio_device Pointer to the current SubGhz radio device.
 * @param radio_device_type The desired SubGhz radio device type.
 * @return const SubGhzDevice* Pointer to the new SubGhz radio device.
 */
const SubGhzDevice* jammer_radio_device_loader_set(
    const SubGhzDevice* current_radio_device,
    SubGhzRadioDeviceType radio_device_type);

/**
 * @brief Unloads the radio device.
 *
 * @param radio_device A pointer to the radio device to unload.
 */
void jammer_radio_device_loader_end(const SubGhzDevice* radio_device);
