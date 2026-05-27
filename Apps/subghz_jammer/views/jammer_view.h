#pragma once

#include <gui/view.h>

typedef struct JammerView JammerView;

typedef enum {
    JammerViewEventStartStop,
    JammerViewEventFrequencyUp,
    JammerViewEventFrequencyDown,
    JammerViewEventInfo,
    JammerViewEventCredits,
    JammerViewEventExit,
} JammerViewEvent;

typedef void (*JammerViewCallback)(JammerViewEvent event, void* context);

/**
 * @brief Allocate a new JammerView instance.
 */
JammerView* jammer_view_alloc(void);

/**
 * @brief Free a JammerView instance.
 */
void jammer_view_free(JammerView* jammer_view);

/**
 * @brief Get the View from JammerView.
 */
View* jammer_view_get_view(JammerView* jammer_view);

/**
 * @brief Set the frequency to display.
 */
void jammer_view_set_frequency(JammerView* jammer_view, uint32_t frequency);

/**
 * @brief Set the frequency name to display.
 */
void jammer_view_set_frequency_name(JammerView* jammer_view, const char* name);

/**
 * @brief Set the jamming state.
 */
void jammer_view_set_jamming(JammerView* jammer_view, bool is_jamming);

/**
 * @brief Set the callback for view events.
 */
void jammer_view_set_callback(JammerView* jammer_view, JammerViewCallback callback, void* context);
