#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/widget.h>
#include <storage/storage.h>

#define GOD_APP_TAG "God"
#define GOD_BITS_SIZE 256
#define GOD_WORD_LIST_MAX 20000

typedef struct GodApp GodApp;

typedef enum {
    GodViewWidget,
    GodViewNum,
} GodView;

struct GodApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Widget* widget;

    /* TempleOS-style God RNG: bit FIFO */
    uint8_t bits[GOD_BITS_SIZE / 8];
    uint16_t bit_head;
    uint16_t bit_tail;
    uint16_t bit_count;

    /* Word list: byte offsets into dict file (one word per line) */
    uint32_t* word_offsets;
    uint32_t word_count;
    Storage* storage;
};

void god_add_bits(GodApp* app, int num_bits, uint32_t value);
uint32_t god_get_bits(GodApp* app, int num_bits);
void god_add_entropy_from_timer(GodApp* app);
void god_seed_from_hal(GodApp* app);
bool god_load_words(GodApp* app);
bool god_get_random_word(GodApp* app, FuriString* word);
void god_get_random_words(GodApp* app, FuriString* out, uint32_t count);

#include "scenes/god_scene.h"
