/* Input System Header - Button and Input Handling */

#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H

#include <stdint.h>
#include <stdbool.h>

// Input types
typedef enum {
    InputTypePress = 0,
    InputTypeRelease = 1,
    InputTypeShort = 2,
    InputTypeLong = 3,
    InputTypeRepeat = 4
} InputType;

// Input keys
typedef enum {
    InputKeyBack = 0x01,
    InputKeyUp = 0x02,
    InputKeyDown = 0x04,
    InputKeyLeft = 0x08,
    InputKeyRight = 0x10,
    InputKeyOk = 0x20,
    InputKeyMAX = 0x40
} InputKey;

// Input event
typedef struct {
    InputType type;
    InputKey key;
    uint16_t sequence;
} InputEvent;

#endif // INPUT_INPUT_H
