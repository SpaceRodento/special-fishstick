# Project Structure - Roboter Gruppe 9

**Comprehensive guide to project architecture and file organization**

Last updated: 2025-11-06

---

## 📂 Project Overview

```
Roboter_Gruppe_9/
├── 🎯 Core Files (Required)
│   ├── Roboter_Gruppe_9.ino    - Main program
│   ├── config.h                - Configuration & feature toggles
│   ├── structs.h               - Data structures
│   ├── functions.h             - Helper functions (LCD, etc.)
│   ├── lora_handler.h          - LoRa communication
│   └── health_monitor.h        - Connection watchdog
│
├── 🔧 Feature Modules (Optional, toggle in config.h)
│   ├── display_sender.h        - TFT display UART output
│   ├── DisplayClient.h         - Display communication library
│   ├── battery_monitor.h       - Battery voltage monitoring
│   ├── audio_detector.h        - Fire alarm sound detection
│   ├── light_detector.h        - Fire alarm light detection
│   ├── current_monitor.h       - INA219 current/power monitoring
│   ├── extended_telemetry.h    - Additional system info
│   ├── adaptive_sf.h           - Dynamic spreading factor
│   ├── advanced_commands.h     - Extended remote commands
│   ├── encryption.h            - Simple XOR encryption
│   ├── packet_stats.h          - Detailed packet statistics
│   ├── performance_monitor.h   - CPU/memory tracking
│   ├── runtime_config.h        - Serial configuration commands
│   └── watchdog_timer.h        - Hardware watchdog timer
│
├── 📖 Documentation
│   ├── README.md                      - Project overview & quick start
│   ├── PROJECT_STRUCTURE.md           - This file
│   ├── TESTING_CHECKLIST.md           - What to test and how
│   ├── COMPREHENSIVE_MANUAL.md        - Complete technical manual
│   ├── FEATURE_TESTING_GUIDE.md       - Testing individual features
│   ├── HARDWARE_TESTING_CHECKLIST.md  - Hardware validation (older)
│   └── PC_LOGGING_README.md           - Python data logging guide
│
├── 🐍 Python Scripts (PC-side)
│   ├── serial_monitor.py       - Real-time colored serial viewer
│   ├── data_logger.py          - SQLite database logger
│   ├── realtime_plotter.py     - Live RSSI/SNR graphs
│   ├── analyze_data.py         - Statistical analysis
│   └── lora_analysis.ipynb     - Jupyter notebook analysis
│
├── 📦 Other
│   ├── platformio.ini          - PlatformIO configuration
│   └── archive/                - Old/deprecated documentation
│
└── 🖥️ Display Station (Separate project)
    └── ../Roboter_Display_TFT/
        └── Roboter_Display_TFT.ino  - TFT display receiver code
```

---

## 🎯 Core Files (Always Compiled)

### `Roboter_Gruppe_9.ino` (789 lines)
**Main application logic**

- Auto role detection (sender/receiver)
- Main setup() and loop()
- Kill-switch implementation
- LCD display management (4 versions)
- Touch sensor and LED handling
- PC data logging (CSV/JSON)
- Integration of all feature modules

**Key functions:**
- `setup()` - Initialize all systems
- `loop()` - Main event loop
- `checkKillSwitch()` - Physical kill-switch monitoring
- `updateLCD()` - Display management
- `parsePayload()` - Parse received LoRa data

### `config.h` (182 lines)
**Central configuration file**

All pin definitions and feature toggles in ONE place.

**Pin Definitions:**
- LoRa: GPIO 25 (RX), GPIO 26 (TX)
- Mode detection: GPIO 16+17
- Kill-switch: GPIO 13+14
- Display: GPIO 23 (TX) → Display GPIO 18 (RX)

**Feature Toggles:**
```cpp
#define ENABLE_DISPLAY_OUTPUT true       // TFT display station
#define ENABLE_BIDIRECTIONAL true        // Two-way communication
#define ENABLE_CSV_OUTPUT true           // PC data logging
#define ENABLE_BATTERY_MONITOR false     // Battery monitoring
#define ENABLE_AUDIO_DETECTION false     // Fire alarm audio
#define ENABLE_LIGHT_DETECTION false     // Fire alarm light
#define ENABLE_CURRENT_MONITOR false     // INA219 power monitoring
// ... and 6 more features
```

