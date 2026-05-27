/* Simple main for testing without STM32 hardware */

#include "furi.h"
#include "furi_hal.h"
#include "version.h"

// Simple test main
int main(void) {
    printf("=== Flipper Next-Gen OS Test ===\n");
    
    // Initialize HAL
    furi_hal_init();
    
    // Initialize core
    furi_init();
    
    printf("System initialized successfully!\n");
    
    // Get system info
    system_info_t info;
    furi_get_system_info(&info);
    
    printf("System Info:\n");
    printf("  Version: %s\n", info.version);
    printf("  Build: %s\n", info.build);
    printf("  Tasks: %lu\n", info.task_count);
    printf("  Free heap: %lu bytes\n", info.free_heap);
    
    // Test logging
    FURI_LOG_I("TEST", "System test completed successfully");
    
    // Simple infinite loop
    while(1) {
        furi_delay_ms(1000);
        FURI_LOG_D("MAIN", "Heartbeat");
    }
    
    return 0;
}
