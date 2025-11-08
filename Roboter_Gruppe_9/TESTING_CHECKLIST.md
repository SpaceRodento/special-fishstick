# Testing Checklist - Roboter Gruppe 9

**Quick testing guide - What to test and how**

Use this checklist to verify all functionality works correctly.

Last updated: 2025-11-06

---

## 📋 How to Use This Checklist

- ✅ = Tested and working
- ❌ = Tested but not working
- ⏭️ = Skipped (feature disabled or not needed)
- 🔲 = Not tested yet

**Test priority:**
1. Core functionality (LoRa, role detection)
2. Display features (LCD, TFT display station)
3. Optional features (as needed)

---

## 🎯 Core Functionality (REQUIRED)

### 1. Hardware Setup

- [ ] **LoRa module connected correctly**
  - RYLR896 TX → ESP32 GPIO 25
  - RYLR896 RX → ESP32 GPIO 26
  - Power: 3.3V, GND

- [ ] **Role detection wiring**
  - RECEIVER: GPIO 16 ↔ GPIO 17 (jumper wire)
  - SENDER: GPIO 16 floating (no connection)

### 2. Code Upload

- [ ] **Code compiles without errors**
  - Open `Roboter_Gruppe_9.ino`
  - Select board: ESP32 Dev Module
  - Compile (Ctrl+R / Cmd+R)

- [ ] **Code uploads successfully**
  - Upload to both ESP32 devices
  - Same code for both!

### 3. Serial Monitor Check

- [ ] **SENDER boots correctly**
  ```
  >>> SENDER MODE
  ✓ LoRa initialized
  ✓ Setup complete!
  ```

- [ ] **RECEIVER boots correctly**
  ```
  >>> RECEIVER MODE
  ✓ LoRa initialized
  ✓ LCD initialized
  ✓ Setup complete!
  ```

### 4. LoRa Communication

- [ ] **Sender transmits messages**
  - Serial: `📤 TX [1]: SEQ:0,LED:1,...`
  - Every 2 seconds
  - Message count increases

- [ ] **Receiver gets messages**
  - Serial: `📥 RX [1]: SEQ:0,LED:1,...`
  - RSSI and SNR values shown
  - Message count increases

- [ ] **RSSI/SNR values look reasonable**
  - RSSI: -50 to -120 dBm (closer = better)
  - SNR: -20 to +10 dB (higher = better)

### 5. Basic Features

- [ ] **LED blinking**
  - GPIO 2 LED blinks every 500ms
  - Both sender and receiver

- [ ] **Touch sensor working**
  - Touch GPIO 4 (T0)
  - Serial shows: `Touch: YES` or `touchState: 1`

- [ ] **Kill-switch works**
  - Connect GPIO 13 ↔ GPIO 14
  - Hold 3 seconds
  - ESP32 restarts

---

## 🖥️ Display Features

### 6. I2C LCD Display (Receiver only)

- [ ] **LCD powers on**
  - Backlight visible
  - Text displayed

- [ ] **LCD shows connection status**
  - "ONLINE" when receiving messages
  - "NO DATA" when no messages

- [ ] **LCD shows RSSI**
  - Signal bars or RSSI number
  - Updates when new message arrives

- [ ] **LCD shows message count**
  - Counter increases
  - PKT:X or similar

- [ ] **Spinner animation works**
  - Rotating symbol: |/-\
  - Updates regularly

**Test different LCD versions (optional):**
- [ ] Version 1: Wide bar + RSSI (`updateLCD_Version1_WideBar`)
- [ ] Version 2: Compact (`updateLCD_Version2_Compact`)
- [ ] Version 3: Detailed info (`updateLCD_Version3_Detailed`)
- [ ] Version 4: Original (`updateLCD_Version4_Original`)

### 7. TFT Display Station (Optional)

**Hardware:**
- Separate ESP32-2432S022 TFT display
- Robot GPIO 23 (TX) → Display GPIO 18 (RX)
- GND → GND

**config.h setting:**
```cpp
#define ENABLE_DISPLAY_OUTPUT true
```

- [ ] **Display station code uploaded**
  - Upload `Roboter_Display_TFT.ino` to display ESP32
  - Serial shows: "Display station ready!"

- [ ] **Wiring verified**
  - Robot GPIO 23 → Display GPIO 18
  - Common GND connected

- [ ] **Display receives data**
  - Display serial: `📥 RX [1]: MODE:SENDER,SEQ:0,...`
  - Data updates every 2 seconds

- [ ] **TFT shows data**
  - Header: "ROBOTER 9" + connection status
  - Data fields: Mode, SEQ, LED, RSSI, etc.
  - Values update in real-time

- [ ] **Display shows alerts (if any)**
  - Red bar at bottom for alerts
  - Blinking text

