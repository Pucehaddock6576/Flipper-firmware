#include "subghz_jammer_i.h"

#define TAG "SubGhzJammerApp"

static void subghz_jammer_update_view(SubGhzJammer* app) {
    jammer_view_set_frequency(app->jammer_view, app->frequency);
    jammer_view_set_frequency_name(app->jammer_view, frequency_presets[app->preset_index].name);
    jammer_view_set_jamming(app->jammer_view, app->is_jamming);
}

static void subghz_jammer_dialog_callback(DialogExResult result, void* context) {
    SubGhzJammer* app = context;
    UNUSED(result);
    app->current_view = SubGhzJammerViewMain;
    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzJammerViewMain);
}

static void subghz_jammer_show_info(SubGhzJammer* app) {
    text_box_reset(app->text_box);
    text_box_set_font(app->text_box, TextBoxFontText);
    text_box_set_text(
        app->text_box,
        "Sub-GHz Jammer v" SUBGHZ_JAMMER_VERSION "\n\n"
        "Generates noise on Sub-GHz frequencies to raise the noise floor.\n\n"
        "Controls:\n"
        "- Up/Down: Change frequency\n"
        "- OK: Start/Stop jamming\n"
        "- Left: This info screen\n"
        "- Right: Credits\n"
        "- Back: Exit app\n\n"
        "Available frequencies range from 300 MHz to 925 MHz.\n\n"
        "Warning: Jamming radio frequencies may be illegal in your jurisdiction.");
    
    app->current_view = SubGhzJammerViewTextBox;
    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzJammerViewTextBox);
}

static void subghz_jammer_show_credits(SubGhzJammer* app) {
    DialogEx* dialog = app->dialog;
    dialog_ex_reset(dialog);
    
    dialog_ex_set_header(dialog, "Credits", 64, 0, AlignCenter, AlignTop);
    dialog_ex_set_text(
        dialog,
        "Sub-GHz Jammer v" SUBGHZ_JAMMER_VERSION "\n\n"
        "Created by:\n"
        ".leviathan",
        64, 16, AlignCenter, AlignTop);
    dialog_ex_set_result_callback(dialog, subghz_jammer_dialog_callback);
    dialog_ex_set_context(dialog, app);
    
    app->current_view = SubGhzJammerViewDialog;
    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzJammerViewDialog);
}

static void subghz_jammer_view_callback(JammerViewEvent event, void* context) {
    SubGhzJammer* app = context;
    
    switch(event) {
        case JammerViewEventFrequencyUp:
            if(!app->is_jamming) {
                if(app->preset_index > 0) {
                    app->preset_index--;
                } else {
                    app->preset_index = FREQUENCY_PRESETS_COUNT - 1;
                }
                app->frequency = frequency_presets[app->preset_index].frequency;
                subghz_jammer_update_view(app);
            }
            break;
            
        case JammerViewEventFrequencyDown:
            if(!app->is_jamming) {
                app->preset_index++;
                if(app->preset_index >= FREQUENCY_PRESETS_COUNT) {
                    app->preset_index = 0;
                }
                app->frequency = frequency_presets[app->preset_index].frequency;
                subghz_jammer_update_view(app);
            }
            break;
            
        case JammerViewEventStartStop:
            if(app->is_jamming) {
                // Stop jamming
                jammer_worker_stop(app->worker);
                app->is_jamming = false;
                notification_message(app->notifications, &sequence_blink_stop);
                notification_message(app->notifications, &sequence_single_vibro);
                FURI_LOG_I(TAG, "Jamming stopped");
            } else {
                // Start jamming
                FURI_LOG_I(TAG, "Starting jamming on %lu Hz", app->frequency);
                
                if(!jammer_worker_is_frequency_valid(app->worker, app->frequency)) {
                    FURI_LOG_E(TAG, "Invalid frequency: %lu", app->frequency);
                    notification_message(app->notifications, &sequence_error);
                } else if(jammer_worker_start(app->worker, app->frequency)) {
                    app->is_jamming = true;
                    notification_message(app->notifications, &sequence_single_vibro);
                    notification_message(app->notifications, &sequence_blink_start_magenta);
                    FURI_LOG_I(TAG, "Jamming started");
                } else {
                    FURI_LOG_E(TAG, "Failed to start jammer");
                    notification_message(app->notifications, &sequence_error);
                }
            }
            subghz_jammer_update_view(app);
            break;
            
        case JammerViewEventInfo:
            if(!app->is_jamming) {
                subghz_jammer_show_info(app);
            }
            break;
            
        case JammerViewEventCredits:
            if(!app->is_jamming) {
                subghz_jammer_show_credits(app);
            }
            break;
            
        case JammerViewEventExit:
            if(app->is_jamming) {
                // Stop jamming before exit
                jammer_worker_stop(app->worker);
                app->is_jamming = false;
                notification_message(app->notifications, &sequence_blink_stop);
            }
            view_dispatcher_stop(app->view_dispatcher);
            break;
    }
}