**Communication Settings:**
- LoRa addresses, network ID
- Baudrates, timeouts
- ACK intervals

### `structs.h` (111 lines)
**Data structures**

Core data types used throughout the project:

```cpp
struct DeviceState {
  uint8_t ledState;          // LED on/off
  int ledCount;              // Blink counter
  bool touchState;           // Touch sensor
  int touchValue;            // Raw touch value
  int messageCount;          // Messages sent/received
  unsigned long lastMessageTime;
  int sequenceNumber;        // Packet tracking
  int spinnerIndex;          // Animation state
  int rssi;                  // Signal strength
  int snr;                   // Signal-to-noise ratio
};

struct TimingData { /* timing variables */ };
struct SpinnerData { /* animation symbols */ };
struct HealthMonitor { /* connection state */ };
```

### `functions.h` (145 lines)
**Helper functions**

LCD initialization and utility functions:

- `initLCD()` - I2C LCD setup
- `getSignalBar()` - Visual signal strength bar
- `getSignalIcon()` - Signal quality icon
- `getConnectionIcon()` - Connection state icon
- `getConnectionStateString()` - State to string
- `getPacketLoss()` - Calculate packet loss %

### `lora_handler.h` (264 lines)
**LoRa communication layer**

Complete RYLR896 module interface:

**Functions:**
- `initLoRa()` - Module initialization with AT commands
- `sendLoRaMessage()` - Send data to specific address
- `receiveLoRaMessage()` - Parse incoming messages with RSSI/SNR
- `sendLoRaCommand()` - Send AT command and get response
- `waitForReady()` - Wait for +READY signal

**LoRa Settings:**
- Spreading Factor: 12 (max range)
- Bandwidth: 125 kHz
- Coding Rate: 4/5
- Network ID: 6

### `health_monitor.h` (310 lines)
**Connection watchdog and health tracking**

Monitors connection quality and auto-recovery:

**Connection States:**
- `CONN_UNKNOWN` - Initial state
- `CONN_CONNECTING` - Waiting for first message
- `CONN_CONNECTED` - Normal operation
- `CONN_WEAK` - Delayed messages (3-8s)
- `CONN_LOST` - No messages >8s

**Functions:**
- `initHealthMonitor()` - Initialize monitoring
- `updateConnectionState()` - Check connection health
- `attemptRecovery()` - Auto-reconnect (3 attempts)
- `trackPacket()` - Sequence number tracking
- `updateRSSI()` - Signal statistics
- `printHealthReport()` - Detailed status report

**Features:**
- RSSI averaging (10 samples)
- Packet loss tracking
- Missed sequence detection
- Recovery attempts with backoff
- Uptime tracking

---

## 🔧 Feature Modules (Optional)

All features can be enabled/disabled in `config.h` without breaking compilation.

### Display Station

#### `display_sender.h` (243 lines)
**TFT display UART output**

Sends real-time data to external TFT display station (ESP32-2432S022).

**Toggle:** `ENABLE_DISPLAY_OUTPUT`

**Connection:**
- Robot TX (GPIO 23) → Display RX (GPIO 18)
- UART2, 115200 baud, TX-only

**Functions:**
- `initDisplaySender()` - Initialize UART connection
- `sendDisplayUpdate()` - Send data every 2s
- `sendDisplayAlert()` - Immediate alert message
- `clearDisplayAlert()` - Remove alert

**Data sent:**
- Role (sender/receiver)
- Sequence number, message count
- LED and touch sensor states
- RSSI, SNR
- Battery voltage (if enabled)
- Current/power (if enabled)
- Uptime, free heap (if enabled)

#### `DisplayClient.h` (194 lines)
**Display communication library**

Simple library for sending CSV-formatted data over UART.

**Usage:**
```cpp
DisplayClient display(23);  // TX pin
display.begin();
display.set("LED", "ON");
display.set("RSSI", -78);
display.send();
```

