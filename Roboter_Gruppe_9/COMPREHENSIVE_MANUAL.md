# Roboter Gruppe 9 - Comprehensive Manual

> **Complete guide to building, deploying, and troubleshooting your LoRa communication system**
>
> 📖 Reading time: ~20 minutes for overview, use as reference guide
>
> *Last updated: November 2025*

---

## 📑 Table of Contents

1. [Introduction](#introduction)
2. [Quick Start](#quick-start)
3. [Hardware Setup](#hardware-setup)
4. [Software Installation](#software-installation)
5. [Features & Capabilities](#features--capabilities)
6. [LCD Display Versions](#lcd-display-versions)
7. [Connection Watchdog](#connection-watchdog)
8. [Kill-Switch Usage](#kill-switch-usage)
9. [PC Data Logging](#pc-data-logging)
10. [Troubleshooting](#troubleshooting)
11. [Configuration Guide](#configuration-guide)
12. [Architecture & Technical Details](#architecture--technical-details)
13. [Development & Customization](#development--customization)
14. [FAQ](#faq)
15. [Reference](#reference)

---

## Introduction

### What is Roboter Gruppe 9?

Roboter Gruppe 9 is a **professional-grade wireless communication system** built on ESP32 microcontrollers and RYLR896 LoRa modules. It provides:

- **Long-range communication** (up to 5+ km line of sight)
- **Automatic role detection** (no code changes needed)
- **Bi-directional data flow** with acknowledgment support
- **Self-healing connection** with automatic recovery
- **Real-time health monitoring** and packet loss tracking
- **Comprehensive data logging** to PC via USB
- **Safety features** including physical and remote kill-switch

### System Overview

```
┌─────────────────────────────────────────────────────────┐
│                  System Architecture                     │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  ┌──────────────┐       LoRa 868MHz      ┌────────────┐│
│  │   SENDER     │◄─────────────────────►│  RECEIVER  ││
│  │   ESP32      │    Bi-directional     │   ESP32    ││
│  │              │                        │            ││
│  │ • Touch      │  Data + ACK            │ • LCD      ││
│  │ • LED        │  Auto-recovery         │ • Touch    ││
│  │ • Health     │  Packet tracking       │ • LED      ││
│  └──────┬───────┘                        └──────┬─────┘│
│         │                                       │      │
│         │ USB                                   │ USB  │
│         ▼                                       ▼      │
│  ┌──────────────┐                     ┌──────────────┐│
│  │  Serial      │                     │ Data Logger  ││
│  │  Monitor     │                     │ (SQLite)     ││
│  │  (Python)    │                     │              ││
│  └──────────────┘                     └──────────────┘│
└─────────────────────────────────────────────────────────┘
```

### Key Features

**Hardware:**
- ✅ ESP32 microcontroller (any variant)
- ✅ RYLR896 LoRa transceiver (868 MHz)
- ✅ 16x2 I2C LCD display (receiver only, optional)
- ✅ Touch sensor support
- ✅ Built-in LED indicators

**Communication:**
- ✅ Bi-directional LoRa communication
- ✅ Automatic acknowledgments (ACK) every 5 messages
- ✅ Packet loss detection with sequence numbers
- ✅ RSSI/SNR signal quality monitoring
- ✅ Range: 5+ km line of sight with SF12

**Intelligence:**
- ✅ Automatic role detection (GPIO jumper)
- ✅ Connection state machine (UNKNOWN/CONNECTED/WEAK/LOST)
- ✅ Automatic recovery on connection loss (3 attempts)
- ✅ Health monitoring with statistics
- ✅ Self-healing architecture

**Safety:**
- ✅ Physical kill-switch (GPIO 13↔14, hold 3s)
- ✅ Remote kill-switch via LoRa (CMD:RESTART)
- ✅ Works even if LoRa module disconnected

**Data Logging:**
- ✅ CSV/JSON serial output (2s interval)
- ✅ Python real-time monitor with colors
- ✅ SQLite database storage
- ✅ Configurable logging intervals

---

## Quick Start

### 5-Minute Setup

**You'll need:**
- 2× ESP32 boards
- 2× RYLR896 LoRa modules
- 1× 16x2 I2C LCD (optional, for receiver)
- Jumper wires
- 2× USB cables

**Steps:**

1. **Connect Hardware**
   ```
   RYLR896 → ESP32
   ─────────────────
   TX  → GPIO 25
   RX  → GPIO 26
   VCC → 3.3V
   GND → GND
   ```

2. **Upload Code**
   - Open `Roboter_Gruppe_9.ino` in Arduino IDE
   - Select board: "ESP32 Dev Module"
   - Upload to **both** devices (identical code!)

3. **Configure Roles**
   - **Receiver:** Connect GPIO 16 to GPIO 17 with jumper wire
   - **Sender:** Leave GPIO 16 floating (no connection)

4. **Power Up**
   - Connect both devices to power
   - Open Serial Monitor (115200 baud) on both

5. **Verify**
   - Sender should show: "Messages TX: X"
   - Receiver should show: "Messages RX: X"
   - LCD (if connected) should display signal bars

**Done!** Messages should be flowing. See [Troubleshooting](#troubleshooting) if issues.

---

## Hardware Setup

### Complete Wiring Diagram

#### RYLR896 LoRa Module
```
RYLR896 Pin    ESP32 Pin      Purpose
─────────────────────────────────────
TX             GPIO 25        Module → ESP32 data
RX             GPIO 26        ESP32 → Module data
VCC            3.3V           Power (3.3V only!)
GND            GND            Ground
```

⚠️ **Warning:** RYLR896 is 3.3V only! Do NOT connect to 5V.

#### Role Detection (Both Devices)
```
ESP32 Pin      Connection     Purpose
─────────────────────────────────────
GPIO 17        OUTPUT LOW     Provides GND
GPIO 16        INPUT_PULLUP   Role detection

Receiver: GPIO 16 ↔ GPIO 17 (jumper wire)
Sender:   GPIO 16 floating (no connection)
```

💡 **Tip:** GPIO 16 and GPIO 17 are physically next to each other on most ESP32 boards.

#### Kill-Switch (Both Devices)
```
ESP32 Pin      Connection     Purpose
─────────────────────────────────────
GPIO 14        OUTPUT LOW     Provides GND
GPIO 13        INPUT_PULLUP   Kill-switch input

To restart: Connect GPIO 13 ↔ GPIO 14 and hold 3 seconds
```

#### LCD Display (Receiver Only)
```
LCD Pin    ESP32 Pin    Purpose
─────────────────────────────────
VCC        5V           Power
GND        GND          Ground
SDA        GPIO 21      I2C data
SCL        GPIO 22      I2C clock
```

Default I2C address: **0x27** (configurable in code if different)

#### Built-in Peripherals
```
Component      ESP32 Pin    Purpose
─────────────────────────────────────
LED            GPIO 2       Status indicator
Touch Sensor   GPIO 4       Touch input (T0)
```

### Hardware Checklist

Before powering up:

- [ ] RYLR896 connected to 3.3V (not 5V!)
- [ ] TX/RX wires not swapped
- [ ] Role jumper correct (GPIO 16↔17 for receiver)
- [ ] LCD I2C address matches code (0x27)
- [ ] Good USB cable (some cables are charge-only)
- [ ] Antenna connected to LoRa module

---

## Software Installation

### Prerequisites

**Arduino IDE:**
- Arduino IDE 1.8.19 or newer
- OR PlatformIO Core 6.0+ (recommended)

**ESP32 Board Support:**
```
1. Open Arduino IDE
2. File → Preferences
3. Additional Board Manager URLs:
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
4. Tools → Board → Boards Manager
5. Search "ESP32" and install "esp32 by Espressif Systems"
```

**Required Libraries:**
```
1. Sketch → Include Library → Manage Libraries
2. Install:
   - LiquidCrystal_I2C (version 1.1.2 or newer)
   - WiFi (included with ESP32 core)
```

### Installation Steps

#### Method 1: Arduino IDE

1. **Download Code**
   ```bash
   git clone <repository-url>
   cd Roboter_Gruppe_9
   ```

2. **Open Project**
   - Open `Roboter_Gruppe_9.ino` in Arduino IDE

3. **Configure Board**
   ```
   Tools → Board → ESP32 Arduino → ESP32 Dev Module
   Tools → Upload Speed → 115200
   Tools → Flash Frequency → 80MHz
   Tools → Partition Scheme → Default 4MB
   ```

4. **Upload**
   - Connect ESP32 via USB
   - Tools → Port → (select your ESP32 port)
   - Click Upload button
   - Repeat for second device

#### Method 2: PlatformIO

1. **Open Project**
   ```bash
   cd Roboter_Gruppe_9
   pio run -t upload
   ```

2. **Monitor**
   ```bash
   pio device monitor -b 115200
   ```

### Verification

After upload, open Serial Monitor (115200 baud):

**Expected output:**
```
╔════════════════════════════╗
║  ZignalMeister 2000        ║
╚════════════════════════════╝
✓ Kill-switch initialized: GPIO13↔14, hold 3s to restart
  GPIO14 (GND): LOW ✓
  GPIO13 (READ): HIGH ✓ - not pressed

>>> SENDER MODE  (or RECEIVER MODE)

✓ RYLR896 Ready!
✓ Setup complete!
```

If you see this, installation successful! 🎉

---

## Features & Capabilities

### Automatic Role Detection

**How it works:**
- GPIO 17 set as OUTPUT LOW (provides GND)
- GPIO 16 set as INPUT_PULLUP (reads HIGH by default)
- If GPIO 16 connected to GPIO 17 → reads LOW → **RECEIVER mode**
- If GPIO 16 floating → reads HIGH → **SENDER mode**

**Benefits:**
- No code changes between devices
- Easy to swap roles (just move jumper)
- Impossible to misconfigure

**Debug output:**
```
╔════════════════════════════╗
║   MODE DETECTION DEBUG     ║
║ GPIO 16: LOW               ║
║ → RECEIVER MODE            ║
╚════════════════════════════╝
```

### Bi-directional Communication

**Message Flow:**

1. **Sender → Receiver** (every 2 seconds)
   ```
   Payload: SEQ:42,LED:1,TOUCH:0,SPIN:2,COUNT:42
   ```

2. **Receiver processes** message and updates display

3. **Receiver → Sender** (every 5th message)
   ```
   ACK: ACK,SEQ:5,LED:0,TOUCH:1,SPIN:3
   ```

4. **Sender receives ACK** and updates statistics

**Configuration:**
```cpp
// config.h
#define ENABLE_BIDIRECTIONAL true   // Enable two-way
#define ACK_INTERVAL 5              // ACK every N messages
#define LISTEN_TIMEOUT 500          // Wait 500ms for ACK
```

**Statistics tracked:**
- Sender: ACKs received, last ACK time, RSSI of ACK
- Receiver: Messages sent, ACK success rate

### Health Monitoring

**Connection States:**
```
UNKNOWN     → Initial state, no data yet
CONNECTING  → First message received, establishing
CONNECTED   → Receiving regularly, signal good
WEAK        → Intermittent or weak signal
LOST        → No messages for 8+ seconds
```

**State Transitions:**
```
Time since last message    RSSI          State
────────────────────────────────────────────────
< 3 seconds               > -100 dBm    CONNECTED
3-8 seconds or RSSI weak  < -100 dBm    WEAK
> 8 seconds               any           LOST
```

**Recovery Process:**
```
1. State changes to LOST
2. Wait 15 seconds (cooldown)
3. Attempt recovery (re-init LoRa)
4. Success → back to CONNECTING
5. Fail → try again (max 3 attempts)
6. After 3 fails → manual intervention needed
```

**Monitored Metrics:**
- RSSI (min, max, average)
- SNR values
- Packet loss percentage
- Duplicate packets
- Uptime
- Connection time

**Example Output:**
```
╔═══════════════════════════════════════╗
║        HEALTH MONITOR REPORT         ║
╠═══════════════════════════════════════╣
║ Status:     CONNECTED *               ║
║ Uptime:     3600 s                    ║
║ Connected:  3540 s                    ║
╠═══════════════════════════════════════╣
║ RSSI Avg:   -67 dBm                   ║
║ RSSI Min:   -89 dBm                   ║
║ RSSI Max:   -52 dBm                   ║
╠═══════════════════════════════════════╣
║ Packets RX: 1800                      ║
║ Lost:       12 (0.7%)                 ║
╚═══════════════════════════════════════╝
```

### Packet Loss Tracking

**How it works:**
- Each message includes sequence number (SEQ:X)
- Receiver expects sequential numbers (1, 2, 3, 4...)
- Gap detected → packet(s) lost
- Backwards → duplicate packet

**Example:**
```
Received: SEQ:10  Expected: 10  ✓ OK
Received: SEQ:11  Expected: 11  ✓ OK
Received: SEQ:14  Expected: 12  ⚠ Lost 2 packets (12, 13)
Received: SEQ:15  Expected: 15  ✓ OK
Received: SEQ:14  Expected: 16  ⚠ Duplicate
```

**Statistics:**
- Packets received
- Packets lost
- Packets duplicate
- Loss percentage = Lost / (Received + Lost) × 100

---

## LCD Display Versions

The receiver can use 4 different LCD layouts. Configure in `Roboter_Gruppe_9.ino`:

### Version 1: Wide Visual Bar ⭐ (Recommended)

**Features:**
- Signal strength bar graph (10 characters wide)
- Connection state icon
- Message count
- Remote spinner
- RSSI value in dBm
- Local LED and Touch state
- Fast local spinner

**Display:**
```
Line 1: *[████████░░]42 >
Line 2: -67dB L:1 T:0   <
```

**Symbols:**
- `*` = Connected, `!` = Weak, `X` = Lost
- `█` = Signal strength (filled)
- `░` = Signal strength (empty)
- `>` = Remote spinner (slow)
- `<` = Local spinner (fast)

**Enable:**
```cpp
void updateLCD() {
  // ...
  updateLCD_Version1_WideBar();  // Uncomment this
}
```

### Version 2: Compact

**Features:**
- Compact signal bar (7 chars)
- RSSI and SNR values
- LED states for both local and remote
- Message count

**Display:**
```
Line 1: *[█████░░]-65  >
Line 2: S:8 L:1 R:0 42 <
```

**Enable:**
```cpp
updateLCD_Version2_Compact();
```

### Version 3: Detailed Info

**Features:**
- Message count first
- RSSI with signal quality icon
- SNR on second line
- Both LED states

**Display:**
```
Line 1: RX:142 -67dB =
Line 2: SNR:9 L:1 R:0  <
```

**Signal Icons:**
- `^` = Excellent (> -50 dBm)
- `=` = Good (> -80 dBm)
- `-` = Fair (> -100 dBm)
- `v` = Poor (> -110 dBm)
- `X` = Critical (< -110 dBm)

**Enable:**
```cpp
updateLCD_Version3_Detailed();
```

### Version 4: Original (Simple)

**Features:**
- Basic remote status
- Local status
- No signal quality
- Minimal information

**Display:**
```
Line 1: REM:1 T:0     >
Line 2: LOC:1 T:0     <
```

**Enable:**
```cpp
updateLCD_Version4_Original();
```

### Choosing a Version

| Version | Best For | Information Density |
|---------|----------|-------------------|
| 1 (Wide Bar) | General use, monitoring signal | High |
| 2 (Compact) | Advanced users, all metrics | Very High |
| 3 (Detailed) | Debugging, signal quality focus | Medium |
| 4 (Original) | Beginners, simple status | Low |

💡 **Recommendation:** Start with Version 1 (Wide Bar) - it's the most intuitive.

---

## Connection Watchdog

### Overview

The connection watchdog monitors link quality and automatically recovers from failures.

### State Machine

```
     ┌──────────┐
     │ UNKNOWN  │ ← Initial state
     └────┬─────┘
          │ First message
          ▼
     ┌──────────┐
     │CONNECTING│
     └────┬─────┘
          │ Regular messages, good RSSI
          ▼
     ┌──────────┐
     │CONNECTED │ ◄──┐
     └────┬─────┘    │
          │          │ Recovery success
          │ 3-8s timeout OR weak RSSI
          ▼          │
     ┌──────────┐    │
     │  WEAK    │    │
     └────┬─────┘    │
          │          │
          │ > 8s timeout
          ▼          │
     ┌──────────┐    │
     │  LOST    │ ───┘
     └──────────┘
       Auto-recovery
```

### Configuration

**Thresholds (editable in `health_monitor.h`):**
```cpp
WatchdogConfig watchdogCfg = {
  .weakTimeout = 3000,           // 3s → WEAK
  .lostTimeout = 8000,           // 8s → LOST
  .weakRssiThreshold = -100,     // -100 dBm → WEAK
  .criticalRssiThreshold = -110, // -110 dBm → CRITICAL
  .recoveryInterval = 15000,     // Try recovery every 15s
  .maxRecoveryAttempts = 3       // Give up after 3 tries
};
```

**Tuning guide:**
- **Short range (<100m):** Decrease timeouts (2s/5s)
- **Long range (>1km):** Increase timeouts (5s/15s)
- **Moving devices:** Increase recovery attempts (5+)
- **Stationary:** Default settings fine

### Recovery Process

**Automatic recovery when LOST:**

1. Wait 15 seconds (cooldown)
2. Print recovery message:
   ```
   ╔════════════════════════════════════╗
   ║ RECOVERY ATTEMPT #1                ║
   ║ Re-initializing LoRa module...     ║
   ╚════════════════════════════════════╝
   ```
3. Call `initLoRa()` to reset module
4. If successful → state = CONNECTING
5. If failed → wait 15s and retry
6. After 3 attempts → stop trying

**Manual recovery:**
- Use kill-switch (GPIO 13↔14, hold 3s)
- Send remote restart: `CMD:RESTART`
- Power cycle device

### Monitoring

**Serial Monitor output:**
```
╔════════ CONNECTION STATE CHANGE ════════╗
║ CONNECTED -> WEAK
║ Time since last message: 4.2 s
║ RSSI: -105 dBm
╚═══════════════════════════════════════╝
```

**Health report (every 30 seconds):**
```
╔═══════════════════════════════════════╗
║        HEALTH MONITOR REPORT         ║
║ Status:     WEAK !                    ║
║ Uptime:     1800 s                    ║
║ RSSI Avg:   -92 dBm                   ║
║ Packets RX: 900                       ║
║ Lost:       15 (1.6%)                 ║
╚═══════════════════════════════════════╝
```

---

## Kill-Switch Usage

### Physical Kill-Switch

**Hardware:**
- GPIO 14 = GND (always LOW)
- GPIO 13 = INPUT_PULLUP (normally HIGH)
- Connect GPIO 13 to GPIO 14 = reads LOW = pressed

**Usage:**
1. Connect GPIO 13 to GPIO 14 with jumper wire or button
2. Hold for 3 seconds
3. Watch countdown in Serial Monitor
4. Device restarts automatically

**Serial output:**
```
🔴 Kill-switch PRESSED - hold to restart...
🔴 2 more seconds...
🔴 1 more second...

╔════════════════════════════════════╗
║  🔴 RESTART: Physical kill-switch  ║
╚════════════════════════════════════╝

[Device reboots]
```

**Use cases:**
- Emergency stop during testing
- Quick restart without removing power
- Recovery from frozen state
- Safety feature for robotics

### Remote Kill-Switch

**Send via LoRa:**
```cpp
// From sender to receiver (or vice versa with bi-directional)
sendLoRaMessage("CMD:RESTART", TARGET_ADDRESS);
```

**Commands:**
- `CMD:RESTART` - Restart device immediately
- `CMD:STOP` - Reserved for future use

**Response:**
```
⚠️  REMOTE RESTART COMMAND RECEIVED!

╔════════════════════════════════════╗
║  🔴 RESTART: Remote command        ║
╚════════════════════════════════════╝

[Device reboots]
```

**Security note:** Anyone on same LoRa network can send restart command. Future versions may add authentication.

### Debug Mode

**Enable continuous status output:**
```cpp
#define KILLSWITCH_DEBUG true
```

**Output (every 2 seconds):**
```
[KillSwitch Debug] GPIO13: 1 (released)
[KillSwitch Debug] GPIO13: 1 (released)
[KillSwitch Debug] GPIO13: 0 (PRESSED)
```

**Disable for production:**
```cpp
#define KILLSWITCH_DEBUG false
```

---

## PC Data Logging

### Overview

Connect ESP32 to your computer via USB and log all communication data in real-time.

**Features:**
- CSV/JSON data export
- Real-time colored terminal monitor
- SQLite database storage
- Configurable intervals

**See `PC_LOGGING_README.md` for complete documentation.**

### Quick Setup

1. **Enable logging in config.h:**
   ```cpp
   #define ENABLE_CSV_OUTPUT true
   #define DATA_OUTPUT_INTERVAL 2000  // 2 seconds
   ```

2. **Install Python dependencies:**
   ```bash
   pip install pyserial
   ```

3. **Run real-time monitor:**
   ```bash
   python serial_monitor.py /dev/ttyUSB0 115200
   ```

4. **Or run database logger:**
   ```bash
   python data_logger.py /dev/ttyUSB0 115200 lora_data.db
   ```

### Data Format

**CSV output (every 2 seconds):**
```
DATA_CSV,TIMESTAMP,ROLE,RSSI,SNR,SEQ,MSG_COUNT,CONN_STATE,PACKET_LOSS,LED,TOUCH
DATA_CSV,45632,RX,-67,9,142,142,OK,0.00,1,0
```

**Fields:**
- `TIMESTAMP` - ESP32 millis() since boot
- `ROLE` - RX (receiver) or TX (sender)
- `RSSI` - Signal strength in dBm
- `SNR` - Signal-to-noise ratio in dB
- `SEQ` - Sequence number
- `MSG_COUNT` - Total messages sent/received
- `CONN_STATE` - OK/WEAK/LOST/UNKNOWN
- `PACKET_LOSS` - Percentage (0.00-100.00)
- `LED` - LED state (0 or 1)
- `TOUCH` - Touch sensor (0 or 1)

### Python Scripts

**serial_monitor.py** - Real-time viewer:
- Color-coded output (green=OK, yellow=WEAK, red=LOST)
- RSSI quality bars
- Live updates
- Parses both CSV and raw messages

**data_logger.py** - Database logger:
- Creates SQLite database automatically
- Stores all data with PC timestamps
- Event logging (errors, state changes)
- Indexed for fast queries

**Example usage:**
```bash
# Real-time monitoring
python serial_monitor.py /dev/ttyUSB0 115200

# Long-term logging
python data_logger.py /dev/ttyUSB0 115200 experiment1.db

# Analyze data later
sqlite3 experiment1.db
> SELECT AVG(rssi), AVG(packet_loss) FROM lora_messages;
```

---

## Troubleshooting

### Common Issues

#### 1. "❌ No response from module!"

**Symptom:** LoRa initialization fails, no AT command response

**Causes & Solutions:**

✅ **Check wiring:**
```
RYLR896 TX → ESP32 GPIO 25 (not 26!)
RYLR896 RX → ESP32 GPIO 26 (not 25!)
```

✅ **Check voltage:**
- RYLR896 requires 3.3V (NOT 5V!)
- Measure voltage at module: should be 3.2-3.4V

✅ **Check baudrate:**
```cpp
#define LORA_BAUDRATE 115200  // In config.h
```

✅ **Test module separately:**
```cpp
// Simple test (upload this first)
void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 25, 26);
  Serial2.println("AT");
}
void loop() {
  if (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}
// Should print: +OK
```

✅ **Check antenna:**
- Module won't respond properly without antenna
- Connect antenna before power-up

#### 2. "LCD stays blank"

**Symptom:** LCD backlight on but no text

**Causes & Solutions:**

✅ **Check I2C address:**
```cpp
// Try scanning for I2C devices
#include <Wire.h>
void setup() {
  Wire.begin();
  Serial.begin(115200);
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C device at 0x");
      Serial.println(addr, HEX);
    }
  }
}
// Common addresses: 0x27, 0x3F
```

✅ **Change address in code:**
```cpp
// functions.h
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Try 0x3F if 0x27 doesn't work
```

✅ **Check wiring:**
```
LCD SDA → ESP32 GPIO 21
LCD SCL → ESP32 GPIO 22
LCD VCC → 5V (LCD needs 5V, not 3.3V!)
LCD GND → GND
```

✅ **Check contrast:**
- LCD may have contrast potentiometer on back
- Turn it slowly while powered to see if text appears

#### 3. "No ACK received" (Sender)

**Symptom:** Sender shows `ACKs RX: 0`, never receives acknowledgments

**Causes & Solutions:**

✅ **Check bi-directional is enabled:**
```cpp
// config.h
#define ENABLE_BIDIRECTIONAL true
```

✅ **Verify sender is listening:**
- After sending, sender should wait 500ms
- Check Serial Monitor for "Listening for response..."

✅ **Check receiver is sending:**
- Receiver Serial Monitor should show "📤 Sending ACK" every 5 messages

✅ **Increase listen timeout:**
```cpp
// config.h
#define LISTEN_TIMEOUT 1000  // Increase from 500 to 1000ms
```

✅ **Check ACK interval:**
```cpp
// config.h
#define ACK_INTERVAL 3  // Decrease from 5 to 3 for more frequent ACKs
```

✅ **Test range:**
- Bring devices closer (< 10m)
- ACKs are affected by distance/obstacles

#### 4. "Kill-switch doesn't work"

**Symptom:** Connecting GPIO 13 to GPIO 14 does nothing

**Causes & Solutions:**

✅ **Verify pins:**
```
GPIO 13 (not 12!) → Read pin
GPIO 14 → GND pin
```

✅ **Check debug output:**
```cpp
#define KILLSWITCH_DEBUG true  // Enable in code
```
Should print every 2 seconds:
```
[KillSwitch Debug] GPIO13: 1 (released)
```

✅ **Hold long enough:**
- Must hold for full 3 seconds
- Watch for countdown messages

✅ **Check if loop() is running:**
- If loop() frozen, kill-switch won't work
- Check for infinite loops in your modifications

#### 5. "Sender and Receiver both think they're sender"

**Symptom:** Both devices show ">>> SENDER MODE"

**Causes & Solutions:**

✅ **Check role jumper:**
- Receiver must have GPIO 16 connected to GPIO 17
- Pins are next to each other on most ESP32 boards

✅ **Verify jumper connection:**
- Use multimeter: continuity test between GPIO 16 and GPIO 17
- Or check debug output:
```
║ MODE_SELECT_PIN (16): LOW  ║  ← Should be LOW for receiver
```

✅ **Try different pins:**
- GPIO 16/17 might be damaged
- Can modify code to use different pins

#### 6. "High packet loss (>10%)"

**Symptom:** Health report shows >10% packet loss

**Causes & Solutions:**

✅ **Check RSSI:**
```
RSSI > -80 dBm   = Excellent (< 1% loss expected)
RSSI -80 to -100 = Good (1-5% loss)
RSSI < -100 dBm  = Poor (>5% loss)
```

✅ **Reduce distance:**
- Move devices closer
- Test with line-of-sight

✅ **Remove obstacles:**
- Walls, metal, water absorb LoRa signals
- Go outside for testing

✅ **Check antenna:**
- Antenna damaged?
- Antenna fully connected?
- Try different antenna orientation

✅ **Increase power:**
```cpp
// In lora_handler.h, add after initialization:
sendLoRaCommand("AT+CRFOP=15");  // Max power (15 dBm)
```

✅ **Reduce send rate:**
```cpp
// Roboter_Gruppe_9.ino, sender section
if (millis() - timing.lastSend >= 5000) {  // Change 2000 to 5000
```

### Debug Checklist

When nothing works, go through this:

**Hardware:**
- [ ] All connections tight and correct
- [ ] RYLR896 powered at 3.3V
- [ ] Antenna connected
- [ ] No loose wires
- [ ] USB cable good (not charge-only)

**Software:**
- [ ] Correct board selected (ESP32 Dev Module)
- [ ] Upload successful (100% complete)
- [ ] Serial Monitor at 115200 baud
- [ ] Libraries installed (LiquidCrystal_I2C)

**Configuration:**
- [ ] Role jumper correct
- [ ] Network ID matches (default: 6)
- [ ] Addresses correct (Sender=2, Receiver=1)

**Testing:**
- [ ] Test each device separately
- [ ] Start with devices close (<1m)
- [ ] Check Serial Monitor output
- [ ] Verify LoRa modules respond to AT

### Getting Help

If still stuck:

1. **Collect information:**
   - Serial Monitor output from both devices
   - Photos of wiring
   - Code modifications made

2. **Check documentation:**
   - This manual
   - RYLR896 datasheet
   - ESP32 pinout diagram

3. **Simplify:**
   - Upload minimal code (just LoRa test)
   - Test with known-good hardware
   - Eliminate variables one by one

---

## Configuration Guide

### config.h Settings

**Pin Definitions:**
```cpp
#define LED_PIN 2           // Built-in LED
#define RXD2 25            // LoRa RX (module TX)
#define TXD2 26            // LoRa TX (module RX)
#define TOUCH_PIN T0       // Touch sensor (GPIO 4)
#define MODE_SELECT_PIN 16  // Role detection
#define MODE_GND_PIN 17    // Role GND
```

**LoRa Configuration:**
```cpp
#define LORA_RECEIVER_ADDRESS 1  // Receiver ID
#define LORA_SENDER_ADDRESS 2    // Sender ID
#define LORA_NETWORK_ID 6        // Must match on both!
#define LORA_BAUDRATE 115200     // RYLR896 baudrate
```

💡 **Change Network ID** to avoid interference with other LoRa devices nearby.

**Communication:**
```cpp
#define SERIAL2_BAUDRATE 115200
#define MAX_RX_BUFFER 256
#define RX_TIMEOUT_WARNING 5000
```

**Bi-directional:**
```cpp
#define ENABLE_BIDIRECTIONAL true  // Enable ACK
#define ACK_INTERVAL 5             // Send ACK every N
#define LISTEN_TIMEOUT 500         // Wait for ACK (ms)
```

**PC Data Logging:**
```cpp
#define ENABLE_CSV_OUTPUT true     // CSV format
#define ENABLE_JSON_OUTPUT false   // JSON format
#define DATA_OUTPUT_INTERVAL 2000  // Output rate (ms)
```

### Kill-Switch Configuration

```cpp
// Roboter_Gruppe_9.ino (lines 38-43)
#define KILLSWITCH_GND_PIN 14      // GND pin
#define KILLSWITCH_READ_PIN 13     // Read pin
#define KILLSWITCH_HOLD_TIME 3000  // Hold time (ms)
#define KILLSWITCH_DEBUG true      // Debug output
```

### Watchdog Configuration

```cpp
// health_monitor.h (lines 27-34)
WatchdogConfig watchdogCfg = {
  .weakTimeout = 3000,           // WEAK threshold
  .lostTimeout = 8000,           // LOST threshold
  .weakRssiThreshold = -100,     // RSSI WEAK
  .criticalRssiThreshold = -110, // RSSI critical
  .recoveryInterval = 15000,     // Recovery wait
  .maxRecoveryAttempts = 3       // Max attempts
};
```

### LCD Display Selection

```cpp
// Roboter_Gruppe_9.ino, updateLCD() function
void updateLCD() {
  // ...
  // Uncomment ONE version:
  updateLCD_Version1_WideBar();    // ⭐ Recommended
  // updateLCD_Version2_Compact();
  // updateLCD_Version3_Detailed();
  // updateLCD_Version4_Original();
}
```

### Advanced: LoRa Parameters

**Modify in lora_handler.h:**
```cpp
// Line 160
response = sendLoRaCommand("AT+PARAMETER=12,7,1,4", 1000);
//                          AT+PARAMETER=SF,BW,CR,PRE
```

**Parameters:**
- **SF** (Spreading Factor): 7-12
  - 12 = Maximum range, slowest
  - 7 = Minimum range, fastest

- **BW** (Bandwidth): 7=125kHz, 8=250kHz, 9=500kHz
  - 7 = Best range
  - 9 = Best speed

- **CR** (Coding Rate): 1=4/5, 2=4/6, 3=4/7, 4=4/8
  - 1 = Best speed
  - 4 = Best error correction

- **PRE** (Preamble): 4-1024
  - 4 = Standard
  - Higher = Better detection

**Example configurations:**

Maximum range (current):
```cpp
AT+PARAMETER=12,7,1,4  // SF12, BW125, CR4/5, PRE4
```

Balanced:
```cpp
AT+PARAMETER=10,7,1,4  // SF10, BW125, CR4/5, PRE4
```

Maximum speed:
```cpp
AT+PARAMETER=7,9,1,4   // SF7, BW500, CR4/5, PRE4
```

---

## Architecture & Technical Details

### Code Structure

```
Roboter_Gruppe_9/
├── Roboter_Gruppe_9.ino    # Main program (730 lines)
├── config.h                # Configuration (57 lines)
├── structs.h               # Data structures (111 lines)
├── functions.h             # LCD helpers (29 lines)
├── lora_handler.h          # LoRa communication (264 lines)
├── health_monitor.h        # Watchdog & monitoring (310 lines)
├── serial_monitor.py       # Real-time viewer (182 lines)
└── data_logger.py          # Database logger (280 lines)
```

### Data Structures

**DeviceState** (10 fields):
```cpp
struct DeviceState {
  bool ledState;              // LED on/off
  int ledCount;               // LED toggle count
  bool touchState;            // Touch pressed
  unsigned long touchValue;   // Touch raw value
  int messageCount;           // Messages sent/received
  unsigned long lastMessageTime;  // Last message timestamp
  int sequenceNumber;         // Packet sequence
  int spinnerIndex;           // Animation frame
  int rssi;                   // Signal strength
  int snr;                    // Signal quality
};
```

**HealthMonitor** (13 fields):
```cpp
struct HealthMonitor {
  ConnectionState state;           // Current state
  unsigned long stateChangeTime;   // Last change
  unsigned long connectedSince;    // Connection start
  int rssiMin, rssiMax;           // RSSI range
  long rssiSum;                   // For average
  int rssiSamples;                // Sample count
  int expectedSeq;                // Expected sequence
  int packetsReceived;            // RX count
  int packetsLost;                // Lost count
  int packetsDuplicate;           // Duplicate count
  int recoveryAttempts;           // Recovery tries
  unsigned long lastRecoveryAttempt;
  unsigned long startTime;        // Uptime reference
};
```

### Memory Usage

**Flash (Program):**
```
Code:         ~230 KB
Strings:      ~20 KB
Total:        ~250 KB / 4 MB (6%)
```

**RAM (Runtime):**
```
Global vars:  ~5 KB
Stack:        ~10 KB
Heap:         ~30 KB (strings, buffers)
Total:        ~45 KB / 520 KB (9%)
```

**Optimization opportunities:**
- Use F() macro for strings (save ~5 KB RAM)
- Reduce buffer sizes (MAX_RX_BUFFER)
- Disable features not needed

### Communication Protocol

**LoRa Settings:**
```
Frequency:       868 MHz (Europe)
Spreading:       SF12 (maximum range)
Bandwidth:       125 kHz
Coding Rate:     4/5
Preamble:        4 symbols
Network ID:      6
Power:           Default (~14 dBm)
```

**Message Format:**
```
Sender → Receiver:
  SEQ:42,LED:1,TOUCH:0,SPIN:2,COUNT:42

Receiver → Sender (ACK):
  ACK,SEQ:5,LED:0,TOUCH:1,SPIN:3

Remote Command:
  CMD:RESTART
```

**AT Command Interface:**
```
AT                    → +OK
AT+RESET              → Reboot
AT+ADDRESS=2          → Set address
AT+NETWORKID=6        → Set network
AT+PARAMETER=12,7,1,4 → Set LoRa params
AT+SEND=1,5,HELLO     → Send to addr 1
+RCV=2,5,HELLO,-67,8  → Received from addr 2
```

### Performance

**Timing:**
```
Loop cycle:      ~10 ms (100 Hz)
Send interval:   2000 ms (0.5 Hz)
ACK timeout:     500 ms
LCD update:      100 ms (10 Hz)
Data logging:    2000 ms (0.5 Hz)
Health report:   30000 ms (0.033 Hz)
```

**LoRa Air Time (SF12, 125kHz):**
```
Payload    Air Time
─────────────────────
10 bytes   ~370 ms
50 bytes   ~990 ms
100 bytes  ~1800 ms
```

**Throughput:**
```
Best case:    ~3 messages/second (short payloads)
Typical:      0.5 messages/second (50 byte payloads)
With ACK:     Halved (bi-directional)
```

---

## Development & Customization

### Adding Custom Sensors

**Example: BME280 Temperature Sensor**

1. **Add to payload (sender):**
```cpp
// Roboter_Gruppe_9.ino, sender section
float temperature = bme.readTemperature();

String payload = "SEQ:" + String(local.sequenceNumber) +
                 ",LED:" + String(local.ledState) +
                 ",TEMP:" + String(temperature, 1);  // Add this
```

2. **Parse on receiver:**
```cpp
// parsePayload() function
int tempIdx = payload.indexOf("TEMP:");
if (tempIdx >= 0) {
  int comma = payload.indexOf(',', tempIdx);
  if (comma < 0) comma = payload.length();
  float temp = payload.substring(tempIdx + 5, comma).toFloat();
  Serial.print("Temperature: ");
  Serial.println(temp);
}
```

3. **Add to LCD display:**
```cpp
// updateLCD_Version1_WideBar()
lcd.setCursor(0, 1);
lcd.print("T:");
lcd.print(temperature, 1);
lcd.print("C ");
```

### Adding Custom Commands

**Example: LED control command**

```cpp
// processRemoteKillSwitch() function
void processRemoteKillSwitch(String payload) {
  if (payload.indexOf("CMD:RESTART") >= 0) {
    executeRestart("Remote command");
  }
  else if (payload.indexOf("CMD:LED_ON") >= 0) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("✓ LED turned ON via remote command");
  }
  else if (payload.indexOf("CMD:LED_OFF") >= 0) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("✓ LED turned OFF via remote command");
  }
}
```

### Extending to 3+ Devices

**Concept:** Mesh network with routing

```cpp
// Each device gets unique address
#define MY_ADDRESS 1  // This device
#define NETWORK_ID 6  // Same for all

// Routing table
struct Route {
  uint8_t destination;
  uint8_t nextHop;
  int rssi;
};
```

**Implementation outline:**
1. Broadcast discovery packets
2. Build routing table
3. Forward packets not for me
4. Update routes based on RSSI
5. Handle route failures

*Full mesh implementation beyond scope of this manual.*

### Power Optimization

**Deep Sleep (ESP32):**
```cpp
#include <esp_sleep.h>

// Send message, then sleep
sendLoRaMessage(payload, TARGET_ADDRESS);
esp_sleep_enable_timer_wakeup(10 * 1000000);  // 10 seconds
esp_deep_sleep_start();
```

**Considerations:**
- WiFi/Bluetooth disabled → saves 80 mA
- CPU halted → saves 40 mA
- Wake time ~1-2 seconds
- RAM contents lost
- Use RTC memory for persistence

**Battery life estimate:**
```
Active mode:     ~150 mA @ 3.3V → ~500 mW
Deep sleep:      ~10 mA @ 3.3V → ~33 mW
1000 mAh battery → ~6-7 hours active, ~100 hours sleeping
```

---

## FAQ

### General

**Q: Do I need two different code files for sender and receiver?**
A: No! Upload the identical code to both devices. Role is detected automatically by GPIO 16↔17 jumper.

**Q: What's the maximum range?**
A: With SF12, line-of-sight: 5-10 km in rural areas, 1-3 km in urban areas with obstacles.

**Q: Can I use 915 MHz LoRa modules instead of 868 MHz?**
A: Yes, but check local regulations. US uses 915 MHz, Europe uses 868 MHz. RYLR896 comes in both variants.

**Q: Does this work with ESP8266?**
A: Partially. ESP8266 has different pinouts and lacks some ESP32 features. Code needs modifications.

**Q: Can I add more sensors?**
A: Yes! See [Development & Customization](#development--customization) section.

### Technical

**Q: Why GPIO 13 instead of GPIO 12 for kill-switch?**
A: GPIO 12 is a strapping pin on ESP32 (used during boot). GPIO 13 is safer for runtime use.

**Q: Can I change the LoRa frequency?**
A: No, it's hardware-defined in RYLR896 module (868/915 MHz variants). Software only changes SF/BW/CR.

**Q: What happens if both devices send at the same time?**
A: Collision! Both messages lost. This is why sender waits 500ms for ACK (collision avoidance).

**Q: How accurate is packet loss tracking?**
A: Very accurate if sequence numbers continuous. Misses packets lost before first received message.

**Q: Can I use multiple LCD displays?**
A: Yes, if they have different I2C addresses. Initialize with different LiquidCrystal_I2C objects.

### Troubleshooting

**Q: Why does LoRa init fail without antenna?**
A: RYLR896 self-protects. Without antenna, reflected power damages module over time. Always connect antenna first.

**Q: Device reboots randomly. Why?**
A: Check power supply. ESP32 + LoRa can draw 500+ mA during transmission. Weak USB cable or power supply causes brownouts.

**Q: Can I use this indoors?**
A: Yes, but range reduced significantly. Walls, especially concrete/metal, block LoRa signals. Test in same room first.

**Q: Why no ACKs received even though devices close?**
A: Check `ENABLE_BIDIRECTIONAL true` in config.h. Also verify sender is actually listening (check Serial Monitor).

**Q: Can I log data without PC (SD card)?**
A: Not currently implemented, but possible. Add SD card library and write CSV to file instead of Serial.

---

## Reference

### Key Files Quick Reference

| File | Purpose | Lines | Key Content |
|------|---------|-------|-------------|
| `Roboter_Gruppe_9.ino` | Main program | 730 | setup(), loop(), kill-switch |
| `config.h` | Settings | 57 | Pins, addresses, enable flags |
| `lora_handler.h` | LoRa comm | 264 | AT commands, send/receive |
| `health_monitor.h` | Watchdog | 310 | State machine, recovery |
| `structs.h` | Data types | 111 | DeviceState, HealthMonitor |
| `functions.h` | LCD | 29 | Display initialization |

### GPIO Pin Map

| GPIO | Function | Direction | Notes |
|------|----------|-----------|-------|
| 2 | LED | Output | Built-in LED |
| 4 | Touch | Input | Touch sensor (T0) |
| 13 | Kill-switch | Input_PullUp | Read pin |
| 14 | Kill-switch GND | Output LOW | GND reference |
| 16 | Role select | Input_PullUp | Auto-detect role |
| 17 | Role GND | Output LOW | GND reference |
| 21 | LCD SDA | I2C | Display data |
| 22 | LCD SCL | I2C | Display clock |
| 25 | LoRa RX | RX | RYLR896 TX → ESP32 |
| 26 | LoRa TX | TX | ESP32 → RYLR896 RX |

### Configuration Flags

| Flag | Default | Purpose |
|------|---------|---------|
| `ENABLE_BIDIRECTIONAL` | true | Two-way communication |
| `ACK_INTERVAL` | 5 | ACK every N messages |
| `ENABLE_CSV_OUTPUT` | true | CSV data logging |
| `ENABLE_JSON_OUTPUT` | false | JSON data logging |
| `DATA_OUTPUT_INTERVAL` | 2000 | Logging rate (ms) |
| `KILLSWITCH_DEBUG` | true | Kill-switch debug output |

### Serial Commands

**From Serial Monitor to ESP32:**
```
(None currently - all automatic)
```

**From ESP32 to LoRa module (AT commands):**
```
AT                      - Test communication
AT+RESET                - Reboot module
AT+ADDRESS=X            - Set address (1-65535)
AT+NETWORKID=X          - Set network (0-255)
AT+PARAMETER=SF,BW,CR,P - Configure LoRa
AT+SEND=ADDR,LEN,DATA   - Send message
AT+MODE=0               - Transceiver mode
```

**LoRa module responses:**
```
+OK                     - Command successful
+ERR                    - Command failed
+READY                  - Module booted
+RCV=FROM,LEN,DATA,RSSI,SNR - Message received
```

### Health Monitor States

| State | Trigger | Meaning |
|-------|---------|---------|
| `UNKNOWN` | Initial | No data yet |
| `CONNECTING` | First message | Establishing link |
| `CONNECTED` | <3s interval | Normal operation |
| `WEAK` | 3-8s or RSSI<-100 | Poor connection |
| `LOST` | >8s no messages | Connection lost |

### RSSI Interpretation

| RSSI (dBm) | Quality | Expected Range | Packet Loss |
|------------|---------|----------------|-------------|
| -30 to -50 | Excellent | <10m | <0.1% |
| -50 to -70 | Very Good | 10-100m | <1% |
| -70 to -90 | Good | 100m-1km | 1-5% |
| -90 to -105 | Fair | 1-3km | 5-15% |
| -105 to -120 | Poor | 3-5km | >15% |
| <-120 | Critical | >5km | >50% |

### Spreading Factor Trade-offs

| SF | Range | Speed | Air Time | Sensitivity | Current |
|----|-------|-------|----------|-------------|---------|
| 7 | 1× | 16× | 41 ms | -123 dBm | Low |
| 8 | 1.6× | 8× | 72 ms | -126 dBm | Low |
| 9 | 2.5× | 4× | 144 ms | -129 dBm | Med |
| 10 | 4× | 2× | 288 ms | -132 dBm | Med |
| 11 | 6.3× | 1.3× | 577 ms | -134.5 dBm | High |
| 12 | 10× | 1× | 991 ms | -137 dBm | High |

*SF12 used by default for maximum range and reliability*

### Useful Calculations

**Battery life:**
```
Life (hours) = Battery (mAh) / Current (mA)

Active mode:  1000 mAh / 150 mA ≈ 6.7 hours
Sleep mode:   1000 mAh / 10 mA  ≈ 100 hours
```

**Maximum payload:**
```
SF12, BW125: ~50 bytes recommended
Higher SF → smaller max payload
Longer messages → longer air time → higher collision risk
```

**Packet loss calculation:**
```
Loss % = (Packets Lost / Total Expected) × 100
Total Expected = Packets Received + Packets Lost
```

---

## Appendix

### Document Change History

| Date | Version | Changes |
|------|---------|---------|
| Nov 2025 | 1.0 | Initial comprehensive manual |

### Glossary

- **ACK** - Acknowledgment message confirming receipt
- **AT Command** - Text command sent to LoRa module
- **Bi-directional** - Two-way communication (both send and receive)
- **dBm** - Decibel-milliwatts, signal power unit
- **GPIO** - General Purpose Input/Output pin
- **I2C** - Inter-Integrated Circuit, serial communication
- **LoRa** - Long Range radio technology
- **RSSI** - Received Signal Strength Indicator
- **SF** - Spreading Factor (7-12)
- **SNR** - Signal-to-Noise Ratio
- **Strapping Pin** - GPIO pin sampled during boot

### Credits

**Hardware:**
- ESP32 by Espressif Systems
- RYLR896 by Reyax
- LiquidCrystal_I2C library

**Development:**
- Built with Arduino framework
- Tested with PlatformIO
- Python 3.8+ for data logging

**Documentation:**
- Markdown with Obsidian formatting
- Diagrams created with ASCII art
- Code examples tested and verified

---

**🎉 You've reached the end of the comprehensive manual!**

For specific topics, see:
- `PC_LOGGING_README.md` - Python scripts detailed guide
- `README.md` - Project overview and quick reference

*Happy coding! 🚀*
