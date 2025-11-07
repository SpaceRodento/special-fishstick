# Display Not Working - Emergency Troubleshooting

**Symptoms:**
- Display is WHITE (or blank)
- Serial Monitor shows NOTHING (no output at all)
- Robot sends TX: HELLO X (robot works fine)

**This means:** Display code is NOT running or crashing immediately!

---

## 🚨 STEP 1: Verify Arduino IDE Settings

**Board settings for ESP32-2432S022:**

```
Tools Menu:
─────────────────────────────────
Board: "ESP32 Dev Module"
Upload Speed: "115200"
CPU Frequency: "240MHz (WiFi/BT)"
Flash Frequency: "80MHz"
Flash Mode: "QIO"
Flash Size: "4MB (32Mb)"
Partition Scheme: "Default 4MB with spdy (1.2MB APP/1.5MB SPIFFS)"
Core Debug Level: "None"
PSRAM: "Disabled"
```

**CRITICAL SETTINGS:**
- Board MUST be "ESP32 Dev Module" (not ESP32-S2, not ESP32-C3!)
- Flash Size: 4MB
- Partition Scheme: Default 4MB

**Wrong board = Code won't run!**

---

## 🚨 STEP 2: Test Minimal Code

### Test A: Serial Only (NO TFT, NO UART)

**Upload `Test_Display_Minimal.ino` to display ESP32**

1. Open Test_Display_Minimal.ino
2. Select correct COM port (unplug/replug USB to find it)
3. Upload
4. Open Serial Monitor (115200 baud)
5. Press RESET button on ESP32-2432S022

**Expected:**
```
===================
MINIMAL TEST START
===================
If you see this, Serial works!

Counter: 1
Counter: 2
Counter: 3
...
```

**If you see this:** ✅ Arduino IDE settings are correct!
**If you see NOTHING:** ❌ Go to "STEP 3: Arduino IDE Problem"

---

### Test B: TFT Only (NO UART)

**Upload `Test_Display_TFT_Only.ino` to display ESP32**

1. Upload code
2. Check Serial Monitor
3. Check display screen

**Expected Serial:**
```
=== TFT TEST START ===
Backlight ON
Initializing TFT...
TFT init OK
Rotation set
Screen filled black
Text color set
Text drawn

=== TFT TEST COMPLETE ===
Check display - should show 'TFT TEST'

Counter: 1
Counter: 2
...
```

**Expected Display:**
- Screen turns BLACK (not white!)
- White text: "TFT TEST"
- White text: "If you see this, TFT works!"
- Green text: "Count: 1", "Count: 2", etc.

**Results:**

| What you see | Meaning | Solution |
|--------------|---------|----------|
| ✅ Serial OK, Display shows text | TFT works! | Go to STEP 4 (UART test) |
| ⚠️ Serial OK, Display stays WHITE | TFT not initializing | Check LovyanGFX library |
| ❌ No serial output at all | Code not running | Go to STEP 3 |
| 🔴 Serial shows error/crash | Code crashes | Check error message |

---

## 🚨 STEP 3: Arduino IDE Problem

**If Serial Monitor shows NOTHING (even with minimal code):**

### A) Wrong COM Port

1. Close Serial Monitor
2. Disconnect display USB
3. Note which COM ports are in Tools > Port menu
4. Reconnect display USB
5. New COM port appears → That's the display!
6. Select that COM port
7. Upload code again

### B) Wrong Board Selected

**Display ESP32-2432S022 uses standard ESP32 chip!**

```
✅ CORRECT: "ESP32 Dev Module"
❌ WRONG: "ESP32-S2"
❌ WRONG: "ESP32-C3"
❌ WRONG: "ESP32-S3"
```

**Fix:** Tools > Board > ESP32 Arduino > ESP32 Dev Module

### C) Bootloader Issue

**Try entering bootloader mode manually:**

1. Hold BOOT button on ESP32-2432S022
2. While holding BOOT, press RESET button
3. Release RESET
4. Release BOOT
5. Click Upload in Arduino IDE

**Display should show "Connecting..." then upload**

### D) USB Cable Problem

**Some USB cables are POWER ONLY (no data)!**

**Test:**
- Try different USB cable
- Use cable that came with ESP32 (if you have it)
- Cable must support DATA, not just charging

### E) ESP32 Board Package

**Make sure ESP32 boards are installed:**

1. File > Preferences
2. Additional Board Manager URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Tools > Board > Boards Manager
4. Search "esp32"
5. Install "esp32 by Espressif Systems" (latest version)
6. Restart Arduino IDE

---

## 🚨 STEP 4: UART Test (After Serial & TFT work)

**Only do this AFTER Test A and B work!**

### Upload Test_Display_UART_Simple.ino

**This tests UART reception only (no TFT drawing)**

**Expected Serial:**
```
╔════════════════════════════════════════╗
║  UART TEST - Display ESP32           ║
╚════════════════════════════════════════╝

✅ UART initialized
  RX Pin: GPIO 18
  Baudrate: 115200

📡 Waiting for data...

📥 RX: HELLO 1
📥 RX: HELLO 2
📥 RX: HELLO 3
...
```

