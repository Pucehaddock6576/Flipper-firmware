/* Flipper Next-Gen OS - Core Implementation */

#include <furi.h>
#include <furi_hal.h>
#include <core/version.h>

// Core task scheduler
typedef struct {
    furi_task_t* tasks[MAX_TASKS];
    uint8_t task_count;
    furi_task_t* current_task;
    uint32_t tick_count;
} task_scheduler_t;

static task_scheduler_t scheduler = {0};

// Memory management
typedef struct {
    void* ptr;
    size_t size;
    const char* file;
    uint32_t line;
} alloc_info_t;

#define MAX_ALLOCS 1024
static alloc_info_t alloc_table[MAX_ALLOCS];
static uint32_t alloc_count = 0;

// System initialization
void furi_init(void) {
    // Initialize hardware abstraction layer
    furi_hal_init();
    
    // Initialize task scheduler
    scheduler.task_count = 0;
    scheduler.current_task = NULL;
    scheduler.tick_count = 0;
    
    // Initialize memory tracking
    alloc_count = 0;
    memset(alloc_table, 0, sizeof(alloc_table));
    
    FURI_LOG_I("CORE", "Flipper Next-Gen OS %s initialized", FLIPPER_VERSION);
}

// Task management
furi_task_t* furi_task_alloc(const char* name) {
    furi_task_t* task = malloc(sizeof(furi_task_t));
    if(task) {
        task->name = name;
        task->state = FURI_TASK_STATE_READY;
        task->priority = FURI_TASK_PRIORITY_NORMAL;
        task->stack_size = DEFAULT_STACK_SIZE;
        task->stack = malloc(task->stack_size);
        task->context = NULL;
        
        if(scheduler.task_count < MAX_TASKS) {
            scheduler.tasks[scheduler.task_count++] = task;
            FURI_LOG_I("CORE", "Task '%s' allocated", name);
        } else {
            free(task->stack);
            free(task);
            task = NULL;
            FURI_LOG_E("CORE", "Failed to allocate task '%s' - too many tasks", name);
        }
    }
    return task;
}

void furi_task_start(furi_task_t* task, furi_task_func_t func, void* context) {
    if(task && func) {
        task->func = func;
        task->context = context;
        task->state = FURI_TASK_STATE_RUNNING;
        scheduler.current_task = task;
        
        FURI_LOG_I("CORE", "Starting task '%s'", task->name);
        
        // Execute task function
        func(context);
        
        task->state = FURI_TASK_STATE_FINISHED;
        FURI_LOG_I("CORE", "Task '%s' finished", task->name);
    }
}

void furi_task_set_priority(furi_task_t* task, FuriTaskPriority priority) {
    if(task) {
        task->priority = priority;
        FURI_LOG_D("CORE", "Task '%s' priority set to %d", task->name, priority);
    }
}

// Memory management with tracking
void* furi_alloc(size_t size) {
    void* ptr = malloc(size);
    if(ptr && alloc_count < MAX_ALLOCS) {
        alloc_table[alloc_count].ptr = ptr;
        alloc_table[alloc_count].size = size;
        alloc_table[alloc_count].file = "unknown";
        alloc_table[alloc_count].line = 0;
        alloc_count++;
        
        FURI_LOG_D("CORE", "Allocated %zu bytes at %p", size, ptr);
    }
    return ptr;
}

void furi_free(void* ptr) {
    if(ptr) {
        // Find and remove from allocation table
        for(uint32_t i = 0; i < alloc_count; i++) {
            if(alloc_table[i].ptr == ptr) {
                FURI_LOG_D("CORE", "Freed %zu bytes at %p", alloc_table[i].size, ptr);
                
                // Remove entry
                memmove(&alloc_table[i], &alloc_table[i+1], 
                       (alloc_count - i - 1) * sizeof(alloc_info_t));
                alloc_count--;
                break;
            }
        }
        free(ptr);
    }
}

// Record system (service locator)
typedef struct {
    const char* name;
    void* record;
    uint32_t ref_count;
} record_entry_t;

#define MAX_RECORDS 64
static record_entry_t records[MAX_RECORDS];
static uint32_t record_count = 0;

