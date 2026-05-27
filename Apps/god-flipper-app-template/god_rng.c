#include "god_app_i.h"
#include <furi_hal_random.h>
#include <furi.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/file_stream.h>

#define TAG GOD_APP_TAG

void god_add_bits(GodApp* app, int num_bits, uint32_t value) {
    for(int i = 0; i < num_bits && app->bit_count < GOD_BITS_SIZE; i++) {
        int byte_idx = app->bit_tail / 8;
        int bit_idx = app->bit_tail % 8;
        if(value & 1)
            app->bits[byte_idx] |= (1u << bit_idx);
        else
            app->bits[byte_idx] &= ~(1u << bit_idx);
        value >>= 1;
        app->bit_tail = (app->bit_tail + 1) % GOD_BITS_SIZE;
        app->bit_count++;
    }
}

uint32_t god_get_bits(GodApp* app, int num_bits) {
    uint32_t result = 0;
    for(int i = 0; i < num_bits && app->bit_count > 0; i++) {
        int byte_idx = app->bit_head / 8;
        int bit_idx = app->bit_head % 8;
        result |= ((app->bits[byte_idx] >> bit_idx) & 1u) << i;
        app->bit_head = (app->bit_head + 1) % GOD_BITS_SIZE;
        app->bit_count--;
    }
    return result;
}

void god_add_entropy_from_timer(GodApp* app) {
    uint32_t t = furi_get_tick();
    god_add_bits(app, 32, t);
}

void god_seed_from_hal(GodApp* app) {
    for(int i = 0; i < 8; i++)
        god_add_bits(app, 32, furi_hal_random_get());
}

bool god_load_words(GodApp* app) {
    Stream* stream = file_stream_alloc(app->storage);
    FuriString* line = furi_string_alloc();
    bool ok = false;

    if(!file_stream_open(stream, APP_ASSETS_PATH("kjv.txt"), FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open word list");
        goto done;
    }

    app->word_count = 0;
    app->word_offsets = malloc(GOD_WORD_LIST_MAX * sizeof(uint32_t));
    if(!app->word_offsets) goto done;

    while(app->word_count < GOD_WORD_LIST_MAX) {
        uint32_t offset = stream_tell(stream);
        if(!stream_read_line(stream, line)) break;
        if(furi_string_size(line) == 0) continue;
        app->word_offsets[app->word_count] = offset;
        app->word_count++;
    }

    FURI_LOG_I(TAG, "Loaded %lu words", (unsigned long)app->word_count);
    ok = (app->word_count > 0);

done:
    furi_string_free(line);
    file_stream_close(stream);
    stream_free(stream);
    if(!ok && app->word_offsets) {
        free(app->word_offsets);
        app->word_offsets = NULL;
    }
    return ok;
}

bool god_get_random_word(GodApp* app, FuriString* word) {
    if(!app->word_offsets || app->word_count == 0) return false;

    uint32_t idx;
    if(app->bit_count >= 14)
        idx = god_get_bits(app, 14) % app->word_count;
    else
        idx = furi_hal_random_get() % app->word_count;

    Stream* stream = file_stream_alloc(app->storage);
    bool ok = false;

    if(!file_stream_open(stream, APP_ASSETS_PATH("kjv.txt"), FSAM_READ, FSOM_OPEN_EXISTING)) {
        stream_free(stream);
        return false;
    }

    if(!stream_seek(stream, (int32_t)app->word_offsets[idx], StreamOffsetFromStart)) {
        file_stream_close(stream);
        stream_free(stream);
        return false;
    }

    if(stream_read_line(stream, word)) {
        furi_string_trim(word);
        ok = (furi_string_size(word) > 0);
    }

    file_stream_close(stream);
    stream_free(stream);
    return ok;
}

void god_get_random_words(GodApp* app, FuriString* out, uint32_t count) {
    furi_string_reset(out);
    FuriString* w = furi_string_alloc();
    for(uint32_t i = 0; i < count; i++) {
        if(!god_get_random_word(app, w)) break;
        if(i > 0) furi_string_cat_printf(out, " ");
        furi_string_cat(out, w);
    }
    furi_string_free(w);
}