**Features:**
- Automatic CSV formatting
- Buffering for efficient transmission
- Alert system
- Template-based for any data type

### Sensor Modules

#### `battery_monitor.h`
**Battery voltage monitoring**

**Toggle:** `ENABLE_BATTERY_MONITOR`
**Hardware:** Voltage divider on GPIO 35 (ADC1_CH7)

Monitors battery voltage, reports low/critical levels.

#### `audio_detector.h`
**Fire alarm sound detection**

**Toggle:** `ENABLE_AUDIO_DETECTION`
**Hardware:** MAX4466 microphone on GPIO 34

Detects smoke alarm audio pattern (3kHz, 85dB, 3-4 beeps/second).

#### `light_detector.h`
**Fire alarm light detection**

**Toggle:** `ENABLE_LIGHT_DETECTION`
**Hardware:** TCS34725 RGB sensor on I2C

Detects flashing red LED (typical fire alarm visual indicator).

#### `current_monitor.h`
**Power consumption monitoring**

**Toggle:** `ENABLE_CURRENT_MONITOR`
**Hardware:** INA219 current sensor on I2C

Tracks current, voltage, power, and total energy usage (mAh).

### System Features

#### `extended_telemetry.h`
**Additional system information**

**Toggle:** `ENABLE_EXTENDED_TELEMETRY`

Adds uptime, free heap, internal temperature to payload.

#### `adaptive_sf.h`
**Dynamic spreading factor**

**Toggle:** `ENABLE_ADAPTIVE_SF`

Automatically adjusts SF based on RSSI for optimal throughput.

#### `advanced_commands.h`
**Extended remote commands**

**Toggle:** `ENABLE_ADVANCED_COMMANDS`

Remote commands: STATUS, RESET_STATS, SET_POWER, SET_SF, etc.

#### `encryption.h`
**Simple XOR encryption**

**Toggle:** `ENABLE_ENCRYPTION`

Basic payload obfuscation (not cryptographically secure).

#### `packet_stats.h`
**Detailed packet statistics**

**Toggle:** `ENABLE_PACKET_STATS`

Tracks retries, duplicates, out-of-order packets.

#### `performance_monitor.h`
**CPU and memory tracking**

**Toggle:** `ENABLE_PERFORMANCE_MONITOR`

Reports loop frequency, CPU usage, memory consumption.

#### `runtime_config.h`
**Serial configuration commands**

**Toggle:** `ENABLE_RUNTIME_CONFIG`

Change settings on-the-fly via serial commands.

#### `watchdog_timer.h`
**Hardware watchdog**

**Toggle:** `ENABLE_WATCHDOG`

Automatic reboot if system hangs (10s timeout).

---

## 📖 Documentation Files

### `README.md`
Quick start guide and project overview. Read this first!

### `PROJECT_STRUCTURE.md` (This file)
Complete guide to file organization and architecture.

### `TESTING_CHECKLIST.md`
**Step-by-step testing guide** with checklists for every feature.

### `COMPREHENSIVE_MANUAL.md`
Complete technical manual with troubleshooting, setup instructions, and advanced features.

### `FEATURE_TESTING_GUIDE.md`
How to test each individual feature module.

### `PC_LOGGING_README.md`
Python data logging setup and usage guide.

---

## 🐍 Python Scripts

All Python scripts are for PC-side data logging and analysis.

### `serial_monitor.py`
Real-time colored serial monitor with RSSI bars.

```bash
python serial_monitor.py /dev/ttyUSB0 115200
```

### `data_logger.py`
Logs data to SQLite database for analysis.

```bash
python data_logger.py /dev/ttyUSB0 115200 lora_data.db
```

### `realtime_plotter.py`
Live graphs of RSSI, SNR, packet loss, connection state.

### `analyze_data.py`
Statistical analysis and visualization of logged data.

### `lora_analysis.ipynb`
Jupyter notebook for interactive data exploration.

---

## 🔌 Hardware Connections

### Required Hardware

**Main ESP32 (Sender OR Receiver):**
- RYLR896 LoRa module
- I2C LCD 16x2 (receiver only, optional)
- LED on GPIO 2 (built-in)
- Touch sensor on T0/GPIO 4

