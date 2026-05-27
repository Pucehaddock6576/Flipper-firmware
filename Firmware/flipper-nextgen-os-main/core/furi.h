/* Flipper Next-Gen OS - Core Headers */

#ifndef FURI_H
#define FURI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Task management enums first
typedef enum {
    FURI_TASK_STATE_READY,
    FURI_TASK_STATE_RUNNING,
    FURI_TASK_STATE_SUSPENDED,
    FURI_TASK_STATE_FINISHED
} FuriTaskState;

typedef enum {
    FURI_TASK_PRIORITY_LOW = 0,
    FURI_TASK_PRIORITY_NORMAL = 1,
    FURI_TASK_PRIORITY_HIGH = 2,
    FURI_TASK_PRIORITY_CRITICAL = 3
} FuriTaskPriority;

// Function pointer type
typedef void (*furi_task_func_t)(void* context);

// Task structure
typedef struct furi_task {
    const char* name;
    FuriTaskState state;
    FuriTaskPriority priority;
    size_t stack_size;
    void* stack;
    furi_task_func_t func;
    void* context;
} furi_task_t;

#define MAX_TASKS 16
#define DEFAULT_STACK_SIZE 4096

// Memory management
void* furi_alloc(size_t size);
void furi_free(void* ptr);

// Task management
furi_task_t* furi_task_alloc(const char* name);
void furi_task_start(furi_task_t* task, furi_task_func_t func, void* context);
void furi_task_set_priority(furi_task_t* task, FuriTaskPriority priority);

// Record system (service locator)
void* furi_record_open(const char* name);
void furi_record_close(const char* name);
bool furi_record_create(const char* name, void* record);

// System initialization
void furi_init(void);
void furi_run(void);
void furi_tick(void);

// System information
typedef struct {
    const char* version;
    const char* build;
    uint32_t tick_count;
    uint32_t task_count;
    uint32_t alloc_count;
    uint32_t free_heap;
} system_info_t;

void furi_get_system_info(system_info_t* info);

// Logging
typedef enum {
    FURI_LOG_LEVEL_ERROR = 0,
    FURI_LOG_LEVEL_WARN = 1,
    FURI_LOG_LEVEL_INFO = 2,
    FURI_LOG_LEVEL_DEBUG = 3
} FuriLogLevel;

#define FURI_LOG_E(tag, ...) furi_log(FURI_LOG_LEVEL_ERROR, tag, __VA_ARGS__)
#define FURI_LOG_W(tag, ...) furi_log(FURI_LOG_LEVEL_WARN, tag, __VA_ARGS__)
#define FURI_LOG_I(tag, ...) furi_log(FURI_LOG_LEVEL_INFO, tag, __VA_ARGS__)
#define FURI_LOG_D(tag, ...) furi_log(FURI_LOG_LEVEL_DEBUG, tag, __VA_ARGS__)

void furi_log(FuriLogLevel level, const char* tag, const char* format, ...);

// Utilities
void furi_delay_ms(uint32_t ms);
void furi_delay_us(uint32_t us);
uint32_t furi_get_tick(void);

// Error handling
void furi_crash(const char* message);
void furi_check_memory_leaks(void);

#endif // FURI_H
