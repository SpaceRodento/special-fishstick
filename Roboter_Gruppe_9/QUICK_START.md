# Quick Start Guide - Roboter Gruppe 9

**Get your LoRa system running in 10 minutes!**

Last updated: 2025-11-06

---

## 🚀 What You Need

**Hardware:**
- 2× ESP32 boards
- 2× RYLR896 LoRa modules (868 MHz)
- 1× I2C LCD 16x2 (optional, for receiver)
- 1× ESP32-2432S022 TFT display (optional, for display station)
- Jumper wires
- USB cables

**Software:**
- Arduino IDE or PlatformIO
- `LiquidCrystal_I2C` library (for LCD)

---

## ⚡ 5-Step Setup

### Step 1: Connect LoRa Modules

**Both ESP32 boards:**
```
RYLR896 → ESP32
─────────────────
TX      → GPIO 25
RX      → GPIO 26
VCC     → 3.3V
GND     → GND
```

### Step 2: Upload Code

1. Open `Roboter_Gruppe_9.ino`
2. Select board: **ESP32 Dev Module**
3. Upload to **BOTH** ESP32 boards
4. **Same code for both!** (role auto-detected)

### Step 3: Configure Roles

**Receiver (with LCD):**
- Connect **GPIO 16 ↔ GPIO 17** with jumper wire
- Connect I2C LCD (SDA=21, SCL=22)

**Sender:**
- Leave **GPIO 16 floating** (no connection)
- No LCD needed

### Step 4: Power On & Test

1. Power both ESP32 boards
2. Open Serial Monitor (115200 baud) on both
3. Check output:

**Sender:**
```
>>> SENDER MODE
✓ LoRa initialized
📤 TX [1]: SEQ:0,LED:1,...
```

**Receiver:**
```
>>> RECEIVER MODE
✓ LoRa initialized
✓ LCD initialized
📥 RX [1]: SEQ:0,LED:1,...
RSSI: -78 dBm, SNR: 7 dB
```

### Step 5: Verify

- ✅ Sender sends messages every 2 seconds
- ✅ Receiver displays on LCD
- ✅ RSSI/SNR values shown

**Done!** Your LoRa system is working!

---

## 🖥️ Optional: Add TFT Display Station

**Want a nice TFT display for real-time monitoring?**

### Hardware Setup

**ESP32-2432S022 TFT Display:**
- Upload `../Roboter_Display_TFT/Roboter_Display_TFT.ino`
- Connect 2 wires:
  ```
  Robot GPIO 23 (TX) → Display GPIO 18 (RX)
  Robot GND          → Display GND
  ```

### Enable Display Output

**In `config.h`:**
```cpp
#define ENABLE_DISPLAY_OUTPUT true
```

Re-upload code to robot ESP32.

### Verify

Display should show:
- Header: "ROBOTER 9" + connection status
- Data: Mode, SEQ, LED, RSSI, SNR, etc.
- Updates every 2 seconds

**Troubleshooting:**
- No data? Check wiring (GPIO 23 → GPIO 18, GND → GND)
- Wrong data? Verify 115200 baud on both devices
- See TESTING_CHECKLIST.md section "Display not receiving data"

---

## 🎛️ Configuration

**All settings in `config.h`:**

```cpp
// Feature toggles (true/false)
#define ENABLE_DISPLAY_OUTPUT true     // TFT display station
#define ENABLE_BIDIRECTIONAL true      // Two-way communication (ACK)
#define ENABLE_CSV_OUTPUT true         // PC data logging

// Timing
#define DISPLAY_UPDATE_INTERVAL 2000   // Send to display every 2s

// Optional sensors (disable if not used)
#define ENABLE_BATTERY_MONITOR false
#define ENABLE_AUDIO_DETECTION false
#define ENABLE_LIGHT_DETECTION false
#define ENABLE_CURRENT_MONITOR false
```

---

## 🐛 Troubleshooting

### No LoRa communication

**Check:**
1. LoRa module powered (3.3V, GND)
2. Wiring: TX→25, RX→26
3. Both devices: Same `LORA_NETWORK_ID` (default: 6)
4. Serial shows: `✓ LoRa initialized`

