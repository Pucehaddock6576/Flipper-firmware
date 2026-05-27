#!/usr/bin/env python3
"""
Simple build script for Flipper Next-Gen OS
This script bypasses SCons and directly calls the compiler
"""

import os
import subprocess
import json
from datetime import datetime

def run_command(cmd, cwd=None):
    """Run a command and return result"""
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error: {result.stderr}")
        return False
    print(f"Success: {result.stdout}")
    return True

def main():
    print("=== Flipper Next-Gen OS Build ===")
    
    # Check tools
    tools = ['arm-none-eabi-gcc', 'arm-none-eabi-ar', 'arm-none-eabi-objcopy']
    for tool in tools:
        if not run_command(['which', tool]):
            print(f"Error: {tool} not found")
            return False
    
    # Version info
    version_info = {
        "version": "2.0.0",
        "build": os.environ.get('FLIPPER_BUILD', 'development'),
        "date": datetime.now().isoformat(),
        "api_version": 2,
        "target": "flipper_zero"
    }
    
    # Write version files
    with open('version.json', 'w') as f:
        json.dump(version_info, f, indent=2)
    
    version_header = f"""
#ifndef FLIPPER_VERSION_H
#define FLIPPER_VERSION_H

#define FLIPPER_VERSION "{version_info['version']}"
#define FLIPPER_BUILD "{version_info['build']}"
#define FLIPPER_DATE "{version_info['date']}"
#define FLIPPER_API_VERSION {version_info['api_version']}

#endif // FLIPPER_VERSION_H
"""
    
    with open('core/version.h', 'w') as f:
        f.write(version_header)
    
    print(f"Building Flipper Next-Gen OS {version_info['version']}")
    
    # Compiler flags
    cflags = [
        '-mcpu=cortex-m4',
        '-mthumb',
        '-mfloat-abi=hard',
        '-mfpu=fpv4-sp-d16',
        '-Os',
        '-g3',
        '-Wall',
        '-Wextra',
        '-std=c11',
        '-DSTM32F4xx',
        '-DTARGET_FLIPPER',
        '-DFURI_RAM_PREFIX=_ram',
        '-DFURI_FLASH_PREFIX=_flash'
    ]
    
    # Include paths
    includes = [
        '-I.',
        '-Icore',
        '-Igui',
        '-Iprotocols',
        '-Iapplications',
        '-Itargets/f7/inc'
    ]
    
    # Source files
    sources = [
        'core/furi_core.c',
        'core/furi_hal.c', 
        'core/furi_log.c',
        'gui/canvas.c',
        'gui/animation.c',
        'gui/gui.c',
        'gui/fonts.c',
        'applications/rfid_bruteforce.c',
        'applications/wifi_scanner.c',
        'applications/bluetooth_scanner.c',
        'applications/ir_cloner.c',
        'applications/serial_monitor.c',
        'applications/nfc_reader.c',
        'targets/f7/test_main.c'
    ]
    
    # Compile object files
    objects = []
    for source in sources:
        if not os.path.exists(source):
            print(f"Warning: {source} not found, skipping")
            continue
            
        obj = source.replace('.c', '.o')
        objects.append(obj)
        
        cmd = ['arm-none-eabi-gcc'] + cflags + includes + ['-c', source, '-o', obj]
        if not run_command(cmd):
            return False
    
    if not objects:
        print("Error: No object files created")
        return False
    
    # Link
    link_flags = [
        '-mcpu=cortex-m4',
        '-mthumb',
        '-mfloat-abi=hard',
        '-mfpu=fpv4-sp-d16',
        '-Wl,--gc-sections',
        '-Wl,-Map=firmware.map',
        '-specs=nano.specs',
        '-specs=nosys.specs',
        '-Ttargets/f7/link.ld'
    ]
    
    cmd = ['arm-none-eabi-gcc'] + link_flags + objects + ['-o', 'flipper_nextgen.elf']
    if not run_command(cmd):
        return False
    
    # Generate binary
    cmd = ['arm-none-eabi-objcopy', '-O', 'binary', 'flipper_nextgen.elf', 'flipper_nextgen.bin']
    if not run_command(cmd):
        return False
    
    # Generate hex
    cmd = ['arm-none-eabi-objcopy', '-O', 'ihex', 'flipper_nextgen.elf', 'flipper_nextgen.hex']
    if not run_command(cmd):
        return False
    
    # Size info
    cmd = ['arm-none-eabi-size', 'flipper_nextgen.elf']
    run_command(cmd)
    
    print("\n=== Build Complete ===")
    print("Files created:")
    for ext in ['elf', 'bin', 'hex']:
        filename = f'flipper_nextgen.{ext}'
        if os.path.exists(filename):
            size = os.path.getsize(filename)
            print(f"  {filename}: {size} bytes")
    
    return True

if __name__ == '__main__':
    success = main()
    exit(0 if success else 1)