**Troubleshooting if display not working:**
- [ ] Check GPIO 23 is actually transmitting (robot serial: "→ Display: MODE:...")
- [ ] Check display is listening on GPIO 18 (display serial: "Waiting for data on UART...")
- [ ] Verify 115200 baud on both sides
- [ ] Check common GND connection
- [ ] Try swapping TX/RX wires (TX must go to RX!)

---

## 💻 PC Data Logging (Optional)

**config.h setting:**
```cpp
#define ENABLE_CSV_OUTPUT true
```

### 8. CSV Output

- [ ] **CSV data appears in serial**
  - Format: `DATA_CSV,timestamp,role,rssi,snr,...`
  - Every 2 seconds (configurable)

- [ ] **Python serial monitor works**
  ```bash
  python serial_monitor.py /dev/ttyUSB0 115200
  ```
  - Colored output
  - RSSI bars visible

- [ ] **Data logger captures to database**
  ```bash
  python data_logger.py /dev/ttyUSB0 115200 test.db
  ```
  - Creates `test.db` file
  - Check: `sqlite3 test.db "SELECT COUNT(*) FROM lora_messages;"`

---

## 🔄 Bi-Directional Communication (Optional)

**config.h setting:**
```cpp
#define ENABLE_BIDIRECTIONAL true
#define ACK_INTERVAL 5
```

### 9. ACK Messages

- [ ] **Receiver sends ACK**
  - Every 5th message (configurable)
  - Receiver serial: `📤 Sending ACK (#5)...`

- [ ] **Sender receives ACK**
  - Sender serial: `✓ ACK #1 received (RSSI: -78 dBm)`
  - ACK count increases

- [ ] **ACK contains data**
  - ACK includes receiver's state
  - Sender can see receiver's LED/touch status

---

## 🔌 Optional Sensor Features

Enable in `config.h` as needed. Test only if you have the hardware!

### 10. Battery Monitor

**Hardware:** Voltage divider on GPIO 35

```cpp
#define ENABLE_BATTERY_MONITOR true
```

- [ ] **Battery voltage displayed**
  - Serial: `🔋 Battery: 3.85V`
  - Every 60 seconds

- [ ] **Low battery warning**
  - Warning at <3.3V
  - Critical at <3.0V

### 11. Audio Detector (Fire Alarm)

**Hardware:** MAX4466 microphone on GPIO 34

```cpp
#define ENABLE_AUDIO_DETECTION true
```

- [ ] **Audio RMS measured**
  - Serial: `🔊 Audio RMS: 150`

- [ ] **Fire alarm detected**
  - Test with 3kHz tone (3-4 beeps/second)
  - Serial: `🚨 FIRE ALARM DETECTED (audio)!`

### 12. Light Detector (Fire Alarm)

**Hardware:** TCS34725 RGB sensor on I2C

```cpp
#define ENABLE_LIGHT_DETECTION true
```

- [ ] **Red light measured**
  - Serial: `💡 Red: 450`

- [ ] **Flashing red LED detected**
  - Test with red LED flashlight (1 Hz)
  - Serial: `🚨 FIRE ALARM DETECTED (light)!`

### 13. Current Monitor (Power)

**Hardware:** INA219 current sensor on I2C

```cpp
#define ENABLE_CURRENT_MONITOR true
```

- [ ] **Current/voltage measured**
  - Serial: `⚡ 3.85V, 85mA, 328mW`

- [ ] **Energy tracking**
  - Serial: `🔋 Energy: 12.5 mAh`

- [ ] **High current warning**
  - Warning at >200mA
  - Critical at >500mA

---

## 🎛️ System Features (Optional)

### 14. Extended Telemetry

```cpp
#define ENABLE_EXTENDED_TELEMETRY true
```

- [ ] **Uptime shown**
  - Serial or display: Uptime in seconds

- [ ] **Free heap shown**
  - Memory available (KB)

- [ ] **Internal temperature**
  - ESP32 internal sensor (°C)

### 15. Adaptive Spreading Factor

```cpp
#define ENABLE_ADAPTIVE_SF true
```

- [ ] **SF changes based on RSSI**
  - Good signal (>-80dBm): Lower SF (faster)
  - Weak signal (<-105dBm): Higher SF (slower)
  - Serial: `📡 SF adjusted: 12 → 10`

### 16. Packet Statistics

```cpp
#define ENABLE_PACKET_STATS true
```

- [ ] **Detailed stats shown**
  - Retries, duplicates, out-of-order packets
  - Serial: Every 30 seconds

### 17. Performance Monitor

```cpp
#define ENABLE_PERFORMANCE_MONITOR true
```

- [ ] **Loop frequency shown**
  - Serial: `⚙️ Loop: 120 Hz`

- [ ] **CPU usage shown**
  - Percentage or load

### 18. Watchdog Timer

```cpp
#define ENABLE_WATCHDOG true
```

- [ ] **Watchdog initializes**
  - Serial: `🐕 Watchdog enabled (10s)`

- [ ] **System doesn't restart unexpectedly**
  - Run for 5+ minutes
  - No spontaneous reboots

