# Roboter Gruppe 9 - LoRa Communication System

**Professional wireless sensor network with real-time monitoring and autonomous health management**

ESP32-based LoRa communication system using RYLR896 modules with automatic role detection, bi-directional communication, connection watchdog, and comprehensive data logging. Perfect for robotics, IoT, and remote monitoring applications.

📊 **Project Stats:** 1,501 lines of C++ code • 462 lines of Python • 35 functions • 7 documentation files

---

## 📖 What is this?

This is a **complete wireless communication system** that allows two ESP32 microcontrollers to communicate over long distances (up to several kilometers) using LoRa radio technology. The system features:

- **Plug-and-play deployment** - Upload identical code to both devices, role auto-detected by hardware
- **Professional monitoring** - Real-time signal quality, packet loss tracking, connection health
- **Self-healing** - Automatic recovery from connection failures
- **Data logging** - Record all communication to PC via USB with Python scripts
- **Safety features** - Physical and remote kill-switch for emergency stops
- **Two-way communication** - Both devices send and receive with ACK support

**Use cases:**
- Robot remote control with telemetry feedback
- Environmental sensor networks (temperature, humidity, etc.)
- Building automation with long-range connectivity
- Agricultural monitoring (soil, weather stations)
- Remote equipment control and monitoring
- Educational projects for learning wireless communication

**Reading time:** ⏱️ ~4 minutes

---

## ✨ Key Features

**IDENTICAL CODE ON BOTH DEVICES!**

Role (sender/receiver) is automatically detected using a jumper wire:
- **GPIO 16 ↔ GPIO 17 connected** = RECEIVER (jumper wire)
- **GPIO 16 floating** = SENDER (no connection)
- **Note:** GPIO 16 and GPIO 17 are physically next to each other on ESP32

## 🚀 Features

- **Auto role detection** - No code changes needed between devices
- **RYLR896 LoRa** communication with proven reliable settings
- **Bi-directional communication** - Both devices can send AND receive (ACK support)
- **LCD display** on receiver with 4 different display versions
- **Connection watchdog** - Automatic state tracking (CONNECTED/WEAK/LOST) and recovery
- **Kill-switch** - Physical button to restart device (GPIO 13↔14, hold 3s) + remote commands
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
- `README.md` - This file, project overview and quick start
- **`COMPREHENSIVE_MANUAL.md`** - **📖 Complete guide** (troubleshooting, setup, advanced features)
- `PC_LOGGING_README.md` - PC data logging guide with Python scripts
- `archive/` - Archived documentation (LCD versions, watchdog guide, etc.)

> 💡 **For detailed instructions**, see `COMPREHENSIVE_MANUAL.md`

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
- **Bi-directional**: Sender listens for ACK, receiver sends ACK every 5 messages

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
**See `PC_LOGGING_README.md` for full documentation.**

---

## 📊 Project Statistics

### Code Metrics
```
Language         Files    Lines    Purpose
─────────────────────────────────────────────────
C++/Arduino         6    1,501    Main firmware
Python              2      462    Data logging
Documentation       7    2,172    Guides & docs
─────────────────────────────────────────────────
Total              15    4,135    lines
```

### Architecture Breakdown
- **Main Program:** 730 lines (`Roboter_Gruppe_9.ino`)
- **LoRa Handler:** 264 lines (AT commands, send/receive)
- **Health Monitor:** 310 lines (watchdog, recovery, statistics)
- **Structs:** 111 lines (data structures)
- **Config:** 57 lines (pin definitions, settings)
- **LCD Functions:** 29 lines (display helpers)

### Features Implemented
- ✅ 35+ functions
- ✅ 10 data structures
- ✅ 4 LCD display versions
- ✅ 2 Python data analysis scripts
- ✅ 7 comprehensive documentation files
- ✅ Bi-directional communication with ACK
- ✅ Automatic health monitoring & recovery
- ✅ Real-time packet loss tracking
- ✅ Physical + remote kill-switch
- ✅ CSV/JSON data export

### Development Timeline
**Total development sessions:** ~6-8 hours of focused coding

**Features by session:**
1. Basic LoRa communication setup
2. Auto role detection & LCD display
3. Connection watchdog & health monitoring
4. Kill-switch implementation (physical + remote)
5. PC data logging with Python scripts
6. Bug fixes (struct initialization, timeouts)
7. Bi-directional communication (ACK support)

### Code Quality
- **Modular design:** Separate files for LoRa, health, config
- **Error handling:** Timeouts, bounds checking, validation
- **Self-documenting:** Clear variable names, inline comments
- **Tested features:** Kill-switch works without LoRa module
- **Memory safe:** Array bounds validation, buffer overflow protection

### Performance
- **Loop frequency:** ~100 Hz (10ms delay)
- **Send interval:** 2 seconds (configurable)
- **ACK response time:** <500ms
- **LCD update rate:** 10 Hz (100ms)
- **Data logging rate:** 0.5 Hz (2 seconds, configurable)