void* furi_record_open(const char* name) {
    for(uint32_t i = 0; i < record_count; i++) {
        if(strcmp(records[i].name, name) == 0) {
            records[i].ref_count++;
            FURI_LOG_D("CORE", "Opened record '%s' (ref: %lu)", name, records[i].ref_count);
            return records[i].record;
        }
    }
    
    FURI_LOG_E("CORE", "Record '%s' not found", name);
    return NULL;
}

void furi_record_close(const char* name) {
    for(uint32_t i = 0; i < record_count; i++) {
        if(strcmp(records[i].name, name) == 0) {
            if(records[i].ref_count > 0) {
                records[i].ref_count--;
                FURI_LOG_D("CORE", "Closed record '%s' (ref: %lu)", name, records[i].ref_count);
            }
            return;
        }
    }
    
    FURI_LOG_W("CORE", "Record '%s' not found for close", name);
}

bool furi_record_create(const char* name, void* record) {
    if(record_count >= MAX_RECORDS) {
        FURI_LOG_E("CORE", "Too many records");
        return false;
    }
    
    // Check for duplicates
    for(uint32_t i = 0; i < record_count; i++) {
        if(strcmp(records[i].name, name) == 0) {
            FURI_LOG_W("CORE", "Record '%s' already exists", name);
            return false;
        }
    }
    
    records[record_count].name = name;
    records[record_count].record = record;
    records[record_count].ref_count = 0;
    record_count++;
    
    FURI_LOG_I("CORE", "Created record '%s'", name);
    return true;
}

// System tick handler
void furi_tick(void) {
    scheduler.tick_count++;
    
    // Update task scheduler
    // TODO: Implement proper task scheduling
}

// System information
void furi_get_system_info(system_info_t* info) {
    if(info) {
        info->version = FLIPPER_VERSION;
        info->build = FLIPPER_BUILD;
        info->tick_count = scheduler.tick_count;
        info->task_count = scheduler.task_count;
        info->alloc_count = alloc_count;
        info->free_heap = 65536; // Dummy 64KB free
    }
}

// Memory leak detection
void furi_check_memory_leaks(void) {
    uint32_t total_leaked = 0;
    
    FURI_LOG_W("CORE", "Checking for memory leaks:");
    
    for(uint32_t i = 0; i < alloc_count; i++) {
        FURI_LOG_W("CORE", "  Leak: %zu bytes at %p (%s:%lu)", 
                   alloc_table[i].size, alloc_table[i].ptr,
                   alloc_table[i].file, alloc_table[i].line);
        total_leaked += alloc_table[i].size;
    }
    
    if(total_leaked > 0) {
        FURI_LOG_E("CORE", "Total memory leaked: %lu bytes", total_leaked);
    } else {
        FURI_LOG_I("CORE", "No memory leaks detected");
    }
}

// Panic handler
void furi_crash(const char* message) {
    FURI_LOG_E("CORE", "SYSTEM CRASH: %s", message);
    
    // Print system info
    system_info_t info;
    furi_get_system_info(&info);
    
    FURI_LOG_E("CORE", "System Info:");
    FURI_LOG_E("CORE", "  Version: %s", info.version);
    FURI_LOG_E("CORE", "  Build: %s", info.build);
    FURI_LOG_E("CORE", "  Ticks: %lu", info.tick_count);
    FURI_LOG_E("CORE", "  Tasks: %lu", info.task_count);
    FURI_LOG_E("CORE", "  Allocations: %lu", info.alloc_count);
    
    // Check for memory leaks
    furi_check_memory_leaks();
    
    // Halt system
    while(1) {
        furi_hal_light_blink(0, 255, 0, 3, 500); // Red blinking
        furi_delay_ms(1000);
    }
}

// Main system loop
void furi_run(void) {
    FURI_LOG_I("CORE", "Starting main system loop");
    
    while(1) {
        // System tick
        furi_tick();
        
        // Process tasks
        // TODO: Implement proper task scheduling
        
        // Power management
        furi_hal_power_check();
        
        // Small delay to prevent busy waiting
        furi_delay_ms(1);
    }
}
