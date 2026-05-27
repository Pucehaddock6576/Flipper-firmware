/* Flipper Next-Gen OS - Hardware Abstraction Layer */

#ifndef FURI_HAL_H
#define FURI_HAL_H

#include "furi.h"
#include <stdint.h>

// GPIO
typedef enum {
    FuriHalGpioModeInput,
    FuriHalGpioModeOutput,
    FuriHalGpioModeAlt,
    FuriHalGpioModeAnalog
} FuriHalGpioMode;

typedef enum {
    FuriHalGpioPullNo,
    FuriHalGpioPullUp,
    FuriHalGpioPullDown
} FuriHalGpioPull;

typedef struct {
    uint8_t port;
    uint8_t pin;
} FuriHalGpioPin;

void furi_hal_gpio_init(FuriHalGpioPin* pin, FuriHalGpioMode mode, FuriHalGpioPull pull);
void furi_hal_gpio_write(FuriHalGpioPin* pin, bool state);
bool furi_hal_gpio_read(FuriHalGpioPin* pin);

// SPI
typedef struct FuriHalSpi FuriHalSpi;

FuriHalSpi* furi_hal_spi_init(uint32_t instance);
void furi_hal_spi_exchange(FuriHalSpi* spi, const uint8_t* tx_data, uint8_t* rx_data, size_t length);
void furi_hal_spi_write(FuriHalSpi* spi, const uint8_t* data, size_t length);
void furi_hal_spi_read(FuriHalSpi* spi, uint8_t* data, size_t length);

// I2C
typedef struct FuriHalI2c FuriHalI2c;

FuriHalI2c* furi_hal_i2c_init(uint32_t instance);
bool furi_hal_i2c_write(FuriHalI2c* i2c, uint8_t address, const uint8_t* data, size_t length);
bool furi_hal_i2c_read(FuriHalI2c* i2c, uint8_t address, uint8_t* data, size_t length);

// UART
typedef struct FuriHalUart FuriHalUart;

FuriHalUart* furi_hal_uart_init(uint32_t instance, uint32_t baudrate);
void furi_hal_uart_write(FuriHalUart* uart, const uint8_t* data, size_t length);
size_t furi_hal_uart_read(FuriHalUart* uart, uint8_t* data, size_t max_length);

// Power management
void furi_hal_power_init(void);
void furi_hal_power_check(void);
uint32_t furi_hal_power_get_battery_voltage(void);
bool furi_hal_power_is_charging(void);

// Light (LED)
typedef enum {
    FuriHalLightRed = 0,
    FuriHalLightGreen = 1,
    FuriHalLightBlue = 2,
    FuriHalLightBacklight = 3
} FuriHalLight;

void furi_hal_light_init(void);
void furi_hal_light_set(FuriHalLight light, uint8_t brightness);
void furi_hal_light_blink(uint8_t r, uint8_t g, uint8_t b, uint8_t count, uint32_t period_ms);

// Vibro
void furi_hal_vibro_init(void);
void furi_hal_vibro_on(bool state);

// Speaker
void furi_hal_speaker_init(void);
void furi_hal_speaker_set_volume(uint8_t volume);
void furi_hal_speaker_play(uint32_t frequency, uint32_t duration_ms);

// RTC
typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} FuriHalRtcDateTime;

void furi_hal_rtc_init(void);
void furi_hal_rtc_get_datetime(FuriHalRtcDateTime* datetime);
void furi_hal_rtc_set_datetime(FuriHalRtcDateTime* datetime);

// Storage (SD card)
typedef struct FuriHalStorage FuriHalStorage;

FuriHalStorage* furi_hal_storage_init(void);
bool furi_hal_storage_mount(FuriHalStorage* storage);
bool furi_hal_storage_file_exists(FuriHalStorage* storage, const char* path);
bool furi_hal_storage_file_read(FuriHalStorage* storage, const char* path, void* buffer, size_t size);
bool furi_hal_storage_file_write(FuriHalStorage* storage, const char* path, const void* buffer, size_t size);

// Sub-GHz radio
typedef struct FuriHalSubGhz FuriHalSubGhz;

FuriHalSubGhz* furi_hal_subghz_init(void);
void furi_hal_subghz_set_frequency(FuriHalSubGhz* subghz, uint32_t frequency);
void furi_hal_subghz_set_data_rate(FuriHalSubGhz* subghz, uint32_t data_rate);
void furi_hal_subghz_set_modulation(FuriHalSubGhz* subghz, uint32_t modulation);
void furi_hal_subghz_transmit(FuriHalSubGhz* subghz, const uint8_t* data, size_t length);
size_t furi_hal_subghz_receive(FuriHalSubGhz* subghz, uint8_t* data, size_t max_length);

// BLE
typedef struct FuriHalBle FuriHalBle;

FuriHalBle* furi_hal_ble_init(void);
void furi_hal_ble_start_advertising(FuriHalBle* ble);
void furi_hal_ble_stop_advertising(FuriHalBle* ble);
bool furi_hal_ble_connect(FuriHalBle* ble, const uint8_t* address);
void furi_hal_ble_disconnect(FuriHalBle* ble);
void furi_hal_ble_send_data(FuriHalBle* ble, const uint8_t* data, size_t length);

// IR
typedef struct FuriHalIr FuriHalIr;

FuriHalIr* furi_hal_ir_init(void);
void furi_hal_ir_transmit(FuriHalIr* ir, const uint32_t* timings, size_t count);
void furi_hal_ir_receive_start(FuriHalIr* ir);
bool furi_hal_ir_receive_data(FuriHalIr* ir, uint32_t* timings, size_t* count);

// System initialization
void furi_hal_init(void);

#endif // FURI_HAL_H
