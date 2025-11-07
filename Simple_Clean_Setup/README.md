# Simple Clean Setup 🚀

**Get ESP32 UART display working in 5 minutes!**

This is the absolute simplest way to get a working robot → display communication system.

---

## 🎯 What You Get

- ✅ **Working UART communication** - Tested and verified
- ✅ **2 wires** - TX→RX and GND→GND
- ✅ **No configuration** - Upload and run
- ✅ **Clean code** - Easy to understand and modify
- ✅ **Visual display** - See your data in real-time

---

## 📦 Hardware Needed

1. **ESP32-2432S022** (Display device)
2. **Any ESP32** (Robot/sender)
3. **2 jumper wires** (for TX→RX and GND)
4. **2 USB cables** (for programming)

---

## 🔌 Wiring (Just 2 Wires!)

```
┌─────────────────┐           ┌──────────────────┐
│   Robot ESP32   │           │  Display ESP32   │
│                 │           │  (2432S022)      │
│                 │           │                  │
│  GPIO 23 (TX) ──┼──────────►│─ GPIO 18 (RX)   │
│  GND          ──┼───────────│─ GND             │
│                 │           │                  │
└─────────────────┘           └──────────────────┘
```

**That's it! Just 2 wires.**

---

## ⚡ 3-Step Setup

### Step 1: Upload Display Code

```bash
# In Arduino IDE:
1. Open: Simple_Clean_Setup/Display_Device/Display_Device.ino
2. Board: ESP32 Dev Module
3. Port: Select your ESP32-2432S022
4. Upload
```

**Display should show:** "SIMPLE DISPLAY" header + "Ready - Waiting for data..."

### Step 2: Upload Robot Code

```bash
# In Arduino IDE:
1. Open: Simple_Clean_Setup/Robot_Sender/Robot_Sender.ino
2. Board: ESP32 Dev Module
3. Port: Select your robot ESP32
4. Upload
```

**Serial Monitor should show:** "TX [1]: SEQ:1,LED:ON,..."

### Step 3: Connect Wires & Test

1. Connect 2 wires as shown in wiring diagram
2. Power on both devices
3. **Display should update every 2 seconds!**

✅ You should see:
- SEQ counter incrementing
- LED toggling ON/OFF
- TEMP, RSSI, SNR values
- Message count at bottom

**Done! 🎉**

---

## 🐛 Troubleshooting

### Display shows "NO SIGNAL"

**Check:**
1. ✅ Wiring: Robot GPIO 23 → Display GPIO 18
2. ✅ GND connected
3. ✅ Both devices powered on
4. ✅ Robot code uploaded successfully

**Fix:** Swap wires if needed. TX must go to RX!

### Display stays at "Waiting for data..."

**Check:**
1. Open Serial Monitor on robot (115200 baud)
2. Should see "TX [1]: ..." messages
3. If yes → wiring problem
4. If no → re-upload robot code

### Garbled text on display

**Check:**
- Both devices use 115200 baud (default)
- Re-upload both if needed

### Still not working?

1. Test display only:
   - Upload Display_Device.ino
   - Should show header + "Ready..."

2. Test robot only:
   - Upload Robot_Sender.ino
   - Open Serial Monitor
   - Should see "TX [1]: ..." every 2 seconds

3. If both work separately → wiring issue!

---

## 🔧 Customization

### Change send interval

In `Robot_Sender.ino`:
```cpp
if (millis() - lastSend >= 2000) {  // Change 2000 to your value (ms)
```

### Change TX pin (robot side)

In `Robot_Sender.ino`:
```cpp
#define UART_TX_PIN 23  // Change to any free GPIO
```

### Add your own data

In `Robot_Sender.ino`, modify the message:
```cpp
String msg = "";
msg += "SEQ:" + String(counter);
msg += ",LED:" + String(ledState ? "ON" : "OFF");
msg += ",YOUR_DATA:" + String(your_value);  // Add this!
msg += "\n";
```

### Change display colors

In `Display_Device.ino`:
```cpp
#define COLOR_BG 0x0000        // Background (black)
#define COLOR_HEADER 0x001F    // Header (blue)
#define COLOR_TEXT 0xFFFF      // Text (white)
#define COLOR_GOOD 0x07E0      // Good values (green)
#define COLOR_WARN 0xFD20      // Warnings (orange)
```

---

## 📖 Data Format

The system uses simple CSV format:

```
KEY:VALUE,KEY2:VALUE2,KEY3:VALUE3
```

