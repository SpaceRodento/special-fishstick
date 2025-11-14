# Features Summary - Roboter Gruppe 9

**Complete list of all features and how to enable/disable them**

Last updated: 2025-11-06

---

## 🎯 Core Features (Always Active)

These features are always compiled and cannot be disabled:

| Feature | Description | Hardware Required |
|---------|-------------|-------------------|
| **LoRa Communication** | RYLR896 module, SF12, 868 MHz | RYLR896 module |
| **Auto Role Detection** | GPIO 16↔17 jumper = receiver, floating = sender | Jumper wire |
| **Kill-Switch** | GPIO 13↔14 hold 3s = restart device | Jumper wire or button |
| **LED Blinking** | Built-in LED status indicator | GPIO 2 (built-in) |
| **Touch Sensor** | Capacitive touch detection | GPIO 4 (T0) |
| **Health Monitoring** | Connection watchdog, packet loss tracking | None |
| **Serial Debug** | Debug output at 115200 baud | USB cable |

---

## 🔌 Display Features

### I2C LCD Display (Receiver only)

| Feature | Toggle | Default | Hardware | Description |
|---------|--------|---------|----------|-------------|
| **LCD Display** | Always active on receiver | ON | I2C LCD 16x2 | Shows RSSI, message count, connection status |
| **LCD Version** | Comment/uncomment in code | Version 2 | Same | 4 different display layouts |

**LCD Versions:**
- **Version 1:** Wide RSSI bar + signal strength (recommended)
- **Version 2:** Compact with numbers (default)
- **Version 3:** Detailed info with SNR
- **Version 4:** Original simple display

**Change LCD version in `Roboter_Gruppe_9.ino`:**
```cpp
void updateLCD() {
  // Uncomment ONE version:
  // updateLCD_Version1_WideBar();
  updateLCD_Version2_Compact();  // ← Currently active
  // updateLCD_Version3_Detailed();
  // updateLCD_Version4_Original();
}
```

### TFT Display Station

| Feature | Toggle | Default | Hardware | Description |
|---------|--------|---------|----------|-------------|
| **Display Output** | `ENABLE_DISPLAY_OUTPUT` | `true` | None (UART only) | Send data via UART |
| **Display Station** | Separate code | N/A | ESP32-2432S022 TFT | External 320x240 TFT display |

**Hardware Setup:**
- Upload `Roboter_Display_TFT.ino` to ESP32-2432S022
- Connect: Robot GPIO 23 (TX) → Display GPIO 18 (RX)
- Connect: GND → GND

**Features:**
- Real-time data visualization
- Connection status
- RSSI/SNR display
- Alert messages
- 320x240 landscape display

**config.h:**
```cpp
#define ENABLE_DISPLAY_OUTPUT true   // Send data to display
#define DISPLAY_UPDATE_INTERVAL 2000 // Update every 2 seconds
#define DISPLAY_TX_PIN 23            // TX pin (connects to display RX)
```

---

## 📡 Communication Features

| Feature | Toggle | Default | Description | Testing Required |
|---------|--------|---------|-------------|------------------|
| **Bi-Directional** | `ENABLE_BIDIRECTIONAL` | `true` | Receiver sends ACK every N messages | Verify ACK count increases |
| **CSV Output** | `ENABLE_CSV_OUTPUT` | `true` | CSV data for PC logging | Check serial for `DATA_CSV,...` |
| **JSON Output** | `ENABLE_JSON_OUTPUT` | `false` | JSON data for PC logging | Alternative to CSV |

**config.h:**
```cpp
// Bi-directional communication
#define ENABLE_BIDIRECTIONAL true
#define ACK_INTERVAL 5               // Send ACK every 5 messages
#define LISTEN_TIMEOUT 500           // Wait 500ms for ACK

// PC Data Logging
#define ENABLE_CSV_OUTPUT true
#define ENABLE_JSON_OUTPUT false     // Use CSV OR JSON, not both
#define DATA_OUTPUT_INTERVAL 2000    // Output every 2 seconds
```

**Testing:**
- **ACK:** Check sender serial for `✓ ACK #X received`
- **CSV:** Use `python serial_monitor.py` to capture data
- **JSON:** Check serial for JSON formatted output

---

## 🔋 Sensor Features

All sensor features are **optional** and disabled by default.

### Battery Monitor

| Feature | Toggle | Default | Hardware | Pin |
|---------|--------|---------|----------|-----|
| Battery voltage monitoring | `ENABLE_BATTERY_MONITOR` | `false` | Voltage divider (2:1) | GPIO 35 |