---

## 🏃 Range Testing

### 19. Close Range (0-10m)

- [ ] **RSSI > -70 dBm**
- [ ] **Packet loss < 1%**
- [ ] **Connection: CONNECTED**

### 20. Medium Range (10-100m)

- [ ] **RSSI -70 to -90 dBm**
- [ ] **Packet loss < 5%**
- [ ] **Connection: CONNECTED or WEAK**

### 21. Long Range (100m+)

- [ ] **RSSI < -90 dBm**
- [ ] **Packet loss 5-20%**
- [ ] **Connection: WEAK**
- [ ] **Auto-recovery works if lost**

### 22. Through Obstacles

- [ ] **Through walls**
  - RSSI drops ~20-30 dBm
  - Communication still works

- [ ] **Through metal**
  - Significant signal loss
  - May lose connection

---

## 🔴 Error Handling

### 23. LoRa Module Disconnected

- [ ] **Graceful failure at boot**
  - Serial: `❌ LoRa init failed!`
  - Serial: `⚠️ Continuing anyway...`
  - Does NOT freeze in while(1)

- [ ] **Kill-switch still works**
  - Even without LoRa
  - GPIO 13↔14 hold 3s = restart

### 24. Connection Loss

- [ ] **Sender powered off**
  - Receiver detects LOST after 8s
  - Serial: `Connection state: LOST`
  - LCD shows "NO DATA"

- [ ] **Auto-recovery attempts**
  - 3 attempts, 5s apart
  - Serial: `📡 Recovery attempt 1/3`

- [ ] **Recovery succeeds**
  - When sender powered back on
  - Connection returns to CONNECTED

---

## ⚡ Performance Testing

### 25. Stability Test

- [ ] **1 hour run**
  - No crashes or reboots
  - Packet loss stable
  - Memory doesn't decrease

- [ ] **Variable send interval**
  - Test: 500ms, 1000ms, 2000ms, 5000ms
  - Adjust `DISPLAY_UPDATE_INTERVAL` in config.h

---

## 📝 Summary

**When all core tests (1-5) pass:**
✅ Basic LoRa communication works!
✅ Role detection works!
✅ System is functional!

**Optional features to test based on your needs:**
- Display features (6-7): If you have LCD or TFT display
- PC logging (8): If you want to analyze data
- Bi-directional (9): If you need ACK/two-way
- Sensors (10-13): If you have additional hardware
- System features (14-18): For advanced functionality

---

## 🐛 Common Issues & Fixes

### Display not receiving data

**Symptoms:** Display shows "Waiting for data..." but never updates

**Checks:**
1. [ ] Robot serial shows: `→ Display: MODE:SENDER,...` (data being sent)
2. [ ] Display serial shows: `📥 RX [1]: ...` (data received)
3. [ ] Wiring: Robot GPIO 23 → Display GPIO 18
4. [ ] Common GND connected
5. [ ] Both use 115200 baud
6. [ ] `ENABLE_DISPLAY_OUTPUT true` in robot's config.h

**Fix:**
- Check `config.h`: Verify `DISPLAY_TX_PIN 23`
- Verify TX wire is connected to RX (not TX to TX!)
- Try adding delay(100) before sending first message

### LoRa not communicating

**Symptoms:** No messages received, RSSI stays at 0

**Checks:**
1. [ ] LoRa module powered (3.3V)
2. [ ] Wiring correct (TX→25, RX→26)
3. [ ] Both devices use same NETWORK_ID
4. [ ] Serial shows `✓ LoRa initialized`

**Fix:**
- Power cycle LoRa module
- Check solder connections
- Verify module is RYLR896 (not RYLR406/998)

### LCD shows garbage characters

**Symptoms:** Random characters on LCD

**Checks:**
1. [ ] I2C address correct (try 0x27 or 0x3F)
2. [ ] SDA/SCL pins correct (GPIO 21/22)
3. [ ] Backlight contrast adjusted

**Fix:**
- Run I2C scanner to find correct address
- Adjust potentiometer on LCD backpack

---

## 📋 Quick Test Sequence

**Minimal 5-minute test:**

1. ✅ Upload code to both ESP32
2. ✅ Add jumper to receiver (GPIO 16↔17)
3. ✅ Power on both devices
4. ✅ Check serial monitors
5. ✅ Verify messages flowing
6. ✅ Check RSSI values
7. ✅ Test kill-switch (GPIO 13↔14, hold 3s)

**Done!** Core functionality verified.

---

**Pro tip:** Test with all features DISABLED first (`false` in config.h), then enable one at a time to isolate issues!

---

**Next steps:**
- ✅ Core working → Enable optional features as needed
- ✅ Display issues → Check TESTING_CHECKLIST section "Display not receiving data"
- ✅ Want more details → Read COMPREHENSIVE_MANUAL.md

---

*Last updated: 2025-11-06*
