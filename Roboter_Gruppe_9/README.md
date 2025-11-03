# Roboter Gruppe 9 - LoRa Communication System

ESP32-based LoRa communication system using RYLR896 modules with automatic role detection, connection watchdog, and kill-switch.

## ✨ Key Features

**IDENTICAL CODE ON BOTH DEVICES!**

Role (sender/receiver) is automatically detected using a jumper wire:
- **GPIO 16 ↔ GPIO 17 connected** = RECEIVER (jumper wire)
- **GPIO 16 floating** = SENDER (no connection)
- **Note:** GPIO 16 and GPIO 17 are physically next to each other on ESP32

## 🚀 Features

- **Auto role detection** - No code changes needed between devices
- **RYLR896 LoRa** communication with proven reliable settings
- **LCD display** on receiver with 4 different display versions
- **Connection watchdog** - Automatic state tracking (CONNECTED/WEAK/LOST) and recovery
- **Kill-switch** - Physical button to restart device (GPIO 13↔14, hold 3s)
- **PC data logging** - CSV/JSON output for Python data analysis
- **Dual spinner animations** - Local (fast) and remote (via LoRa) indicators
- **Touch sensor** and LED status monitoring
- **RSSI/SNR** signal quality monitoring (working correctly)
- **Packet loss tracking** with sequence numbers
- **Health monitoring** - RSSI statistics, packet loss %, uptime tracking
- **Modular architecture** with clean separation of concerns

## 📁 Project Structure

### Main Program
- `Roboter_Gruppe_9.ino` - Main application with auto role detection

### Header Files
- `config.h` - Pin definitions and configuration constants
- `structs.h` - Data structures for device state and timing
- `lora_handler.h` - RYLR896 LoRa communication handler
- `functions.h` - LCD and helper functions

### Python Scripts
- `serial_monitor.py` - Real-time colored serial data viewer
- `data_logger.py` - SQLite database logger for data analysis

### Documentation
- `README.md` - This file, project overview
- `PC_LOGGING_README.md` - **PC data logging guide with Python scripts**
- `LCD_VERSIONS.md` - Detailed guide for all 4 LCD display versions
- `STATUS_SUMMARY.md` - Current project status and features
- `WATCHDOG_GUIDE.md` - Connection watchdog and health monitoring guide
- `FUTURE_DEVELOPMENT.md` - Development roadmap

### Reference/Testing
- `RYLR896_simple.ino` - Simple test code for basic LoRa validation
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
GPIO 16 (MODE_SELECT_PIN) -> Read with INPUT_PULLUP

RECEIVER: Connect GPIO 16 ↔ GPIO 17 with jumper wire
SENDER:   Leave GPIO 16 floating (no connection)

Note: GPIO 16 and GPIO 17 are physically next to each other!
```

### Kill-Switch
```
GPIO 14 (KILLSWITCH_GND_PIN)  -> Set as OUTPUT LOW (provides GND)
GPIO 13 (KILLSWITCH_READ_PIN) -> Read with INPUT_PULLUP

To restart device: Connect GPIO 13 ↔ GPIO 14 and hold 3 seconds
Note: GPIO 12 is a strapping pin on ESP32, so GPIO 13 is used instead
```

### Additional Hardware
- **LED**: GPIO 2 (built-in LED)
- **Touch Sensor**: T0 (GPIO 4)
- **LCD**: I2C address 0x27 (16x2 display, receiver only)

## ⚡ Quick Start

1. **Upload identical code to both ESP32 devices**
   - Use `Roboter_Gruppe_9.ino`

2. **Configure Device 1 (Receiver)**
   - Connect GPIO 16 to GPIO 17 with jumper wire
   - Connect I2C LCD display

3. **Configure Device 2 (Sender)**
   - Leave GPIO 16 floating (no jumper)

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
- Simple jumper wire determines sender vs receiver role (GPIO 16 ↔ 17)
- No need to modify code when switching roles
- Proven reliable LoRa settings based on extensive testing
- RSSI/SNR parsing fixed to handle comma-separated data correctly
- 4 LCD display versions available (see `LCD_VERSIONS.md`)
- Dual spinner animations show local activity (fast) and remote updates (slow)
- **Kill-switch**: GPIO 13↔14, hold 3 seconds to restart device
- **PC data logging**: CSV/JSON output for Python analysis (see `PC_LOGGING_README.md`)
- **Connection watchdog**: Automatic state tracking and recovery (see `WATCHDOG_GUIDE.md`)

## 🔴 Kill-Switch Usage

The kill-switch allows you to restart the device without re-uploading code.

**How to use:**
1. Connect GPIO 13 to GPIO 14 with a jumper wire (or button)
2. Hold for 3 seconds
3. Watch Serial Monitor for countdown
4. Device restarts automatically

**Serial Monitor output:**
```
🔴 Kill-switch PRESSED - hold to restart...
🔴 2 more seconds...
🔴 1 more second...
╔════════════════════════════════╗
║  🔴 RESTART: Physical kill-switch  ║
╚════════════════════════════════╝
[Device restarts]
```

**Use cases:**
- Emergency stop during testing
- Quick restart without power cycling
- Safety feature for robot control
- Remote restart via LoRa command: Send "CMD:RESTART"

## 💻 PC Data Logging

Connect ESP32 to your computer and log data in real-time!

**Quick Start:**
```bash
# Install pyserial
pip install pyserial

# Real-time monitoring
python serial_monitor.py /dev/ttyUSB0 115200

# Database logging
python data_logger.py /dev/ttyUSB0 115200 lora_data.db
```

**Features:**
- CSV data output every 2 seconds (configurable)
- Real-time colored terminal display
- SQLite database logging for analysis
- RSSI, SNR, packet loss, connection state tracking

**See `PC_LOGGING_README.md` for full documentation.**