### Hardware Support
- **Microcontroller:** ESP32 (any variant)
- **LoRa Module:** RYLR896 (868 MHz)
- **Display:** I2C LCD 16x2 (optional, receiver only)
- **Power:** 3.3V, ~100mA typical
- **Range:** Up to 5+ km (line of sight, SF12)

---

## 🎓 Technical Details

### Communication Protocol
**LoRa Settings:**
- Spreading Factor: 12 (maximum range)
- Bandwidth: 125 kHz
- Coding Rate: 4/5
- Network ID: 6
- Addresses: Sender=2, Receiver=1

**Message Format:**
```
Sender → Receiver:  SEQ:42,LED:1,TOUCH:0,SPIN:2,COUNT:42
Receiver → Sender:  ACK,SEQ:5,LED:0,TOUCH:1,SPIN:3
```

### System Architecture
```
┌─────────────────────────────────────────────────────┐
│              Roboter Gruppe 9 System                │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌──────────────┐         LoRa         ┌──────────────┐
│  │   SENDER     │◄──────────────────────►│   RECEIVER   │
│  │   ESP32      │      868 MHz          │   ESP32      │
│  │              │                       │              │
│  │ • Touch      │  Data + ACK           │ • LCD        │
│  │ • LED        │                       │ • Touch      │
│  │ • Health     │                       │ • LED        │
│  └──────┬───────┘                       └──────┬───────┘
│         │                                      │
│         │ USB                                  │ USB
│         ▼                                      ▼
│  ┌─────────────────┐                   ┌─────────────────┐
│  │  Serial Monitor │                   │  Data Logger    │
│  │  (Python)       │                   │  (SQLite)       │
│  └─────────────────┘                   └─────────────────┘
└─────────────────────────────────────────────────────────┘
```

### Data Flow
1. **Sender** reads sensors → packages data → sends via LoRa
2. **Receiver** receives → parses → updates display → sends ACK
3. **Sender** receives ACK → updates statistics
4. **Both** log to PC via USB (optional)
5. **Python** scripts capture data → store in database → visualize

### Memory Usage
- **Flash:** ~250 KB (program)
- **RAM:** ~45 KB (variables, buffers)
- **Efficiency:** Optimized for ESP32 constraints

---

## 🏆 Why This Project is Special

### 1. **Truly Identical Code**
Most LoRa projects require separate sender/receiver code. This uses **automatic role detection** with a simple jumper wire - no code changes needed!

### 2. **Production-Ready**
Not just a proof-of-concept. Includes:
- Error handling & recovery
- Health monitoring
- Data logging
- Safety features (kill-switch)
- Comprehensive documentation

### 3. **Professional Architecture**
Clean separation of concerns:
- `config.h` - All settings in one place
- `lora_handler.h` - Communication logic
- `health_monitor.h` - Connection management
- `structs.h` - Data structures

### 4. **Self-Healing System**
If connection is lost:
1. State changes to `CONN_LOST`
2. Automatic recovery attempts (3 tries)
3. Re-initializes LoRa module
4. Returns to normal operation

### 5. **Data Science Ready**
CSV/JSON export + Python scripts = Easy analysis:
```python
import pandas as pd
df = pd.read_csv('data.csv')
df['rssi'].plot()  # Instant signal quality graph
```

### 6. **Educational Value**
Learn about:
- LoRa radio communication
- Real-time systems
- Finite state machines
- Error handling
- Data logging
- Wireless protocols

---

## 🚀 Getting Started (Quick Version)

**5-minute setup:**

1. **Hardware** - Connect LoRa modules to ESP32
2. **Upload** - Flash same code to both devices
3. **Configure** - Add jumper on receiver (GPIO 16↔17)
4. **Power** - Turn on both devices
5. **Monitor** - Open Serial Monitor (115200 baud)

**Done!** You should see messages flowing and LCD updating.

**Detailed guides available in documentation files.**

---

## 🤝 Contributing & Support

This project demonstrates professional embedded systems development practices. Feel free to:

- Fork and modify for your needs
- Report issues or suggest improvements
- Use in educational settings
- Adapt for commercial projects

**Documentation structure:**
- `README.md` - Overview (this file)
- `WATCHDOG_GUIDE.md` - Connection monitoring details
- `LCD_VERSIONS.md` - Display options
- `PC_LOGGING_README.md` - Data logging guide
- `FUTURE_DEVELOPMENT.md` - Roadmap & ideas

---

## 📜 License & Credits

**Development:** Professional embedded systems project
**Hardware:** ESP32 + RYLR896 LoRa modules
**Libraries:** Arduino, LiquidCrystal_I2C
**Tools:** PlatformIO, Arduino IDE, Python 3

Built with attention to detail, proper error handling, and real-world use cases in mind.

---

**⭐ Star this project if you find it useful!**

*Last updated: November 2025*
