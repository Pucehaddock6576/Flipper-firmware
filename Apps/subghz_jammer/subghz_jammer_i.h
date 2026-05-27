#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>

#include <notification/notification.h>
#include <notification/notification_messages.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/text_box.h>

#include <assets_icons.h>

#include "subghz_jammer.h"
#include "helpers/jammer_radio_device_loader.h"
#include "helpers/jammer_worker.h"
#include "views/jammer_view.h"

#define SUBGHZ_JAMMER_VERSION "1.0"

// Common frequency presets (in Hz)
typedef struct {
    const char* name;
    uint32_t frequency;
} FrequencyPreset;

// Available frequency presets for quick selection
static const FrequencyPreset frequency_presets[] = {
    {"300.00", 300000000},
    {"303.87", 303875000},
    {"304.25", 304250000},
    {"310.00", 310000000},
    {"315.00", 315000000},
    {"318.00", 318000000},
    {"390.00", 390000000},
    {"418.00", 418000000},
    {"433.07", 433075000},
    {"433.42", 433420000},
    {"433.92", 433920000},
    {"434.42", 434420000},
    {"434.77", 434775000},
    {"438.90", 438900000},
    {"868.35", 868350000},
    {"868.92", 868920000},
    {"915.00", 915000000},
    {"925.00", 925000000},
};

#define FREQUENCY_PRESETS_COUNT (sizeof(frequency_presets) / sizeof(frequency_presets[0]))

typedef enum {
    SubGhzJammerViewMain,
    SubGhzJammerViewDialog,
    SubGhzJammerViewTextBox,
} SubGhzJammerViewId;

struct SubGhzJammer {
    NotificationApp* notifications;
    Gui* gui;
    ViewDispatcher* view_dispatcher;

    // Views
    JammerView* jammer_view;
    DialogEx* dialog;
    TextBox* text_box;

    // Radio
    const SubGhzDevice* radio_device;
    JammerWorker* worker;

    // State
    uint32_t frequency;        // Current frequency in Hz
    uint8_t preset_index;      // Index in preset list
    bool is_jamming;           // Whether jamming is active
    SubGhzJammerViewId current_view;  // Current view
};