**Examples:**
```
SEQ:1,LED:ON,TEMP:25
SEQ:2,LED:OFF,TEMP:26,RSSI:-78,SNR:8
Counter:42,Voltage:3.7,Status:OK
```

**Rules:**
- Max 256 characters per message
- Max 10 key-value pairs per message
- Keys and values: alphanumeric + basic symbols
- End with `\n` (newline)

---

## 🎓 How It Works

### Display Device (ESP32-2432S022)

1. **Initializes TFT display** (320x240 landscape)
2. **Opens UART on GPIO 18** (RX pin)
3. **Waits for data** (115200 baud)
4. **Parses CSV format** (KEY:VALUE pairs)
5. **Displays on screen** (color-coded)

### Robot Sender (Any ESP32)

1. **Opens UART on GPIO 23** (TX pin)
2. **Builds CSV message** (demo data)
3. **Sends every 2 seconds** (via UART)
4. **Increments counter** (sequence tracking)

### Communication

```
Robot (TX) ──UART──> Display (RX)
    |                     |
  GPIO 23              GPIO 18
    |                     |
  115200 baud         115200 baud
    |                     |
   CSV data ────────> Parse & Display
```

---

## 🚀 Next Steps

**System working? Great!** Now you can:

### 1. Integrate with Your Robot

Replace the demo data in `Robot_Sender.ino` with your actual robot data:

```cpp
// Instead of demo data:
msg += ",TEMP:" + String(random(20, 30));

// Use real sensor data:
msg += ",TEMP:" + String(sensor.readTemperature());
```

### 2. Add More Data

Add any sensors or values you want to display:
```cpp
msg += ",Battery:" + String(batteryVoltage);
msg += ",Speed:" + String(motorSpeed);
msg += ",Distance:" + String(ultrasonic);
```

### 3. Use DisplayClient Library

For easier integration, use the DisplayClient library (in `Roboter_Display_TFT/DisplayClient.h`):

```cpp
#include "DisplayClient.h"
DisplayClient display(23);

void setup() {
  display.begin();
}

void loop() {
  display.set("TEMP", 25);
  display.set("LED", "ON");
  display.send();
}
```

### 4. Explore Full Project

Check out the full robot project in `Roboter_Gruppe_9/`:
- LoRa communication
- Multiple sensors
- Advanced features
- Comprehensive documentation

---

## 📊 Project Structure

```
Simple_Clean_Setup/
├── README.md                    ← You are here
├── Display_Device/
│   └── Display_Device.ino       ← Upload to ESP32-2432S022
└── Robot_Sender/
    └── Robot_Sender.ino         ← Upload to robot ESP32
```

**Dependencies:**
- LovyanGFX library (for display)
- Arduino ESP32 core

**Install LovyanGFX:**
1. Arduino IDE → Library Manager
2. Search "LovyanGFX"
3. Install

---

## ✅ Success Checklist

Your system is working if:

- ✅ Display shows "SIMPLE DISPLAY" header
- ✅ Data updates every 2 seconds
- ✅ SEQ counter increments (1, 2, 3, ...)
- ✅ LED toggles ON/OFF
- ✅ Message count increases
- ✅ No "NO SIGNAL" message

**Congratulations! Your display system is working!** 🎉

---

## 🆘 Need More Help?

**Quick tests:**
- See `Test_Display_Incremental/` for step-by-step debugging
- See `Test_Display_Minimal/` for minimal display test
- See `Roboter_Display_TFT/README.md` for full display documentation

**Full project:**
- See `Roboter_Gruppe_9/QUICK_START.md` for complete robot setup
- See `Roboter_Gruppe_9/TESTING_CHECKLIST.md` for comprehensive testing

---

## 💡 Why This Works

This setup is based on extensive testing (see git history):

1. ✅ **Test_Display_Minimal** - Verified display hardware works
2. ✅ **Test_Display_TFT_Only** - Verified TFT library works
3. ✅ **Test_Display_Incremental** - Verified UART initialization works
4. ✅ **Simple_Clean_Setup** - Combined everything that works!

All code is tested and verified to work. If you follow the steps exactly, it will work.

---

## 🔗 Related Documentation

- **Quick Start:** `Roboter_Gruppe_9/QUICK_START.md`
- **Testing:** `Roboter_Gruppe_9/TESTING_CHECKLIST.md`
- **Display Library:** `Roboter_Display_TFT/README.md`
- **Project Structure:** `Roboter_Gruppe_9/PROJECT_STRUCTURE.md`

---

**Happy building! 🚀**

*This setup has been tested and verified to work on 2025-11-07.*