**Display ESP32 (Optional):**
- ESP32-2432S022 TFT display
- Connection: Main TX → Display RX (only 2 wires + GND!)

### Pin Assignments

```
RYLR896 LoRa Module:
  TX  → ESP32 GPIO 25 (RX2)
  RX  → ESP32 GPIO 26 (TX2)
  VCC → 3.3V
  GND → GND

Role Detection:
  GPIO 16 ↔ GPIO 17 (jumper) = RECEIVER
  GPIO 16 floating           = SENDER

Kill-Switch:
  GPIO 13 ↔ GPIO 14 (hold 3s) = Restart

Display Station:
  Main GPIO 23 (TX) → Display GPIO 18 (RX)
  GND              → GND

I2C LCD (Receiver only):
  SDA → GPIO 21
  SCL → GPIO 22
  Address: 0x27

Optional Sensors:
  Battery   → GPIO 35 (ADC1_CH7)
  Audio     → GPIO 34 (ADC1_CH6)
  TCS34725  → I2C (same bus as LCD)
  INA219    → I2C (same bus as LCD)
```

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Roboter Gruppe 9 System                  │
└─────────────────────────────────────────────────────────────┘

┌─────────────────┐                           ┌─────────────────┐
│   SENDER        │      LoRa 868 MHz         │   RECEIVER      │
│   ESP32         │◄─────────────────────────►│   ESP32         │
│                 │                           │                 │
│ • Touch sensor  │   Data + ACK (SF12)       │ • I2C LCD       │
│ • LED           │                           │ • Touch sensor  │
│ • Health mon.   │                           │ • LED           │
│                 │                           │ • Health mon.   │
└────────┬────────┘                           └────────┬────────┘
         │                                             │
         │ UART (USB)                                  │ UART (USB)
         │ Debug + Data                                │ Debug + Data
         ▼                                             ▼
┌─────────────────┐                           ┌─────────────────┐
│  PC (Python)    │                           │  PC (Python)    │
│ • Serial mon.   │                           │ • Serial mon.   │
│ • Data logger   │                           │ • Data logger   │
│ • Analysis      │                           │ • Analysis      │
└─────────────────┘                           └─────────────────┘
         │
         │ UART2 (GPIO 23)
         │ Display data
         ▼
┌─────────────────┐
│  Display ESP32  │
│  (2432S022 TFT) │
│                 │
│ • 320x240 LCD   │
│ • Real-time     │
│ • Status & data │
└─────────────────┘
```

---

## 🔄 Data Flow

### Sender → Receiver (Every 2s)

1. **Sender** reads sensors (LED, touch)
2. **Sender** packages data: `SEQ:42,LED:1,TOUCH:0,SPIN:2`
3. **Sender** sends via LoRa to receiver address
4. **Receiver** receives message with RSSI/SNR
5. **Receiver** parses payload, updates LCD
6. **Receiver** tracks sequence for packet loss

### Receiver → Sender (Every 5th message, if enabled)

7. **Receiver** sends ACK: `ACK,SEQ:5,LED:0,TOUCH:1`
8. **Sender** receives ACK within 500ms
9. **Sender** updates statistics (ACK count, RSSI)

### Both → PC (Every 2s, if enabled)

10. **ESP32** outputs CSV via USB Serial
11. **Python** script captures and logs to database
12. **Analysis** scripts process data (graphs, statistics)

### Main → Display Station (Every 2s, if enabled)

13. **Main ESP32** sends status via UART2 (GPIO 23)
14. **Display ESP32** receives on GPIO 18
15. **Display** shows data on TFT screen

---

## 🎛️ Configuration Guide

### How to Enable/Disable Features

**Edit `config.h`:**

```cpp
// Display station output
#define ENABLE_DISPLAY_OUTPUT true   // Set false to disable

// Bi-directional communication
#define ENABLE_BIDIRECTIONAL true    // Set false for one-way only

// PC data logging
#define ENABLE_CSV_OUTPUT true       // CSV format
#define ENABLE_JSON_OUTPUT false     // JSON format (alternative)