**If you see "📥 RX: HELLO X":** ✅ UART works!

**If you see "Waiting for data..." but no RX:**
- Check wiring: Robot GPIO 23 → Display GPIO 18
- Check GND connection
- Verify robot is sending (check robot serial: "📤 TX: HELLO 1")

---

## 🚨 STEP 5: Full Display Code

**Only after ALL previous tests pass!**

Now try `Roboter_Display_TFT.ino`:

1. Make sure Test A, B, and 4 all passed
2. Upload full code
3. Check Serial Monitor
4. Check display

**If it crashes now but tests worked:**
- Code is too complex / uses too much memory
- Check for `#include` errors
- Make sure `display_config.h` is in same folder

---

## 📊 Troubleshooting Decision Tree

```
Serial Monitor shows NOTHING?
│
├─ YES → STEP 3: Arduino IDE Problem
│         - Wrong COM port
│         - Wrong board
│         - USB cable (data vs power-only)
│         - ESP32 package not installed
│
└─ NO (Serial works) → Test Minimal.ino passed!
    │
    └─ Display stays WHITE?
       │
       ├─ YES → TFT not initializing
       │         - Test TFT_Only.ino
       │         - Check LovyanGFX library installed
       │         - Check board settings
       │
       └─ NO (Display shows text) → TFT works!
           │
           └─ UART not receiving data?
               │
               ├─ Check wiring (GPIO 23 → GPIO 18)
               ├─ Check GND connection
               ├─ Check robot is sending (TX: HELLO X)
               └─ Try Test_Display_UART_Simple.ino
```

---

## 🔧 Library Check

**Make sure you have LovyanGFX library:**

1. Sketch > Include Library > Manage Libraries
2. Search "LovyanGFX"
3. Install "LovyanGFX" by lovyan03
4. Restart Arduino IDE

**If library is installed but TFT still doesn't work:**
- Update to latest version
- Try older version (sometimes newer versions have bugs)

---

## 💡 Common Issues

### Issue 1: Serial Monitor blank after upload

**Symptoms:** Upload succeeds, but Serial Monitor completely blank

**Causes:**
- Wrong COM port selected
- Serial Monitor not set to 115200 baud
- Need to press RESET button after upload

**Fix:**
1. Check COM port is correct
2. Set baud rate to 115200
3. Press RESET button on ESP32
4. If still blank → Wrong board selected!

### Issue 2: White screen, no Serial output

**Symptoms:** Display is white, Serial Monitor completely blank

**Causes:**
- Code not running at all
- ESP32 in bootloader mode (stuck)
- Wrong board / partition scheme

**Fix:**
1. Press RESET button
2. Verify board is "ESP32 Dev Module"
3. Re-upload code
4. Try Test_Display_Minimal.ino

### Issue 3: Serial works but TFT stays white

**Symptoms:** Serial shows output, but display stays white

**Causes:**
- LovyanGFX library not installed
- TFT initialization failing
- Wrong pin configuration

**Fix:**
1. Install LovyanGFX library
2. Test with Test_Display_TFT_Only.ino
3. Check Serial for error messages during tft.init()

### Issue 4: TFT works but UART doesn't receive

**Symptoms:** Display shows graphics, but no data from robot

**Causes:**
- Wiring wrong (GPIO 23 ↔ GPIO 18)
- GND not connected
- Robot not sending
- Wrong UART port (should be UART1, not UART0)

**Fix:**
1. Verify robot Serial shows "📤 TX: HELLO X"
2. Check wiring with multimeter
3. Verify GPIO 23 → GPIO 18 (crossover TX → RX!)
4. Connect GND between devices

---

## 📋 Report Template

**If still not working, provide these details:**

```
1. ARDUINO IDE SETTINGS:
   Board: _______________
   Flash Size: ___________
   Partition Scheme: _____

2. TEST RESULTS:
   Test_Display_Minimal.ino:
   [ ] Serial output OK
   [ ] No serial output

   Test_Display_TFT_Only.ino:
   [ ] Serial output OK, TFT shows text
   [ ] Serial output OK, TFT stays white
   [ ] No serial output

   Test_Display_UART_Simple.ino:
   [ ] UART receives data
   [ ] UART doesn't receive data
   [ ] Serial output doesn't work

3. HARDWARE:
   Display powered by: [ ] USB  [ ] External 5V
   Robot powered by: [ ] USB  [ ] External 5V
   Wiring:
   - Robot GPIO 23 → Display GPIO ___
   - GND connected: [ ] Yes  [ ] No

4. SERIAL MONITOR OUTPUT:
   (Copy-paste here)

5. ERROR MESSAGES:
   (If any)
```

---

**Start with Test_Display_Minimal.ino!**

If that doesn't show anything in Serial Monitor, the problem is Arduino IDE settings, not your code!

---

*Last updated: 2025-11-06*
