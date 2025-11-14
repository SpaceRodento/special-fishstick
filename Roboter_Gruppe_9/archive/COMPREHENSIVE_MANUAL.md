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
5. [Deployment Guide](#deployment-guide)
6. [Features & Capabilities](#features--capabilities)
7. [LCD Display Versions](#lcd-display-versions)
8. [Connection Watchdog](#connection-watchdog)
9. [Kill-Switch Usage](#kill-switch-usage)
10. [PC Data Logging](#pc-data-logging)
11. [Data Analysis & Visualization](#data-analysis--visualization)
12. [Troubleshooting](#troubleshooting)
13. [Configuration Guide](#configuration-guide)
14. [Architecture & Technical Details](#architecture--technical-details)
15. [Development & Customization](#development--customization)
16. [Advanced Topics](#advanced-topics)
17. [FAQ](#faq)
18. [Reference](#reference)

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

## Deployment Guide

### Production Deployment Checklist

This section provides step-by-step instructions for deploying your LoRa system from development to production.

#### Pre-Deployment Checklist

**Hardware Verification:**
- [ ] All connections verified with multimeter
- [ ] LoRa modules tested individually (AT commands work)
- [ ] LCD address verified (I2C scan)
- [ ] Antennas securely attached
- [ ] Power supply adequate (>500mA @ 3.3V)
- [ ] Enclosures prepared (if outdoor deployment)
- [ ] Backup hardware available

**Software Verification:**
- [ ] Code uploaded successfully to both devices
- [ ] Serial Monitor shows boot messages
- [ ] Role detection working (correct mode displayed)
- [ ] LoRa initialization successful
- [ ] Kill-switch tested (physical and remote)
- [ ] Data logging tested (if using PC connection)

**Configuration Verification:**
- [ ] Network ID same on both devices (`config.h`)
- [ ] Addresses correct (Sender=2, Receiver=1)
- [ ] LoRa parameters set (SF12, BW125)
- [ ] Bi-directional enabled (if needed)
- [ ] CSV/JSON logging configured
- [ ] Debug flags set appropriately

#### Step-by-Step Deployment

##### Phase 1: Lab Testing (Day 1)

**1. Bench Test (1 hour)**
```
Distance: < 1 meter
Environment: Indoor, controlled
Goal: Verify basic functionality
```

**Test procedure:**
1. Power up receiver first
2. Check Serial Monitor: ">>> RECEIVER MODE"
3. Verify LCD displays default screen
4. Power up sender
5. Check Serial Monitor: ">>> SENDER MODE"
6. Within 5 seconds, receiver should show messages
7. LCD should update with RSSI/SNR values
8. Test kill-switch on both devices
9. Verify ACK messages (check sender Serial Monitor)

**Success criteria:**
- ✅ Messages flow consistently
- ✅ RSSI > -40 dBm (close range)
- ✅ Packet loss < 1%
- ✅ LCD updates smoothly
- ✅ ACKs received every 5 messages

**2. Extended Run Test (4 hours)**
```
Distance: < 5 meters
Duration: 4+ hours
Goal: Verify stability
```

**Monitor for:**
- Memory leaks (check uptime doesn't cause crashes)
- Connection drops
- Packet loss trends
- RSSI stability

**Success criteria:**
- ✅ No crashes or reboots
- ✅ Packet loss remains < 2%
- ✅ No memory issues
- ✅ Connection state stays CONNECTED

**3. USB Data Logging Test (1 hour)**
```
Connection: Receiver → PC via USB
Goal: Verify data collection
```

**Test procedure:**
1. Connect receiver to PC
2. Run: `python serial_monitor.py /dev/ttyUSB0 115200`
3. Verify colored output appears
4. Run: `python data_logger.py /dev/ttyUSB0 115200 test.db`
5. Let run for 1 hour
6. Query database:
   ```bash
   sqlite3 test.db
   > SELECT COUNT(*) FROM lora_messages;
   > SELECT AVG(rssi), AVG(packet_loss) FROM lora_messages;
   ```

**Success criteria:**
- ✅ CSV data appears in Serial Monitor
- ✅ Database grows consistently
- ✅ No gaps in data
- ✅ Statistics make sense

##### Phase 2: Range Testing (Day 2)

**1. Line-of-Sight Test**
```
Distance: Progressive (10m → 100m → 500m → 1km+)
Environment: Outdoor, minimal obstacles
Goal: Determine maximum range
```

**Test procedure:**
1. Start at 10 meters
   - Verify RSSI, packet loss
   - Move sender further if stable
2. 50 meters
   - Check RSSI (should be -60 to -80 dBm)
   - Packet loss should be < 3%
3. 100 meters
   - RSSI around -70 to -90 dBm
   - Packet loss < 5%
4. 500 meters
   - RSSI around -90 to -105 dBm
   - Packet loss may increase to 10%
5. 1+ km
   - RSSI < -105 dBm
   - Packet loss varies, may need testing

**Record:**
- Distance
- RSSI
- SNR
- Packet loss %
- Connection state
- Environmental notes (weather, obstacles)

**2. Obstacle Test**
```
Distance: Fixed (e.g., 50m)
Obstacles: Walls, buildings, trees
Goal: Understand signal attenuation
```

**Test scenarios:**
- Through 1 wall (wood)
- Through 2 walls
- Through concrete wall
- Around building corner
- Through trees/foliage

**Record RSSI drop for each obstacle type**

##### Phase 3: Environmental Testing (Day 3)

**1. Temperature Test**
```
Conditions: Various temperatures
Goal: Verify operation range
```

**If indoor:**
- Cold: Refrigerator (2-5°C) - 1 hour
- Normal: Room temp (20-25°C)
- Warm: Near heater (30-35°C) - 1 hour

**Monitor for:**
- Crashes or reboots
- Performance degradation
- RSSI changes

**2. Interference Test**
```
Sources: WiFi, Bluetooth, other LoRa
Goal: Verify network isolation
```

**Test procedure:**
1. Baseline with no interference
2. Turn on multiple WiFi routers nearby
3. Run Bluetooth devices
4. If possible, test with other LoRa devices (different network ID)

**Success criteria:**
- ✅ WiFi/Bluetooth: No impact (different frequencies)
- ✅ Other LoRa (same freq): No impact if network ID different

##### Phase 4: Load Testing (Day 4)

**1. Continuous Operation**
```
Duration: 24 hours
Goal: Verify long-term stability
```

**Setup:**
- Both devices powered
- Receiver logging to PC
- Monitor periodically (every 2 hours)

**Monitor:**
- Uptime
- Packet loss trends
- Memory usage (if possible)
- Temperature (touch ESP32 - should be warm, not hot)

**Success criteria:**
- ✅ 24 hours without crashes
- ✅ Packet loss < 5% average
- ✅ No thermal issues

**2. Recovery Testing**
```
Duration: 2 hours
Goal: Verify auto-recovery works
```

**Test procedure:**
1. Let system run normally (CONNECTED state)
2. Disconnect LoRa antenna from sender
3. Wait for LOST state (8+ seconds)
4. Reconnect antenna
5. Verify recovery attempts start
6. System should return to CONNECTED

**Repeat:**
- Power cycle sender (simulate crash)
- Use kill-switch to restart
- Move sender out of range and back

**Success criteria:**
- ✅ System recovers automatically
- ✅ Recovery takes < 30 seconds
- ✅ No manual intervention needed

##### Phase 5: Production Deployment (Day 5+)

**1. Final Preparation**

**Firmware:**
- [ ] Disable debug output (KILLSWITCH_DEBUG = false)
- [ ] Set appropriate timeouts for your use case
- [ ] Verify network ID is unique
- [ ] Final code upload

**Hardware:**
- [ ] Secure all connections (consider solder instead of jumpers)
- [ ] Weatherproof enclosures (if outdoor)
- [ ] Label devices (SENDER/RECEIVER)
- [ ] Backup power plan (battery, solar)

**Documentation:**
- [ ] Record device locations
- [ ] Document network settings
- [ ] Create maintenance schedule
- [ ] Prepare troubleshooting guide

**2. Installation**

**Site survey:**
1. Walk the deployment area
2. Identify obstacles
3. Plan device placement
4. Test preliminary locations

**Installation:**
1. Install receiver first (fixed location)
2. Power up and verify operation
3. Install sender (mobile or fixed)
4. Verify communication
5. Mark RSSI and packet loss baseline

**3. Post-Deployment Monitoring**

**Day 1:**
- Check every 2 hours
- Verify messages flowing
- Check RSSI stability
- Test kill-switch accessibility

**Week 1:**
- Check daily
- Download logs from PC (if logging)
- Analyze packet loss trends
- Check for any crashes (uptime counter)

**Month 1:**
- Check weekly
- Analyze long-term trends
- Plan any optimizations
- Update documentation

#### Deployment Scenarios

##### Scenario 1: Indoor Building Monitoring

**Use case:** Office sensors reporting to central hub

**Configuration:**
```cpp
// Reduce transmit power (not needed indoors)
// In lora_handler.h after init:
sendLoRaCommand("AT+CRFOP=10");  // 10dBm instead of 15

// Faster send rate (less delay needed)
#define SEND_INTERVAL 1000  // 1 second instead of 2

// Tighter timeouts (close range)
.weakTimeout = 2000,   // 2s
.lostTimeout = 5000,   // 5s
```

**Considerations:**
- Walls attenuate signal
- Metal ducts/pipes cause reflections
- Concrete floors block signal
- Place receiver in central location

**Expected performance:**
- Range: 50-100m through walls
- RSSI: -60 to -90 dBm
- Packet loss: 1-3%

##### Scenario 2: Outdoor Long-Range

**Use case:** Farm sensors, weather stations

**Configuration:**
```cpp
// Maximum power
sendLoRaCommand("AT+CRFOP=15");  // 15dBm max

// Keep SF12 for maximum range
// Current settings optimal

// Longer timeouts (distance + weather)
.weakTimeout = 5000,   // 5s
.lostTimeout = 15000,  // 15s
```

**Considerations:**
- Height is critical (raise antennas)
- Weather affects signal (rain, snow)
- Temperature extremes
- Solar power recommended
- Weatherproof enclosures essential

**Expected performance:**
- Range: 1-5 km line of sight
- RSSI: -90 to -115 dBm
- Packet loss: 5-15%

##### Scenario 3: Mobile Robot Control

**Use case:** RC vehicle, rover, drone

**Configuration:**
```cpp
// Bi-directional essential
#define ENABLE_BIDIRECTIONAL true
#define ACK_INTERVAL 2  // More frequent ACKs

// Fast reaction time
#define SEND_INTERVAL 500  // 0.5 seconds

// Aggressive recovery
.maxRecoveryAttempts = 5
```

**Considerations:**
- Fast-moving creates Doppler effect
- Orientation affects antenna
- Battery life critical
- Kill-switch essential for safety
- Remote kill-switch for emergency stop

**Expected performance:**
- Range: 100-500m (depends on obstacles)
- RSSI: Varies rapidly with movement
- Packet loss: Higher during movement (5-10%)

#### Maintenance Schedule

**Daily (if critical):**
- Visual inspection (power LEDs on)
- Check Serial Monitor for errors
- Verify data logging

**Weekly:**
- Download and analyze logs
- Check packet loss trends
- Verify battery levels (if battery powered)
- Clean enclosures (if outdoor)

**Monthly:**
- Deep inspection of connections
- Antenna check (corrosion, damage)
- Firmware update check
- Performance review (vs. baseline)

**Yearly:**
- Replace batteries (preventive)
- Check all solder joints
- Update to latest firmware
- Performance audit

#### Common Deployment Mistakes

**❌ Mistake 1: Not testing thoroughly before deployment**
- Result: Failures in production
- Solution: Follow all testing phases

**❌ Mistake 2: Inadequate power supply**
- Result: Random reboots, brownouts
- Solution: Use quality power supplies rated > 500mA

**❌ Mistake 3: Antenna inside metal enclosure**
- Result: Signal blocked, no communication
- Solution: External antenna or plastic enclosure

**❌ Mistake 4: Same Network ID as neighbors**
- Result: Interference, packet collisions
- Solution: Change Network ID to unique value

**❌ Mistake 5: No backup plan**
- Result: System down = project failure
- Solution: Spare devices, documented recovery procedure

**❌ Mistake 6: Forgetting environmental factors**
- Result: Rain damage, overheating
- Solution: Weatherproof enclosures, temperature monitoring

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

## Data Analysis & Visualization

### Overview

Once you've collected data using `data_logger.py`, you can analyze it to gain insights into your LoRa system's performance. This section covers offline analysis tools and techniques.

### Quick Analysis with SQLite

**Basic statistics:**
```sql
-- Open database
sqlite3 lora_data.db

-- Total messages logged
SELECT COUNT(*) FROM lora_messages;

-- Average signal quality
SELECT
  AVG(rssi) as avg_rssi,
  MIN(rssi) as min_rssi,
  MAX(rssi) as max_rssi,
  AVG(snr) as avg_snr,
  AVG(packet_loss) as avg_loss
FROM lora_messages;

-- Connection state distribution
SELECT
  connection_state,
  COUNT(*) as occurrences,
  ROUND(COUNT(*) * 100.0 / (SELECT COUNT(*) FROM lora_messages), 2) as percentage
FROM lora_messages
GROUP BY connection_state;

-- Packet loss over time (hourly)
SELECT
  DATETIME((timestamp - MIN(timestamp)) / 3600000, 'unixepoch') as hour,
  AVG(packet_loss) as avg_loss
FROM lora_messages
GROUP BY hour
ORDER BY hour;

-- RSSI histogram (10 dBm bins)
SELECT
  (rssi / 10) * 10 as rssi_bin,
  COUNT(*) as count
FROM lora_messages
GROUP BY rssi_bin
ORDER BY rssi_bin;
```

### Python Analysis Script

**analyze_data.py** - Comprehensive analysis tool

Create this script in your `Roboter_Gruppe_9/` folder:

```python
#!/usr/bin/env python3
"""
analyze_data.py - LoRa Data Analysis Tool

Analyzes SQLite database from data_logger.py and generates:
- Statistical summary
- RSSI/SNR plots
- Packet loss trends
- Connection state timeline
- PDF report

Usage:
    python analyze_data.py lora_data.db
    python analyze_data.py lora_data.db --output report.pdf
"""

import sqlite3
import sys
from datetime import datetime
import argparse

# Optional imports (install if needed)
try:
    import matplotlib.pyplot as plt
    import matplotlib.dates as mdates
    from matplotlib.backends.backend_pdf import PdfPages
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    print("⚠️  Matplotlib not installed. Plots disabled.")
    print("   Install with: pip install matplotlib")

try:
    import pandas as pd
    HAS_PANDAS = True
except ImportError:
    HAS_PANDAS = False
    print("⚠️  Pandas not installed. Advanced analysis disabled.")
    print("   Install with: pip install pandas")


def load_data(db_file):
    """Load data from SQLite database"""
    conn = sqlite3.connect(db_file)

    if HAS_PANDAS:
        df = pd.read_sql_query("SELECT * FROM lora_messages", conn,
                                parse_dates=['timestamp'])
        return df
    else:
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM lora_messages")
        data = cursor.fetchall()
        conn.close()
        return data


def print_summary(db_file):
    """Print statistical summary"""
    conn = sqlite3.connect(db_file)
    cursor = conn.cursor()

    print("\n" + "="*60)
    print("  LoRa System Analysis Report")
    print("="*60)

    # Basic info
    cursor.execute("SELECT COUNT(*) FROM lora_messages")
    total = cursor.fetchone()[0]
    print(f"\n📊 Dataset:")
    print(f"   Total records: {total}")

    if total == 0:
        print("   ⚠️  No data in database!")
        return

    cursor.execute("SELECT MIN(timestamp), MAX(timestamp) FROM lora_messages")
    start, end = cursor.fetchone()
    duration = (datetime.fromisoformat(end) - datetime.fromisoformat(start)).total_seconds()
    print(f"   Duration: {duration/3600:.1f} hours")
    print(f"   Start: {start}")
    print(f"   End: {end}")

    # RSSI statistics
    cursor.execute("""
        SELECT AVG(rssi), MIN(rssi), MAX(rssi),
               AVG(snr), MIN(snr), MAX(snr)
        FROM lora_messages
    """)
    rssi_avg, rssi_min, rssi_max, snr_avg, snr_min, snr_max = cursor.fetchone()

    print(f"\n📡 Signal Quality:")
    print(f"   RSSI: avg={rssi_avg:.1f} dBm, min={rssi_min} dBm, max={rssi_max} dBm")
    print(f"   SNR:  avg={snr_avg:.1f} dB, min={snr_min} dB, max={snr_max} dB")

    # Signal quality assessment
    if rssi_avg > -70:
        quality = "Excellent"
    elif rssi_avg > -90:
        quality = "Good"
    elif rssi_avg > -105:
        quality = "Fair"
    else:
        quality = "Poor"
    print(f"   Overall quality: {quality}")

    # Packet loss
    cursor.execute("SELECT AVG(packet_loss), MAX(packet_loss) FROM lora_messages")
    loss_avg, loss_max = cursor.fetchone()
    print(f"\n📦 Packet Loss:")
    print(f"   Average: {loss_avg:.2f}%")
    print(f"   Maximum: {loss_max:.2f}%")

    if loss_avg < 1:
        print(f"   Assessment: Excellent (< 1%)")
    elif loss_avg < 5:
        print(f"   Assessment: Good (1-5%)")
    elif loss_avg < 10:
        print(f"   Assessment: Fair (5-10%)")
    else:
        print(f"   Assessment: Poor (> 10%)")

    # Connection states
    cursor.execute("""
        SELECT connection_state, COUNT(*),
               ROUND(COUNT(*) * 100.0 / (SELECT COUNT(*) FROM lora_messages), 2)
        FROM lora_messages
        GROUP BY connection_state
    """)

    print(f"\n🔗 Connection States:")
    for state, count, pct in cursor.fetchall():
        print(f"   {state:10s}: {count:6d} ({pct:5.2f}%)")

    # Messages per hour
    avg_rate = total / (duration / 3600) if duration > 0 else 0
    print(f"\n⏱️  Message Rate:")
    print(f"   Average: {avg_rate:.1f} messages/hour")

    conn.close()
    print("\n" + "="*60 + "\n")


def plot_rssi_timeline(df, ax):
    """Plot RSSI over time"""
    ax.plot(df['timestamp'], df['rssi'], 'b-', alpha=0.7, linewidth=0.5)
    ax.set_xlabel('Time')
    ax.set_ylabel('RSSI (dBm)')
    ax.set_title('Signal Strength Over Time')
    ax.grid(True, alpha=0.3)
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))

    # Add quality zones
    ax.axhspan(-120, -105, alpha=0.1, color='red', label='Poor')
    ax.axhspan(-105, -90, alpha=0.1, color='yellow', label='Fair')
    ax.axhspan(-90, -70, alpha=0.1, color='lightgreen', label='Good')
    ax.axhspan(-70, -40, alpha=0.1, color='green', label='Excellent')


def plot_packet_loss(df, ax):
    """Plot packet loss over time"""
    ax.plot(df['timestamp'], df['packet_loss'], 'r-', alpha=0.7)
    ax.set_xlabel('Time')
    ax.set_ylabel('Packet Loss (%)')
    ax.set_title('Packet Loss Over Time')
    ax.grid(True, alpha=0.3)
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax.set_ylim(bottom=0)


def plot_rssi_histogram(df, ax):
    """Plot RSSI distribution"""
    ax.hist(df['rssi'], bins=30, color='blue', alpha=0.7, edgecolor='black')
    ax.set_xlabel('RSSI (dBm)')
    ax.set_ylabel('Frequency')
    ax.set_title('RSSI Distribution')
    ax.grid(True, alpha=0.3, axis='y')

    # Add mean line
    mean_rssi = df['rssi'].mean()
    ax.axvline(mean_rssi, color='red', linestyle='--', linewidth=2,
               label=f'Mean: {mean_rssi:.1f} dBm')
    ax.legend()


def plot_connection_states(df, ax):
    """Plot connection state timeline"""
    # Map states to numbers for plotting
    state_map = {'OK': 3, 'WEAK': 2, 'LOST': 1, 'UNKNOWN': 0}
    df['state_num'] = df['connection_state'].map(state_map)

    ax.plot(df['timestamp'], df['state_num'], 'g-', linewidth=1)
    ax.set_xlabel('Time')
    ax.set_ylabel('Connection State')
    ax.set_yticks([0, 1, 2, 3])
    ax.set_yticklabels(['UNKNOWN', 'LOST', 'WEAK', 'OK'])
    ax.set_title('Connection State Timeline')
    ax.grid(True, alpha=0.3)
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))

    # Color background
    ax.axhspan(-0.5, 0.5, alpha=0.2, color='gray')
    ax.axhspan(0.5, 1.5, alpha=0.2, color='red')
    ax.axhspan(1.5, 2.5, alpha=0.2, color='yellow')
    ax.axhspan(2.5, 3.5, alpha=0.2, color='green')


def generate_plots(df, output_file=None):
    """Generate all plots"""
    if not HAS_MATPLOTLIB:
        print("⚠️  Matplotlib not installed. Cannot generate plots.")
        return

    print("📊 Generating plots...")

    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle('LoRa System Performance Analysis', fontsize=16, fontweight='bold')

    plot_rssi_timeline(df, ax1)
    plot_packet_loss(df, ax2)
    plot_rssi_histogram(df, ax3)
    plot_connection_states(df, ax4)

    plt.tight_layout()

    if output_file:
        print(f"💾 Saving to {output_file}...")
        if output_file.endswith('.pdf'):
            with PdfPages(output_file) as pdf:
                pdf.savefig(fig)
        else:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"✓ Saved: {output_file}")
    else:
        plt.show()

    plt.close()


def main():
    parser = argparse.ArgumentParser(description='Analyze LoRa data from SQLite database')
    parser.add_argument('database', help='SQLite database file')
    parser.add_argument('--output', '-o', help='Output file for plots (PDF or PNG)')
    parser.add_argument('--no-plots', action='store_true', help='Skip plot generation')

    args = parser.parse_args()

    # Check if database exists
    try:
        conn = sqlite3.connect(args.database)
        conn.close()
    except sqlite3.Error as e:
        print(f"❌ Error: Cannot open database '{args.database}'")
        print(f"   {e}")
        sys.exit(1)

    # Print summary
    print_summary(args.database)

    # Generate plots
    if not args.no_plots and HAS_MATPLOTLIB and HAS_PANDAS:
        df = load_data(args.database)
        if len(df) > 0:
            generate_plots(df, args.output)

    print("✓ Analysis complete!\n")


if __name__ == '__main__':
    main()
```

Save this script and use it after collecting data.

### Using the Analysis Script

**Install dependencies:**
```bash
pip install matplotlib pandas
```

**Basic usage:**
```bash
# Print summary only
python analyze_data.py lora_data.db --no-plots

# Show interactive plots
python analyze_data.py lora_data.db

# Save plots to PDF
python analyze_data.py lora_data.db --output report.pdf

# Save plots to PNG
python analyze_data.py lora_data.db --output charts.png
```

**The script generates:**
1. **Statistical summary** - Printed to console
2. **RSSI timeline** - Signal strength over time with quality zones
3. **Packet loss plot** - Loss percentage trends
4. **RSSI histogram** - Distribution of signal strengths
5. **Connection state timeline** - Visual state changes

### Advanced Analysis Examples

**Correlation between RSSI and packet loss:**
```python
import sqlite3
import pandas as pd
import matplotlib.pyplot as plt

conn = sqlite3.connect('lora_data.db')
df = pd.read_sql_query("SELECT rssi, packet_loss FROM lora_messages", conn)

plt.scatter(df['rssi'], df['packet_loss'], alpha=0.5)
plt.xlabel('RSSI (dBm)')
plt.ylabel('Packet Loss (%)')
plt.title('RSSI vs Packet Loss')
plt.grid(True)
plt.show()
```

**Hourly statistics:**
```python
df = pd.read_sql_query("SELECT * FROM lora_messages", conn,
                        parse_dates=['timestamp'])
df['hour'] = df['timestamp'].dt.hour

hourly = df.groupby('hour').agg({
    'rssi': ['mean', 'min', 'max'],
    'packet_loss': 'mean',
    'message_count': 'count'
})

print(hourly)
```

**Export to CSV for Excel:**
```python
conn = sqlite3.connect('lora_data.db')
df = pd.read_sql_query("SELECT * FROM lora_messages", conn)
df.to_csv('lora_export.csv', index=False)
print("✓ Exported to lora_export.csv")
```

### Visualization Tips

**What to look for:**

1. **RSSI Timeline:**
   - Stable = Good deployment
   - Fluctuating = Movement or interference
   - Dropping trend = Battery dying or distance increasing

2. **Packet Loss:**
   - Spikes correlate with RSSI drops
   - Gradual increase = Failing hardware
   - Periodic = Interference source

3. **Connection States:**
   - Mostly green (OK) = Healthy system
   - Yellow (WEAK) periods = Investigate cause
   - Red (LOST) = Critical issue

4. **RSSI Histogram:**
   - Single peak = Stable conditions
   - Two peaks = Two distinct scenarios (e.g., mobile device)
   - Spread out = Unstable environment

### Performance Benchmarks

**Typical good results:**
```
RSSI:         -70 dBm average (indoor)
              -95 dBm average (outdoor 1km)
Packet Loss:  < 2% (indoor)
              < 10% (outdoor)
Connection:   > 95% in OK state
Uptime:       24+ hours no crashes
```

**Red flags:**
```
RSSI:         < -110 dBm consistently
Packet Loss:  > 20%
Connection:   Frequent state changes
Crashes:      Any unexpected reboots
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

---

## Advanced Topics

This section covers advanced architectural concepts, performance optimization, and security features that go beyond basic usage.

### Mesh Networking (3+ Devices)

**Concept:** Multi-hop routing for extended range and reliability

**Basic Mesh Architecture:**
```cpp
// Each device gets unique address
#define MY_ADDRESS 1  // This device (1, 2, 3...)
#define NETWORK_ID 6  // Same for all devices in mesh

// Routing table structure
struct Route {
  uint8_t destination;    // Final destination address
  uint8_t nextHop;        // Next device to forward to
  int rssi;               // Signal quality to next hop
  unsigned long lastSeen; // Timestamp for route aging
};

Route routingTable[MAX_DEVICES];
```

**Implementation Steps:**

1. **Discovery Phase:**
```cpp
// Broadcast "HELLO" packets periodically
String helloPacket = "HELLO," + String(MY_ADDRESS) +
                     ",RSSI:" + String(WiFi.RSSI());
sendLoRaMessage(helloPacket, LORA_BROADCAST_ADDR);

// Listen for responses and build neighbor table
void processHello(String payload, int rssi, uint8_t sender) {
  addNeighbor(sender, rssi);
  updateRoutingTable(sender, sender, rssi);  // Direct route
}
```

2. **Packet Forwarding:**
```cpp
void forwardPacket(String payload, uint8_t destination) {
  // Check if packet is for me
  if (destination == MY_ADDRESS) {
    processPacket(payload);
    return;
  }

  // Find route in table
  Route* route = findRoute(destination);
  if (route != NULL) {
    // Forward to next hop
    sendLoRaMessage(payload, route->nextHop);
    Serial.print("→ Forwarding to device ");
    Serial.println(route->nextHop);
  } else {
    Serial.println("⚠ No route to destination");
  }
}
```

3. **Route Maintenance:**
```cpp
void maintainRoutes() {
  unsigned long now = millis();

  // Age out old routes (>60 seconds)
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (routingTable[i].destination != 0 &&
        now - routingTable[i].lastSeen > 60000) {
      Serial.print("✗ Route to ");
      Serial.print(routingTable[i].destination);
      Serial.println(" expired");
      routingTable[i].destination = 0;  // Invalidate route
    }
  }

  // Request route updates if needed
  if (now - lastDiscovery > 30000) {  // Every 30 seconds
    sendDiscoveryPacket();
    lastDiscovery = now;
  }
}
```

**Mesh Network Challenges:**
- **Packet loops:** Add TTL field, decrement on each hop
- **Bandwidth:** Each forwarded packet uses air time
- **Latency:** Multi-hop adds delay (1-2 seconds per hop)
- **Power:** Forwarding drains battery faster
- **Route changes:** Handle device mobility/failure

**Example 3-Device Setup:**
```
Device 1 (Base)  ←→  Device 2 (Relay)  ←→  Device 3 (Remote)
   [1000m]               [1000m]

Device 1 can reach Device 3 through Device 2 as relay
```

*Full mesh implementation is beyond the scope of this manual. See LoRaMesh or Meshtastic projects for complete implementations.*

### Power Optimization

**Goal:** Extend battery life from ~7 hours to 100+ hours

#### Deep Sleep Mode

**Basic Deep Sleep:**
```cpp
#include <esp_sleep.h>

void enterDeepSleep(int seconds) {
  Serial.println("💤 Entering deep sleep...");
  Serial.flush();  // Wait for serial to finish

  // Configure wake-up timer
  esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);  // Microseconds

  // Optional: Configure wake on GPIO
  // esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 0);  // Wake on LOW

  // Enter deep sleep (resets ESP32 on wake)
  esp_deep_sleep_start();
}

void loop() {
  // Send data
  sendLoRaMessage(payload, TARGET_ADDRESS);

  // Sleep for 10 seconds
  enterDeepSleep(10);

  // Code here never runs - deep sleep resets the chip!
}
```

**Deep Sleep Power Savings:**
| Mode | Current Draw | Power @3.3V | Notes |
|------|-------------|-------------|-------|
| Active (WiFi ON) | ~150 mA | ~500 mW | Default mode |
| Active (WiFi OFF) | ~80 mA | ~264 mW | WiFi disabled |
| Light Sleep | ~15 mA | ~50 mW | CPU halted, RAM retained |
| Deep Sleep | ~10 µA | ~33 µW | Everything off except RTC |

**Battery Life Calculation:**
```
1000 mAh battery:

Continuous active:  1000 mAh / 150 mA = 6.7 hours
Continuous sleep:   1000 mAh / 0.01 mA = 100,000 hours (11 years!)

Duty cycle (1 sec active, 59 sec sleep):
Avg current = (1/60 × 150 mA) + (59/60 × 0.01 mA) ≈ 2.5 mA
Battery life = 1000 / 2.5 = 400 hours (16 days)
```

#### Persistent Data (RTC Memory)

**Problem:** Deep sleep clears RAM
**Solution:** Use RTC memory (8 KB survives deep sleep)

```cpp
RTC_DATA_ATTR int messageCount = 0;      // Survives deep sleep
RTC_DATA_ATTR int sequenceNumber = 0;

void setup() {
  // messageCount retains value after wake from deep sleep!
  Serial.print("Wake count: ");
  Serial.println(messageCount++);
}
```

#### Light Sleep (Alternative)

**Pros:** Faster wake, RAM retained
**Cons:** Higher power (~15 mA vs 10 µA)

```cpp
void enterLightSleep(int seconds) {
  // Configure timer wake source
  esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);

  // Enter light sleep (resumes from here after wake)
  esp_light_sleep_start();

  // Code continues after wake!
  Serial.println("✓ Woke from light sleep");
}
```

#### Optimizing LoRa for Low Power

```cpp
// Reduce transmit power (default = 15 dBm)
sendATCommand("AT+CRFOP=10");  // 10 dBm saves ~30 mA

// Increase send interval
#define SEND_INTERVAL 60000  // 60 seconds instead of 2 seconds

// Use lower spreading factor if range allows
// SF7 uses 1/3 the air time of SF12 → less TX current
```

**Power Budget Example:**
```
Active time per cycle:  2 seconds
Sleep time per cycle:   58 seconds
Cycle time:            60 seconds

Active phase:  150 mA × 2s   = 300 mAs
Sleep phase:   0.01 mA × 58s = 0.58 mAs
Total per cycle:               300.58 mAs

Cycles per hour: 3600 / 60 = 60
Current per hour: 300.58 × 60 / 3600 = 5 mA average

1000 mAh battery: 1000 / 5 = 200 hours (8.3 days)
```

### Over-The-Air (OTA) Updates

**Concept:** Update firmware wirelessly without USB cable

**Basic OTA Setup:**

```cpp
#include <WiFi.h>
#include <ArduinoOTA.h>

void setup() {
  // Connect to WiFi
  WiFi.begin("SSID", "password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Configure OTA
  ArduinoOTA.setHostname("lora-sender-01");
  ArduinoOTA.setPassword("secure_password");

  ArduinoOTA.onStart([]() {
    Serial.println("🔄 OTA Update Starting...");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\n✓ OTA Update Complete!");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.print("❌ OTA Error: ");
    Serial.println(error);
  });

  ArduinoOTA.begin();
  Serial.println("✓ OTA Ready");
}

void loop() {
  ArduinoOTA.handle();  // Check for OTA updates
  // ... rest of code
}
```

**Uploading via OTA:**
```bash
# Arduino IDE: Tools → Port → Network Port → lora-sender-01
# Or via CLI:
python ~/.arduino15/packages/esp32/tools/esptool_py/*/espota.py \
  -i 192.168.1.100 -p 3232 --auth=secure_password \
  -f Roboter_Gruppe_9.ino.bin
```

**Considerations:**
- Requires WiFi (increases power consumption)
- Can enable WiFi only when needed (e.g., on button press)
- OTA partition needs ~1.3 MB flash space
- Useful for deployed devices in hard-to-reach locations

### Data Encryption

**Why:** Prevent eavesdropping and tampering

**Simple XOR Encryption (Lightweight):**

```cpp
String xorEncrypt(String data, uint8_t key) {
  String encrypted = "";
  for (int i = 0; i < data.length(); i++) {
    encrypted += char(data[i] ^ key);
  }
  return encrypted;
}

// Sender
String payload = "LED:1,TEMP:25.5";
String encrypted = xorEncrypt(payload, 0xA5);  // Key = 0xA5
sendLoRaMessage(encrypted, TARGET_ADDRESS);

// Receiver
String encrypted = receiveLoRaMessage();
String decrypted = xorEncrypt(encrypted, 0xA5);  // Same key
```

**⚠️ Warning:** XOR is NOT secure against serious attacks! Use for obfuscation only.

**AES Encryption (Secure):**

```cpp
#include <mbedtls/aes.h>

mbedtls_aes_context aes;
uint8_t key[16] = {0x2b, 0x7e, 0x15, ...};  // 128-bit key

void encryptAES(uint8_t* input, uint8_t* output, int len) {
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key, 128);
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input, output);
  mbedtls_aes_free(&aes);
}

// Use for sensitive data (coordinates, commands)
```

**Trade-offs:**
- Encryption adds CPU time (~5-10 ms per packet)
- Increases packet size (padding required)
- Key management complexity
- AES reduces air time vs no encryption

### Adaptive Spreading Factor

**Concept:** Automatically adjust SF based on signal quality for optimal throughput

```cpp
int currentSF = 12;  // Start with max range

void adaptSpreadingFactor(int rssi) {
  int targetSF = currentSF;

  // Strong signal → decrease SF (faster speed)
  if (rssi > -80 && currentSF > 7) {
    targetSF = currentSF - 1;
  }
  // Weak signal → increase SF (better range)
  else if (rssi < -105 && currentSF < 12) {
    targetSF = currentSF + 1;
  }

  if (targetSF != currentSF) {
    currentSF = targetSF;
    String cmd = "AT+PARAMETER=" + String(currentSF) + ",7,1,4";
    sendATCommand(cmd);
    Serial.print("📡 SF adjusted to: ");
    Serial.println(currentSF);
  }
}

void loop() {
  // In receiver
  if (receiveLoRaMessage(remote, payload)) {
    adaptSpreadingFactor(remote.rssi);
    // Process message...
  }
}
```

**Benefits:**
- SF7 at close range: 5.5 kbps (11× faster than SF12)
- SF12 at long range: 0.5 kbps (maximum sensitivity)
- Automatic optimization as distance changes
- Reduces air time (less collisions, better battery)

**Challenges:**
- Both devices must sync SF changes
- Need handshake protocol for SF negotiation
- Temporary packet loss during SF transition

### Performance Monitoring

**Track system health metrics:**

```cpp
struct PerformanceMetrics {
  unsigned long uptimeSeconds;
  int freeHeapKB;
  int minFreeHeapKB;
  float cpuUsage;
  int loopFrequency;
  int packetQueueDepth;
};

PerformanceMetrics perf;

void updatePerformanceMetrics() {
  perf.uptimeSeconds = millis() / 1000;
  perf.freeHeapKB = ESP.getFreeHeap() / 1024;
  perf.minFreeHeapKB = ESP.getMinFreeHeap() / 1024;

  // CPU usage estimation
  static unsigned long lastLoopTime = 0;
  static int loopCount = 0;
  loopCount++;

  if (millis() - lastLoopTime >= 1000) {
    perf.loopFrequency = loopCount;
    loopCount = 0;
    lastLoopTime = millis();
  }
}

void printPerformanceReport() {
  Serial.println("\n╔══════════ PERFORMANCE ══════════╗");
  Serial.print("║ Uptime:     ");
  Serial.print(perf.uptimeSeconds / 3600);
  Serial.println(" hours");
  Serial.print("║ Free RAM:   ");
  Serial.print(perf.freeHeapKB);
  Serial.println(" KB");
  Serial.print("║ Min RAM:    ");
  Serial.print(perf.minFreeHeapKB);
  Serial.println(" KB");
  Serial.print("║ Loop freq:  ");
  Serial.print(perf.loopFrequency);
  Serial.println(" Hz");
  Serial.println("╚═════════════════════════════════╝");
}
```

**Memory Leak Detection:**
```cpp
// Call every 10 seconds
if (ESP.getMinFreeHeap() < MEMORY_WARNING_THRESHOLD) {
  Serial.println("⚠️ LOW MEMORY WARNING!");
  // Possible memory leak - investigate
}
```

### Emergency Recovery Features

**Watchdog Timer (Auto-Reboot on Hang):**

```cpp
#include <esp_task_wdt.h>

void setup() {
  // Configure watchdog (10 second timeout)
  esp_task_wdt_init(10, true);  // 10s, panic on timeout
  esp_task_wdt_add(NULL);       // Add current task

  Serial.println("✓ Watchdog enabled");
}

void loop() {
  // Reset watchdog every loop iteration
  esp_task_wdt_reset();

  // If loop hangs for >10s, ESP32 reboots automatically
  // ... rest of code
}
```

**Remote Diagnostic Commands:**

```cpp
void processRemoteCommand(String cmd) {
  if (cmd == "CMD:STATUS") {
    // Send full status report
    sendStatusReport();
  }
  else if (cmd == "CMD:RESET_STATS") {
    // Reset packet counters
    health.packetsReceived = 0;
    health.packetsLost = 0;
    Serial.println("✓ Statistics reset");
  }
  else if (cmd == "CMD:SET_SF:9") {
    // Change spreading factor remotely
    int sf = cmd.substring(11).toInt();
    updateSpreadingFactor(sf);
  }
  else if (cmd == "CMD:PING") {
    // Simple ping/pong for connectivity test
    sendLoRaMessage("PONG", senderAddress);
  }
}
```

**Further Reading:**
- ESP32 Technical Reference Manual (power modes)
- LoRaWAN Specification (mesh networking standards)
- Meshtastic Project (open-source LoRa mesh)
- RadioHead Library (advanced packet radio)

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