static bool subghz_jammer_back_event_callback(void* context) {
    SubGhzJammer* app = context;
    
    if(app->current_view == SubGhzJammerViewDialog || 
       app->current_view == SubGhzJammerViewTextBox) {
        app->current_view = SubGhzJammerViewMain;
        view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzJammerViewMain);
        return true;
    }
    
    // On main view, back exits (handled by view callback)
    return false;
}

static SubGhzJammer* subghz_jammer_alloc(void) {
    SubGhzJammer* app = malloc(sizeof(SubGhzJammer));

    app->view_dispatcher = view_dispatcher_alloc();
    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, subghz_jammer_back_event_callback);

    // Initialize SubGhz devices
    subghz_devices_init();

    // Initialize radio device (try external first, then internal)
    app->radio_device = jammer_radio_device_loader_set(
        NULL, SubGhzRadioDeviceTypeExternalCC1101);

    subghz_devices_reset(app->radio_device);
    subghz_devices_idle(app->radio_device);

    // Initialize worker
    app->worker = jammer_worker_alloc(app->radio_device);

    // Jammer View (main screen)
    app->jammer_view = jammer_view_alloc();
    jammer_view_set_callback(app->jammer_view, subghz_jammer_view_callback, app);
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubGhzJammerViewMain,
        jammer_view_get_view(app->jammer_view));

    // Dialog (for credits)
    app->dialog = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubGhzJammerViewDialog,
        dialog_ex_get_view(app->dialog));

    // TextBox (for info - scrollable)
    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubGhzJammerViewTextBox,
        text_box_get_view(app->text_box));

    // Default state - start at 433.92 MHz (index 10)
    app->preset_index = 10;
    app->frequency = frequency_presets[app->preset_index].frequency;
    app->is_jamming = false;
    app->current_view = SubGhzJammerViewMain;

    subghz_jammer_update_view(app);

    return app;
}

static void subghz_jammer_free(SubGhzJammer* app) {
    furi_assert(app);

    // Stop worker if running
    if(app->is_jamming) {
        jammer_worker_stop(app->worker);
    }
    jammer_worker_free(app->worker);

    // Deinit radio
    subghz_devices_sleep(app->radio_device);
    jammer_radio_device_loader_end(app->radio_device);
    subghz_devices_deinit();

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, SubGhzJammerViewMain);
    jammer_view_free(app->jammer_view);

    view_dispatcher_remove_view(app->view_dispatcher, SubGhzJammerViewDialog);
    dialog_ex_free(app->dialog);

    view_dispatcher_remove_view(app->view_dispatcher, SubGhzJammerViewTextBox);
    text_box_free(app->text_box);

    // Free view dispatcher
    view_dispatcher_free(app->view_dispatcher);

    // Close records
    notification_message(app->notifications, &sequence_blink_stop);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t subghz_jammer_app(void* p) {
    UNUSED(p);

    furi_hal_power_suppress_charge_enter();

    SubGhzJammer* app = subghz_jammer_alloc();

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzJammerViewMain);

    notification_message(app->notifications, &sequence_display_backlight_on);
    view_dispatcher_run(app->view_dispatcher);

    subghz_jammer_free(app);

    furi_hal_power_suppress_charge_exit();

    return 0;
}
