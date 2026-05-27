#include "../subghz_i.h"
#include <lib/subghz/subghz_protocol_registry.h>

#define TAG "SubGhzSceneProtocolList"

/* ── helpers ──────────────────────────────────────────────────────────────── */

static bool proto_filter_contains(const char* filter, const char* name) {
    const char* p = filter;
    while(*p) {
        const char* comma = strchr(p, ',');
        size_t len = comma ? (size_t)(comma - p) : strlen(p);
        if(len == strlen(name) && strncmp(p, name, len) == 0) return true;
        if(!comma) break;
        p = comma + 1;
    }
    return false;
}

static void proto_filter_toggle(char* filter, size_t filter_size, const char* name) {
    if(proto_filter_contains(filter, name)) {
        /* remove it */
        char tmp[256] = {0};
        const char* p = filter;
        bool first = true;
        while(*p) {
            const char* comma = strchr(p, ',');
            size_t len = comma ? (size_t)(comma - p) : strlen(p);
            if(!(len == strlen(name) && strncmp(p, name, len) == 0)) {
                if(!first) strncat(tmp, ",", sizeof(tmp) - strlen(tmp) - 1);
                strncat(tmp, p, len < sizeof(tmp) - strlen(tmp) - 1 ? len : 0);
                first = false;
            }
            if(!comma) break;
            p = comma + 1;
        }
        strncpy(filter, tmp, filter_size - 1);
        filter[filter_size - 1] = '\0';
    } else {
        /* add it */
        if(filter[0] != '\0') strncat(filter, ",", filter_size - strlen(filter) - 1);
        strncat(filter, name, filter_size - strlen(filter) - 1);
    }
}

/* ── callbacks ────────────────────────────────────────────────────────────── */

static void subghz_scene_protocol_list_item_changed(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t value_index = variable_item_get_current_value_index(item);

    uint32_t proto_idx =
        scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneProtocolList);
    size_t selected =
        variable_item_list_get_selected_item_index(subghz->variable_item_list);
    UNUSED(proto_idx);

    const SubGhzProtocol* protocol =
        subghz_protocol_registry_get_by_index(&subghz_protocol_registry, selected);
    if(!protocol) return;

    const char* name = protocol->name;

    bool currently_in =
        proto_filter_contains(subghz->last_settings->protocol_filter, name);

    if((value_index == 1) != currently_in) {
        proto_filter_toggle(
            subghz->last_settings->protocol_filter,
            sizeof(subghz->last_settings->protocol_filter),
            name);
    }

    variable_item_set_current_value_text(item, value_index ? "ONLY" : "---");
    subghz_last_settings_save(subghz->last_settings);
}

/* ── scene callbacks ──────────────────────────────────────────────────────── */

void subghz_scene_protocol_list_on_enter(void* context) {
    SubGhz* subghz = context;
    VariableItemList* list = subghz->variable_item_list;
    variable_item_list_reset(list);

    size_t protocol_count = subghz_protocol_registry_count(&subghz_protocol_registry);

    for(size_t i = 0; i < protocol_count; i++) {
        const SubGhzProtocol* protocol =
            subghz_protocol_registry_get_by_index(&subghz_protocol_registry, i);
        if(!protocol) continue;

        VariableItem* item = variable_item_list_add(
            list,
            protocol->name,
            2,
            subghz_scene_protocol_list_item_changed,
            subghz);

        bool enabled = proto_filter_contains(
            subghz->last_settings->protocol_filter, protocol->name);
        variable_item_set_current_value_index(item, enabled ? 1 : 0);
        variable_item_set_current_value_text(item, enabled ? "ONLY" : "---");
    }

    variable_item_list_set_selected_item(
        list,
        scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneProtocolList));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdVariableItemList);
}

bool subghz_scene_protocol_list_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(
            subghz->scene_manager, SubGhzSceneProtocolList, event.event);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(subghz->scene_manager);
        consumed = true;
    }

    return consumed;
}

void subghz_scene_protocol_list_on_exit(void* context) {
    SubGhz* subghz = context;
    scene_manager_set_scene_state(
        subghz->scene_manager,
        SubGhzSceneProtocolList,
        variable_item_list_get_selected_item_index(subghz->variable_item_list));
    variable_item_list_reset(subghz->variable_item_list);
}
