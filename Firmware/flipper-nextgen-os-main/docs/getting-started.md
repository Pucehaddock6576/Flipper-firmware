# Flipper Next-Gen OS - Getting Started

## 🚀 Quick Start

### Förutsättningar
- **Hardware:** Flipper Zero device
- **OS:** Linux, macOS, eller Windows 10+
- **Tools:** Git, Python 3.9+, Docker (valfritt)

### Installation

1. **Klona repository**
```bash
git clone https://github.com/gracestack/flipper-nextgen-os.git
cd flipper-nextgen-os
```

2. **Installera dependencies**
```bash
# Ubuntu/Debian
sudo apt-get install gcc-arm-none-eabi scons python3-pip

# macOS
brew install arm-none-eabi-gcc scons python3

# Windows
# Använd WSL2 eller Docker
```

3. **Installera Python dependencies**
```bash
pip3 install -r requirements.txt
```

4. **Bygg firmware**
```bash
scons -j$(nproc)
```

## 📱 Flasha Firmware

### Metod 1: USB Flashing
```bash
# Anslut Flipper Zero via USB
python tools/flash.py --port /dev/ttyACM0 --firmware flipper_firmware.bin
```

### Metod 2: SD Card
```bash
# Kopiera firmware till SD-kort
cp flipper_firmware.bin /media/FLIPPER/
```

### Metod 3: Debugger (för utvecklare)
```bash
# Använd OpenOCD med ST-Link
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program flipper_firmware.elf verify reset exit"
```

## 🛠️ Utveckling

### Projektstruktur
```
flipper-nextgen-os/
├── core/           # Kärnsystem (C + Rust)
├── applications/   # Applikationer
├── gui/           # GUI-system
├── protocols/     # Kommunikationsprotokoll
├── drivers/       # Hardwaredrivrutiner
├── build/         # Byggsystem
└── docs/          # Dokumentation
```

### Skapa en ny applikation

1. **App manifest**
```json
{
    "name": "Min App",
    "version": "1.0.0",
    "author": "Ditt Namn",
    "entry_point": "app_main",
    "icon": "assets/icon.png",
    "permissions": ["storage.read"]
}
```

2. **App källkod**
```c
#include <furi.h>
#include <gui/gui.h>

int32_t app_main(void* p) {
    UNUSED(p);
    
    // Initiera GUI
    Gui* gui = furi_record_open("gui");
    ViewPort* viewport = view_port_alloc();
    
    // Lägg till view
    gui_add_view_port(gui, viewport, GuiLayerFullscreen);
    
    // Main loop
    while(true) {
        // Hantera events
        furi_delay_ms(100);
    }
    
    return 0;
}
```

3. **Bygg app**
```bash
scons apps/min_app
```

### Debugging

#### Serial Debug
```bash
# Anslut till serial console
minicom -D /dev/ttyACM0 -b 115200
```

#### SWD Debug
```bash
# Starta GDB server
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

# Anslut GDB
arm-none-eabi-gdb flipper_firmware.elf
(gdb) target remote localhost:3333
(gdb) load
(gdb) continue
```

## 📚 API Referens

### Core API
```c
// Task management
furi_task_t* furi_task_alloc(const char* name);
void furi_task_start(furi_task_t* task, furi_task_func_t func, void* context);

// Memory management
void* furi_alloc(size_t size);
void furi_free(void* ptr);

// Record system (service locator)
void* furi_record_open(const char* name);
void furi_record_close(const char* name);
```

### GUI API
```c
// Canvas operations
void canvas_draw_frame(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void canvas_draw_str(Canvas* canvas, uint8_t x, uint8_t y, const char* str);
void canvas_draw_icon(Canvas* canvas, uint8_t x, uint8_t y, const Icon* icon);

// View management
ViewPort* view_port_alloc();
void view_port_draw_callback_set(ViewPort* viewport, ViewPortDrawCallback callback, void* context);
void view_port_input_callback_set(ViewPort* viewport, ViewPortInputCallback callback, void* context);
```

### Protocol API
```c
// Sub-GHz
SubGhzEnvironment* subghz_environment_alloc();
void subghz_environment_load(SubGhzEnvironment* environment, const char* filename);
bool subghz_transmit(SubGhzEnvironment* environment, const char* protocol_name, const uint8_t* data);

// BLE
BleProfile* ble_profile_alloc();
void ble_profile_advertise_start(BleProfile* profile);
bool ble_profile_connect(BleProfile* profile, const uint8_t* address);
```

## 🧪 Testing

### Unit Tests
```bash
# Kör alla tester
python -m pytest tests/

# Kör specifikt test
python -m pytest tests/test_core.py::test_memory_allocation
```

### Integration Tests
```bash
# Starta emulator
python tools/emulator.py --firmware flipper_firmware.bin

# Kör integrationstester
python tests/integration/test_boot.py
```

## 📦 Bygga för Release

### Versionering
```bash
# Sätt version
export FLIPPER_VERSION="2.0.0"
export FLIPPER_BUILD="release"

# Bygg release
scons target=release
```

### Paketering
```bash
# Skapa release paket
python tools/package.py --version 2.0.0 --output flipper-nextgen-2.0.0.zip
```

## 🔧 Felsökning

### Vanliga Problem

#### Build Failures
```bash
# Rensa build cache
scons -c

# Uppdatera submoduler
git submodule update --init --recursive

# Verifiera toolchain
arm-none-eabi-gcc --version
```

#### Flash Failures
```bash
# Kontrollera anslutning
ls /dev/ttyACM*

# Testa kommunikation
echo "version\r" > /dev/ttyACM0
cat /dev/ttyACM0
```

#### Runtime Issues
```bash
# Visa systemloggar
cat /tmp/flipper.log

# Memory analysis
python tools/memory_analyzer.py flipper_firmware.elf
```

## 📖 Resurser

### Dokumentation
- [API Reference](docs/api.md)
- [Architecture Guide](docs/architecture.md)
- [Protocol Documentation](docs/protocols.md)
- [GUI Development](docs/gui.md)

### Community
- [Discord](https://discord.gg/flipper)
- [Forum](https://forum.flipperzero.one)
- [GitHub Discussions](https://github.com/gracestack/flipper-nextgen-os/discussions)

### Externa Resurser
- [Official Flipper Zero Docs](https://docs.flipper.net)
- [Unleashed Firmware](https://github.com/DarkFlippers/unleashed-firmware)
- [Awesome Flipper](https://awesome-flipper.com)

## 🤝 Bidra

1. **Fork** repository
2. **Skapa branch** för din feature
3. **Implementera** dina ändringar
4. **Testa** noggrant
5. **Submit** Pull Request

### Code Style
- C: Follow Linux kernel style
- Rust: Use rustfmt
- Python: PEP 8
- Comments: Swedish for business logic, English for infrastructure

### Commit Messages
```
feat: add new protocol support
fix: resolve memory leak in GUI
docs: update API documentation
test: add unit tests for core functions
```

## 📄 Licens

GPL-3.0 - Se [LICENSE](LICENSE) för detaljer.

---

**Gracestack AB © 2026**