**Fix:**
- Power cycle LoRa modules
- Check solder connections
- Try different USB power source

### Wrong role detected

**Problem:** Sender acts as receiver or vice versa

**Check:**
- Receiver: GPIO 16 ↔ GPIO 17 **connected**
- Sender: GPIO 16 **floating** (no connection)

**Fix:**
- Add/remove jumper wire
- Power cycle ESP32

### Display not working (TFT)

**Check:**
1. Display code uploaded (`Roboter_Display_TFT.ino`)
2. Wiring: Robot GPIO 23 → Display GPIO 18
3. Common GND connected
4. config.h: `ENABLE_DISPLAY_OUTPUT true`

**Fix:**
- Check serial monitors (both robot and display)
- Robot should show: `→ Display: MODE:...`
- Display should show: `📥 RX [1]: ...`
- Swap wires if needed (TX must go to RX!)

---

## 📊 What's Next?

✅ **System working?** Great! Now you can:

### Test More Features

- **Kill-switch:** Connect GPIO 13↔14, hold 3s = restart
- **PC logging:** `python serial_monitor.py /dev/ttyUSB0`
- **ACK messages:** Already enabled by default!

### Enable Optional Features

Edit `config.h` and set to `true`:
- `ENABLE_BATTERY_MONITOR` - Battery voltage monitoring
- `ENABLE_EXTENDED_TELEMETRY` - Uptime, memory, temperature
- `ENABLE_PACKET_STATS` - Detailed packet statistics

### Range Testing

- **Close (0-10m):** RSSI > -70 dBm, packet loss < 1%
- **Medium (10-100m):** RSSI -70 to -90 dBm
- **Long (100m+):** RSSI < -90 dBm, may need line-of-sight

### Advanced

- **Change LCD version:** See 4 versions in `Roboter_Gruppe_9.ino`
- **Adjust send interval:** Change `DISPLAY_UPDATE_INTERVAL`
- **Custom commands:** Enable `ENABLE_ADVANCED_COMMANDS`

---

## 📖 Documentation

**Quick references:**
- **TESTING_CHECKLIST.md** - Step-by-step testing guide
- **PROJECT_STRUCTURE.md** - Complete file organization
- **COMPREHENSIVE_MANUAL.md** - Detailed technical manual

**For specific topics:**
- **PC_LOGGING_README.md** - Python data logging
- **FEATURE_TESTING_GUIDE.md** - Testing individual features

---

## 🎓 Learning Path

**New to the project? Read in this order:**

1. ✅ QUICK_START.md (this file) - Get it running
2. 📋 TESTING_CHECKLIST.md - Verify it works
3. 🏗️ PROJECT_STRUCTURE.md - Understand the architecture
4. 🔧 config.h - See what's configurable
5. 📖 COMPREHENSIVE_MANUAL.md - Deep dive

---

## 💡 Pro Tips

1. **Test minimal config first**
   - Disable all optional features
   - Get LoRa working
   - Then enable features one by one

2. **Use serial monitors**
   - Essential for debugging
   - Shows RSSI, SNR, errors
   - Both sender and receiver

3. **Check connections**
   - Most issues are wiring
   - Verify with multimeter if needed
   - Common GND is critical!

4. **Save power**
   - Disable unused features
   - Increase send interval
   - Lower LoRa TX power (if needed)

---

## 🎯 Success Checklist

**Your system is working if:**

- ✅ Serial shows messages flowing
- ✅ RSSI values reasonable (-50 to -120 dBm)
- ✅ Receiver LCD updates
- ✅ Display station shows data (if enabled)
- ✅ No error messages in serial
- ✅ Kill-switch restarts device

**Congratulations!** 🎉

---

## 🆘 Need Help?

**Common issues:**
1. LoRa not working → Check power and wiring
2. Wrong role → Check GPIO 16↔17 jumper
3. Display blank → Check TX→RX wiring and GND
4. LCD garbage → Check I2C address (0x27 or 0x3F)

**Still stuck?**
- Read TESTING_CHECKLIST.md "Common Issues" section
- Check serial monitors for error messages
- Test with minimal configuration
- Verify hardware connections

---

**Happy building! 🚀**

*Last updated: 2025-11-06*