// Sensor features
#define ENABLE_BATTERY_MONITOR false  // Set true to enable
#define ENABLE_AUDIO_DETECTION false
#define ENABLE_LIGHT_DETECTION false
#define ENABLE_CURRENT_MONITOR false

// System features
#define ENABLE_EXTENDED_TELEMETRY false
#define ENABLE_ADAPTIVE_SF false
#define ENABLE_ENCRYPTION false
#define ENABLE_PACKET_STATS false
#define ENABLE_PERFORMANCE_MONITOR false
#define ENABLE_RUNTIME_CONFIG false
#define ENABLE_WATCHDOG false
```

### Dependencies

**No external dependencies!**

Only standard libraries are used:
- `Arduino.h` - Arduino core
- `LiquidCrystal_I2C.h` - For LCD display (receiver only)
- `WiFi.h` - For WiFi features (if enabled)
- `esp_system.h` - For restart functionality

**Optional libraries for features:**
- `Adafruit_TCS34725.h` - For light detection
- `Adafruit_INA219.h` - For current monitoring

---

## 💾 Memory Usage

**Flash (Program):** ~250 KB
- Core system: ~150 KB
- Each feature: ~5-15 KB

**RAM (Runtime):** ~45 KB
- Core system: ~30 KB
- Feature modules: ~1-5 KB each

**Optimization tips:**
- Disable unused features in `config.h`
- Use `const` for read-only data
- Minimize String usage (prefer char arrays)

---

## 🚀 Build and Upload

### Arduino IDE

1. Open `Roboter_Gruppe_9.ino`
2. Select board: `ESP32 Dev Module`
3. Select port
4. Upload

All `.h` files are automatically included!

### PlatformIO

```bash
pio run -t upload
pio device monitor -b 115200
```

### Compilation Time

- Full build: ~30 seconds
- Incremental: ~5 seconds

---

## 📊 Code Statistics

```
Component              Lines   %     Purpose
──────────────────────────────────────────────────────
Roboter_Gruppe_9.ino    789   31%   Main program
lora_handler.h          264   10%   LoRa communication
health_monitor.h        310   12%   Connection monitoring
display_sender.h        243   10%   Display output
DisplayClient.h         194    8%   Display library
structs.h               111    4%   Data structures
functions.h             145    6%   Helper functions
config.h                182    7%   Configuration
Feature modules         300+  12%   Optional features
──────────────────────────────────────────────────────
Total                 ~2500  100%   C++ code
```

---

## 🎓 Learning Path

**If you're new to the project, read in this order:**

1. **README.md** - Project overview
2. **PROJECT_STRUCTURE.md** (this file) - Architecture
3. **config.h** - See what can be configured
4. **TESTING_CHECKLIST.md** - Test basic functionality
5. **Roboter_Gruppe_9.ino** - Read main program
6. **lora_handler.h** - Understand LoRa communication
7. **COMPREHENSIVE_MANUAL.md** - Deep dive into details

---

## 🏆 Best Practices

### Adding New Features

1. Create new `.h` file in project root
2. Add feature toggle in `config.h`
3. Wrap all code in `#if ENABLE_YOUR_FEATURE`
4. Include in `Roboter_Gruppe_9.ino`
5. Update documentation
6. Test with feature ON and OFF

### Modifying Existing Code

1. Test with all features disabled first
2. Enable one feature at a time
3. Verify no compilation errors
4. Test functionality
5. Document changes

### Code Style

- Use clear, descriptive variable names
- Add comments for complex logic
- Keep functions small and focused
- Use `const` for constants
- Avoid global variables (use structs instead)

---

## 🐛 Troubleshooting

**Compilation errors:**
- Check all feature toggles in `config.h`
- Ensure all required libraries are installed
- Verify no missing `.h` files

**Runtime issues:**
- Enable serial debug output
- Check pin connections
- Verify LoRa module is powered
- Test with minimal configuration first

**Feature not working:**
- Check if feature toggle is `true` in `config.h`
- Verify hardware is connected correctly
- Read feature-specific documentation
- Test feature in isolation

---

**Next:** Read `TESTING_CHECKLIST.md` to start testing!
