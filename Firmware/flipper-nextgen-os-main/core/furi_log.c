/* Logging implementation for Furi Core */

#include "furi.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Log level strings
static const char* log_level_strings[] = {
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG"
};

void furi_log(FuriLogLevel level, const char* tag, const char* format, ...) {
    if(level > FURI_LOG_LEVEL_DEBUG) return;
    
    // Format timestamp
    uint32_t tick = furi_get_tick();
    
    // Print log header
    printf("[%lu.%03lu] %s [%s] ", 
           tick / 1000, tick % 1000, 
           log_level_strings[level], tag);
    
    // Print formatted message
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
}

// Utility functions
void furi_delay_ms(uint32_t ms) {
    // Simple delay simulation
    volatile uint32_t i;
    for(i = 0; i < ms * 1000; i++) {
        __asm__("nop");
    }
}

void furi_delay_us(uint32_t us) {
    volatile uint32_t i;
    for(i = 0; i < us; i++) {
        __asm__("nop");
    }
}

uint32_t furi_get_tick(void) {
    static uint32_t tick_count = 0;
    return tick_count++;
}
