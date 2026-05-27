# Sub-GHz Jammer

A Flipper Zero application that generates noise on Sub-GHz frequencies to raise the noise floor.

## Features

- **Single Screen Interface**: Simple, intuitive controls
- **18 Frequency Presets**: Ranging from 300 MHz to 925 MHz
- **Programmatic Noise Generation**: Generates random noise patterns - no external files needed
- **External CC1101 Support**: Automatically uses external CC1101 module if connected

## Controls

| Button | Action |
|--------|--------|
| **Up/Down** | Cycle through frequency presets |
| **OK** | Start/Stop jamming |
| **Left** | Show app info |
| **Right** | Show credits |
| **Back** | Exit app |

## Available Frequencies

- 300.00 MHz, 303.87 MHz, 304.25 MHz, 310.00 MHz
- 315.00 MHz, 318.00 MHz, 390.00 MHz, 418.00 MHz
- 433.07 MHz, 433.42 MHz, 433.92 MHz, 434.42 MHz
- 434.77 MHz, 438.90 MHz, 868.35 MHz, 868.92 MHz
- 915.00 MHz, 925.00 MHz

## How It Works

The jammer generates OOK (On-Off Keying) noise patterns consisting of:
- Short pulses (180-280 microseconds)
- Variable gaps (short: 500-700µs, occasional long: 48000-50000µs)

This pattern effectively raises the noise floor on the target frequency, making it harder for receivers to decode legitimate signals.

## Credits

Created by: **.leviathan**

## Legal Notice

**WARNING**: Jamming radio frequencies may be illegal in your jurisdiction. This application is provided for educational and research purposes only. Use responsibly and in compliance with all applicable laws and regulations.