**What it does:**
- Monitors battery voltage via ADC
- Reports low battery (<3.3V)
- Critical battery alert (<3.0V)

**Hardware:**
```
Battery+ ──[R1]── GPIO 35 ──[R2]── GND
         (10kΩ)            (10kΩ)

Max input: 6.6V (with 2:1 divider)
```

**config.h:**
```cpp
#define ENABLE_BATTERY_MONITOR false
#define BATTERY_PIN 35                   // ADC1_CH7
#define BATTERY_VOLTAGE_DIVIDER 2.0      // R1=R2 (2:1 ratio)
#define BATTERY_CHECK_INTERVAL 60000     // Check every 60s
#define BATTERY_LOW_THRESHOLD 3.3        // Warning
#define BATTERY_CRITICAL_THRESHOLD 3.0   // Critical
```

**Testing:**
- Connect battery via voltage divider
- Check serial: `🔋 Battery: 3.85V`
- Test warning: Use <3.3V battery

### Audio Detector (Fire Alarm)

| Feature | Toggle | Default | Hardware | Pin |
|---------|--------|---------|----------|-----|
| Fire alarm sound detection | `ENABLE_AUDIO_DETECTION` | `false` | MAX4466 microphone | GPIO 34 |

**What it does:**
- Detects smoke alarm audio pattern
- 3kHz frequency, 85dB volume
- 3-4 beeps per second pattern

**Hardware:**
- MAX4466 microphone amplifier module
- OUT → GPIO 34 (ADC1_CH6)
- VCC → 3.3V, GND → GND

**config.h:**
```cpp
#define ENABLE_AUDIO_DETECTION false
#define AUDIO_PIN 34                     // ADC1_CH6
#define AUDIO_SAMPLES 100                // Samples for RMS
#define AUDIO_THRESHOLD 200              // RMS threshold
#define AUDIO_PEAK_MIN 3                 // Min peaks/second
#define AUDIO_PEAK_MAX 5                 // Max peaks/second
#define AUDIO_COOLDOWN 5000              // 5s between alerts
```

**Testing:**
- Use actual smoke alarm, or
- Use 3kHz tone generator (phone app)
- 3-4 beeps per second
- Check serial: `🚨 FIRE ALARM DETECTED (audio)!`

### Light Detector (Fire Alarm)

| Feature | Toggle | Default | Hardware | Pin |
|---------|--------|---------|----------|-----|
| Fire alarm LED detection | `ENABLE_LIGHT_DETECTION` | `false` | TCS34725 RGB sensor | I2C (SDA=21, SCL=22) |

**What it does:**
- Detects flashing red LED
- Typical fire alarm pattern (~1 Hz)
- Uses RGB color sensor

**Hardware:**
- TCS34725 RGB color sensor
- SDA → GPIO 21, SCL → GPIO 22
- VCC → 3.3V, GND → GND
- Shares I2C bus with LCD

**Library required:**
```cpp
#include <Adafruit_TCS34725.h>
```

**config.h:**
```cpp
#define ENABLE_LIGHT_DETECTION false
// No additional pins needed (uses I2C)
```

**Testing:**
- Point sensor at flashing red LED
- Flash rate: ~1 Hz (fire alarm pattern)
- Check serial: `🚨 FIRE ALARM DETECTED (light)!`

### Current Monitor

| Feature | Toggle | Default | Hardware | Pin |
|---------|--------|---------|----------|-----|
| Power consumption tracking | `ENABLE_CURRENT_MONITOR` | `false` | INA219 sensor | I2C (SDA=21, SCL=22) |

**What it does:**
- Monitors current, voltage, power
- Tracks total energy usage (mAh)
- High current warnings

**Hardware:**
```
Battery+ ──► INA219 VIN+ ──► INA219 VIN- ──► ESP32 VIN
                │
                └──► I2C (SDA=21, SCL=22)
```

**Library required:**
```cpp
#include <Adafruit_INA219.h>
```

**config.h:**
```cpp
#define ENABLE_CURRENT_MONITOR false
#define CURRENT_MONITOR_I2C_ADDR 0x40    // INA219 I2C address
#define CURRENT_CHECK_INTERVAL 10000     // Check every 10s
#define CURRENT_HIGH_THRESHOLD 200       // Warning at 200mA
#define CURRENT_MAX_THRESHOLD 500        // Critical at 500mA
```

**Testing:**
- Check serial: `⚡ 3.85V, 85mA, 328mW`
- Check energy tracking: `🔋 Energy: 12.5 mAh`
- Vary load to test current measurement

