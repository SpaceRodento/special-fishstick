# Roboter Gruppe 8 - LoRa Communication System

ESP32-based LoRa communication system using RYLR896 modules with automatic role detection.

## ✨ Key Features

**IDENTICAL CODE ON BOTH DEVICES!**

Role (sender/receiver) is automatically detected using a jumper wire:
- **GPIO 15 ↔ GPIO 17 connected** = RECEIVER
- **GPIO 15 floating** = SENDER

## 🚀 Features

- **Auto role detection** - No code changes needed between devices
- **RYLR896 LoRa** communication with proven reliable settings
- **LCD display** on receiver showing local and remote status
- **Touch sensor** and LED status monitoring
- **RSSI/SNR** signal quality monitoring
- **Modular architecture** with clean separation of concerns

## 📁 Project Structure

### Main Program
- `Roboter_Gruppe_8_LoRa.ino` - Main application with auto role detection

### Header Files
- `config.h` - Pin definitions and configuration constants
- `structs.h` - Data structures for device state and timing
- `lora_handler.h` - RYLR896 LoRa communication handler
- `functions.h` - LCD and helper functions

### Reference/Testing
- `RYLR896_simple.ino` - Simple test code for basic LoRa validation
- `KAYTTOOHJE.md` - Finnish instructions for simple test code
- `platformio.ini` - PlatformIO configuration

## 🔌 Hardware Setup

### RYLR896 LoRa Module Connections
```
RYLR896 -> ESP32
-----------------
TX  -> GPIO 25 (RXD2)
RX  -> GPIO 26 (TXD2)
VCC -> 3.3V
GND -> GND
```

### Role Detection
```
GPIO 17 (MODE_GND_PIN)    -> Set as OUTPUT LOW (provides GND)
GPIO 15 (MODE_SELECT_PIN) -> Read with INPUT_PULLUP

RECEIVER: Connect GPIO 15 ↔ GPIO 17 with jumper wire
SENDER:   Leave GPIO 15 floating (no connection)
```

### Additional Hardware
- **LED**: GPIO 2 (built-in LED)
- **Touch Sensor**: T0 (GPIO 4)
- **LCD**: I2C address 0x27 (16x2 display, receiver only)

## ⚡ Quick Start

1. **Upload identical code to both ESP32 devices**
   - Use `Roboter_Gruppe_8_LoRa.ino`

2. **Configure Device 1 (Receiver)**
   - Connect GPIO 15 to GPIO 17 with jumper wire
   - Connect I2C LCD display

3. **Configure Device 2 (Sender)**
   - Leave GPIO 15 floating (no jumper)

4. **Power up and test**
   - Open Serial Monitor (115200 baud) on both devices
   - Receiver will display status on LCD
   - Check signal quality (RSSI/SNR) in Serial Monitor

## 📊 LoRa Settings

Optimized for maximum range and reliability:
- **Spreading Factor**: 12 (max range, slower speed)
- **Bandwidth**: 125 kHz
- **Coding Rate**: 4/5
- **Network ID**: 6 (must match on both devices)
- **Addresses**: Receiver=1, Sender=2 (auto-configured)

## 🔧 Development

Developed and tested with:
- ESP32 DevKit v1
- RYLR896 LoRa modules (868 MHz)
- Arduino IDE / PlatformIO
- LiquidCrystal_I2C library

## 📝 Notes

- Both devices run **identical code** - role is auto-detected
- Simple jumper wire determines sender vs receiver role
- No need to modify code when switching roles
- Proven reliable LoRa settings based on extensive testing