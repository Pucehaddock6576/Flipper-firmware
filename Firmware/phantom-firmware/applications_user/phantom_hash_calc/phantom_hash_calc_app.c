#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/text_box.h>
#include <mbedtls/md5.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <string.h>
#include <stdio.h>

#define TEXT_INPUT_BUF_SIZE 128
#define RESULT_BUF_SIZE    256

typedef enum {
    ViewSubmenu,
    ViewTextInput,
    ViewTextBox,
} ViewId;

typedef enum {
    AlgoMD5,
    AlgoSHA1,
    AlgoSHA256,
} HashAlgo;

typedef struct {
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    TextInput* text_input;
    TextBox* text_box;
    HashAlgo algo;
    char input_buf[TEXT_INPUT_BUF_SIZE];
    char result_buf[RESULT_BUF_SIZE];
} App;

static void bytes_to_hex(const uint8_t* bytes, size_t len, char* out) {
    for(size_t i = 0; i < len; i++) {
        snprintf(out + i * 2, 3, "%02x", bytes[i]);
    }
}

static void compute_hash(App* app) {
    size_t input_len = strlen(app->input_buf);
    char hex[65] = {0};
    const char* algo_name;
    size_t hex_len;

    switch(app->algo) {
    case AlgoMD5: {
        uint8_t hash[16];
        mbedtls_md5_context ctx;
        mbedtls_md5_init(&ctx);
        mbedtls_md5_starts(&ctx);
        mbedtls_md5_update(&ctx, (const uint8_t*)app->input_buf, input_len);
        mbedtls_md5_finish(&ctx, hash);
        mbedtls_md5_free(&ctx);
        bytes_to_hex(hash, 16, hex);
        algo_name = "MD5";
        hex_len = 32;
        break;
    }
    case AlgoSHA1: {
        uint8_t hash[20];
        mbedtls_sha1_context ctx;
        mbedtls_sha1_init(&ctx);
        mbedtls_sha1_starts(&ctx);
        mbedtls_sha1_update(&ctx, (const uint8_t*)app->input_buf, input_len);
        mbedtls_sha1_finish(&ctx, hash);
        mbedtls_sha1_free(&ctx);
        bytes_to_hex(hash, 20, hex);
        algo_name = "SHA1";
        hex_len = 40;
        break;
    }
    case AlgoSHA256: {
        uint8_t hash[32];
        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0);
        mbedtls_sha256_update(&ctx, (const uint8_t*)app->input_buf, input_len);
        mbedtls_sha256_finish(&ctx, hash);
        mbedtls_sha256_free(&ctx);
        bytes_to_hex(hash, 32, hex);
        algo_name = "SHA256";
        hex_len = 64;
        break;
    }
    default:
        return;
    }

    /* Format result with line breaks every 24 chars for readability */
    size_t off = 0;
    int w = snprintf(app->result_buf + off, RESULT_BUF_SIZE - off, "%s:\n", algo_name);
    if(w > 0) off += w;

    size_t hoff = 0;
    while(hoff < hex_len && off < RESULT_BUF_SIZE - 1) {
        size_t chunk = 24;
        if(hoff + chunk > hex_len) chunk = hex_len - hoff;
        for(size_t i = 0; i < chunk && off < RESULT_BUF_SIZE - 1; i++) {
            app->result_buf[off++] = hex[hoff + i];
        }
        hoff += chunk;
        if(hoff < hex_len && off < RESULT_BUF_SIZE - 1) {
            app->result_buf[off++] = '\n';
        }
    }
    app->result_buf[off] = '\0';
}

static void text_input_cb(void* ctx) {
    App* app = ctx;
    compute_hash(app);
    text_box_set_text(app->text_box, app->result_buf);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewTextBox);
}

static void submenu_cb(void* ctx, uint32_t index) {
    App* app = ctx;
    app->algo = (HashAlgo)index;
    app->input_buf[0] = '\0';

    const char* header;
    switch(app->algo) {
    case AlgoMD5: header = "Text to MD5:"; break;
    case AlgoSHA1: header = "Text to SHA1:"; break;
    case AlgoSHA256: header = "Text to SHA256:"; break;
    default: header = "Enter text:"; break;
    }

    text_input_set_header_text(app->text_input, header);
    text_input_set_result_callback(
        app->text_input, text_input_cb, app, app->input_buf, TEXT_INPUT_BUF_SIZE, true);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewTextInput);
}

static uint32_t submenu_prev_cb(void* ctx) {
    UNUSED(ctx);
    return VIEW_NONE;
}

static uint32_t text_input_prev_cb(void* ctx) {
    UNUSED(ctx);
    return ViewSubmenu;
}

static uint32_t text_box_prev_cb(void* ctx) {
    UNUSED(ctx);
    return ViewSubmenu;
}

static App* app_alloc(void) {
    App* app = malloc(sizeof(App));
    memset(app, 0, sizeof(App));

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    app->submenu = submenu_alloc();
    submenu_add_item(app->submenu, "MD5", AlgoMD5, submenu_cb, app);
    submenu_add_item(app->submenu, "SHA1", AlgoSHA1, submenu_cb, app);
    submenu_add_item(app->submenu, "SHA256", AlgoSHA256, submenu_cb, app);
    view_set_previous_callback(submenu_get_view(app->submenu), submenu_prev_cb);
    view_dispatcher_add_view(app->view_dispatcher, ViewSubmenu, submenu_get_view(app->submenu));

    app->text_input = text_input_alloc();
    view_set_previous_callback(text_input_get_view(app->text_input), text_input_prev_cb);
    view_dispatcher_add_view(app->view_dispatcher, ViewTextInput, text_input_get_view(app->text_input));

    app->text_box = text_box_alloc();
    view_set_previous_callback(text_box_get_view(app->text_box), text_box_prev_cb);
    view_dispatcher_add_view(app->view_dispatcher, ViewTextBox, text_box_get_view(app->text_box));

    return app;
}

static void app_free(App* app) {
    view_dispatcher_remove_view(app->view_dispatcher, ViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, ViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, ViewTextBox);
    submenu_free(app->submenu);
    text_input_free(app->text_input);
    text_box_free(app->text_box);
    view_dispatcher_free(app->view_dispatcher);
    free(app);
}

int32_t phantom_hash_calc_app(void* p) {
    UNUSED(p);
    App* app = app_alloc();

    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewSubmenu);
    view_dispatcher_run(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    app_free(app);
    return 0;
}