---

## ⚙️ System Features

### Extended Telemetry

| Feature | Toggle | Default | Description |
|---------|--------|---------|-------------|
| Extended system info | `ENABLE_EXTENDED_TELEMETRY` | `false` | Adds uptime, heap, temperature to payload |

**What it adds:**
- Uptime (seconds since boot)
- Free heap memory (KB)
- Internal temperature (°C)

**config.h:**
```cpp
#define ENABLE_EXTENDED_TELEMETRY false
```

**Testing:**
- Check serial or display for uptime/heap/temp
- Verify values make sense

### Adaptive Spreading Factor

| Feature | Toggle | Default | Description |
|---------|--------|---------|-------------|
| Dynamic SF adjustment | `ENABLE_ADAPTIVE_SF` | `false` | Changes SF based on signal quality |

**What it does:**
- Good signal (>-80dBm): Lower SF (faster, less air time)
- Weak signal (<-105dBm): Higher SF (slower, more reliable)
- Automatic optimization

**config.h:**
```cpp
#define ENABLE_ADAPTIVE_SF false
#define ADAPTIVE_SF_RSSI_GOOD -80        // Threshold for decreasing SF
#define ADAPTIVE_SF_RSSI_WEAK -105       // Threshold for increasing SF
```

**Testing:**
- Move sender away from receiver
- Check serial: `📡 SF adjusted: 12 → 10` or vice versa
- Verify SF changes with distance

### Advanced Commands

| Feature | Toggle | Default | Description |
|---------|--------|---------|-------------|
| Extended remote commands | `ENABLE_ADVANCED_COMMANDS` | `false` | Remote configuration and diagnostics |

**What it adds:**
- `CMD:STATUS` - Get device status
- `CMD:RESET_STATS` - Reset packet statistics
- `CMD:SET_POWER:X` - Set TX power (0-20 dBm)
- `CMD:SET_SF:X` - Set spreading factor (7-12)

**config.h:**
```cpp
#define ENABLE_ADVANCED_COMMANDS false
```

**Testing:**
- Send commands via LoRa
- Check serial for response
- Verify command executed

### Encryption

| Feature | Toggle | Default | Description |
|---------|--------|---------|-------------|
| XOR payload encryption | `ENABLE_ENCRYPTION` | `false` | Simple payload obfuscation |

**What it does:**
- XOR cipher (not cryptographically secure!)
- Basic privacy / obfuscation
- Both devices must use same key

**config.h:**
```cpp
#define ENABLE_ENCRYPTION false
#define ENCRYPTION_KEY 0xA5              // XOR key (0x00-0xFF)
```

**Testing:**
- Enable on both sender and receiver
- Verify communication still works
- Change key on one device → communication fails

### Packet Statistics

| Feature | Toggle | Default | Description |
|---------|--------|---------|-------------|
| Detailed packet tracking | `ENABLE_PACKET_STATS` | `false` | Retries, duplicates, out-of-order |

**What it adds:**
- Retry count
- Duplicate packet detection
- Out-of-order packet tracking
- Detailed statistics report

**config.h:**
```cpp
#define ENABLE_PACKET_STATS false
#define PACKET_STATS_INTERVAL 30000      // Report every 30s
```

**Testing:**
- Check serial for detailed statistics
- Verify stats make sense

### Performance Monitor

| Feature | Toggle | Default | Description |
|---------|--------|---------|-------------|
| CPU/memory monitoring | `ENABLE_PERFORMANCE_MONITOR` | `false` | Loop frequency, CPU usage, memory |

**What it adds:**
- Loop frequency (Hz)
- CPU usage (%)
- Memory consumption
- Performance report

**config.h:**
```cpp
#define ENABLE_PERFORMANCE_MONITOR false
#define PERF_REPORT_INTERVAL 60000       // Report every 60s
```

**Testing:**
- Check serial for performance metrics
- Verify loop frequency >100 Hz
- Monitor memory over time

### Runtime Configuration

| Feature | Toggle | Default | Description |
|---------|--------|---------|-------------|
| Serial configuration | `ENABLE_RUNTIME_CONFIG` | `false` | Change settings via serial commands |

**What it does:**
- Configure system without re-uploading code
- Commands: `CONFIG:INTERVAL:2000`, `CONFIG:SF:10`, `CONFIG:POWER:15`

**config.h:**
```cpp
#define ENABLE_RUNTIME_CONFIG false
#define CONFIG_COMMAND_PREFIX "CONFIG:"
```

**Testing:**
- Send `CONFIG:INTERVAL:1000` via serial
- Verify setting changes
- Check serial for confirmation

### Watchdog Timer

| Feature | Toggle | Default | Description |
|---------|--------|---------|-------------|
| Hardware watchdog | `ENABLE_WATCHDOG` | `false` | Auto-reboot if system hangs |

**What it does:**
- Monitors system responsiveness
- Reboots if no response for 10s
- Prevents permanent hangs

**config.h:**
```cpp
#define ENABLE_WATCHDOG false
#define WATCHDOG_TIMEOUT_S 10            // Timeout in seconds
```

**Testing:**
- Enable and verify no spontaneous reboots
- System should run stable for hours

---

## 🔧 Configuration Matrix

**Quick reference table:**

| Feature | Toggle | Hardware | I2C Address | Pin(s) |
|---------|--------|----------|-------------|--------|
| **Display Output** | `ENABLE_DISPLAY_OUTPUT` | None | - | GPIO 23 |
| **Bi-Directional** | `ENABLE_BIDIRECTIONAL` | None | - | - |
| **CSV Output** | `ENABLE_CSV_OUTPUT` | None | - | - |
| **Battery Monitor** | `ENABLE_BATTERY_MONITOR` | Voltage divider | - | GPIO 35 |
| **Audio Detector** | `ENABLE_AUDIO_DETECTION` | MAX4466 | - | GPIO 34 |
| **Light Detector** | `ENABLE_LIGHT_DETECTION` | TCS34725 | 0x29 | I2C (21/22) |
| **Current Monitor** | `ENABLE_CURRENT_MONITOR` | INA219 | 0x40 | I2C (21/22) |
| **Extended Telemetry** | `ENABLE_EXTENDED_TELEMETRY` | None | - | - |
| **Adaptive SF** | `ENABLE_ADAPTIVE_SF` | None | - | - |
| **Advanced Commands** | `ENABLE_ADVANCED_COMMANDS` | None | - | - |
| **Encryption** | `ENABLE_ENCRYPTION` | None | - | - |
| **Packet Stats** | `ENABLE_PACKET_STATS` | None | - | - |
| **Performance Mon.** | `ENABLE_PERFORMANCE_MONITOR` | None | - | - |
| **Runtime Config** | `ENABLE_RUNTIME_CONFIG` | None | - | - |
| **Watchdog** | `ENABLE_WATCHDOG` | None | - | - |

---

## 📝 Feature Recommendations

### Minimal Configuration (Default)
```cpp
#define ENABLE_DISPLAY_OUTPUT true       // ✅ Display station
#define ENABLE_BIDIRECTIONAL true        // ✅ ACK messages
#define ENABLE_CSV_OUTPUT true           // ✅ PC logging
// All others: false
```

**Use for:**
- Basic LoRa communication
- Display monitoring
- Data logging
- Learning the system

### Extended Configuration
```cpp
#define ENABLE_DISPLAY_OUTPUT true
#define ENABLE_BIDIRECTIONAL true
#define ENABLE_CSV_OUTPUT true
#define ENABLE_EXTENDED_TELEMETRY true   // ✅ System info
#define ENABLE_PACKET_STATS true         // ✅ Statistics
// Sensors: false (unless hardware present)
```

**Use for:**
- Advanced monitoring
- Performance analysis
- Detailed debugging

### Sensor Configuration
```cpp
#define ENABLE_DISPLAY_OUTPUT true
#define ENABLE_BIDIRECTIONAL true
#define ENABLE_BATTERY_MONITOR true      // ✅ If battery powered
#define ENABLE_CURRENT_MONITOR true      // ✅ For power analysis
#define ENABLE_EXTENDED_TELEMETRY true
// Audio/light: true if fire detection needed
```

**Use for:**
- Battery-powered deployments
- Fire alarm monitoring
- Power consumption optimization

---

## 🎯 Testing Priority

**Test in this order:**

1. ✅ **Core features** (LoRa, role detection) - REQUIRED
2. ✅ **Display output** (if using TFT display station)
3. ✅ **PC logging** (if analyzing data)
4. ⚙️ **Optional sensors** (if hardware present)
5. ⚙️ **System features** (as needed)

---

## 💡 Tips

- **Start minimal:** Test with all features `false` first
- **Enable one at a time:** Easier to debug
- **Check memory:** Enable `ENABLE_PERFORMANCE_MONITOR` to track
- **Save power:** Disable unused features
- **Read docs:** Each feature has detailed documentation

---

**Next:** See TESTING_CHECKLIST.md for step-by-step testing guide!

---

*Last updated: 2025-11-06*
