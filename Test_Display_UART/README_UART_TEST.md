# UART Connection Test - Troubleshooting Guide

**Simple test to verify UART connection between Robot and Display ESP32**

Last updated: 2025-11-06

---

## 🎯 Purpose

Test if UART communication works between two ESP32 boards WITHOUT:
- LoRa code
- Display graphics code
- Any complex features

Just pure UART TX/RX testing!

---

## ⚠️ CRITICAL: Power Supply

**ESP32-2432S022 Display MUST be powered separately!**

```
❌ WRONG:
Robot ESP32 5V → Display 5V  (Does NOT work! Not enough current)

✅ CORRECT:
Display ESP32 → Own USB cable
                (OR)
              → Own 5V power supply (2A minimum)
```

**Why?**
- ESP32-2432S022 with TFT display draws ~300-500mA
- ESP32 USB port can only supply ~500mA total
- If display tries to draw power from robot, robot crashes!

---

## 🔌 Hardware Setup

### Required Hardware

- 2× ESP32 boards (robot + display)
- 2× USB cables (one for each ESP32!)
- 2× Jumper wires (TX-RX and GND)

### Connections

```
Robot ESP32              Display ESP32-2432S022
────────────              ──────────────────────
GPIO 23 (TX) ─────────────► GPIO 18 (RX)
GND          ─────────────── GND

USB cable                  USB cable
(to PC for power           (to PC or charger
 and debug)                 for power)
```

**IMPORTANT:**
- Display MUST have its own USB power!
- GND connection is signal reference, NOT power!
- TX goes to RX (crossover)

---

## 📝 Step-by-Step Testing

### Step 1: Upload Test Code

**Display ESP32:**
```
1. Open Test_Display_UART_Simple.ino
2. Select board: ESP32 Dev Module
3. Select correct COM port
4. Upload
```

**Robot ESP32:**
```
1. Open Test_Robot_TX_Simple.ino
2. Select board: ESP32 Dev Module
3. Select correct COM port
4. Upload
```

### Step 2: Check Display Serial Monitor

Open Serial Monitor on Display ESP32 (115200 baud):

**Expected output:**
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

**If you see this:** ✅ UART is working!

**If you don't see data:**
- ⚠️ Check next section "Troubleshooting"

### Step 3: Check Robot Serial Monitor

Open Serial Monitor on Robot ESP32 (115200 baud):

**Expected output:**
```
╔════════════════════════════════════════╗
║  UART TEST - Robot ESP32              ║
╚════════════════════════════════════════╝

✅ UART initialized
  TX Pin: GPIO 23
  Baudrate: 115200

📡 Sending test messages every 2 seconds...

📤 TX: HELLO 1
📤 TX: HELLO 2
📤 TX: HELLO 3
...
```

**If you see this:** ✅ Robot is transmitting!

---

## 🐛 Troubleshooting

### Problem 1: Display shows "Waiting for data..." but no messages

**Symptoms:**
- Display serial shows UART initialized
- But no "📥 RX:" messages appear
- Robot shows "📤 TX:" messages

**Possible causes:**

#### A) Wrong wiring
```bash
Check:
✅ Robot GPIO 23 → Display GPIO 18 (TX goes to RX!)
✅ GND → GND
❌ Not TX → TX or RX → RX (wrong!)
```

**Fix:** Use multimeter to verify continuity

#### B) Display not powered
```bash
Check:
✅ Display has own USB cable plugged in
✅ Display Serial Monitor shows output (proves it has power)
```

**Fix:** Connect display to its own USB power

#### C) Wrong COM port selected
```bash
Check:
✅ Display Serial Monitor is on correct COM port
```

**Fix:**
- Disconnect display USB
- Note which COM ports disappear in Arduino IDE
- Reconnect display USB
- Select the new COM port

#### D) Baudrate mismatch
```bash
Check:
✅ Both codes use 115200 baud
✅ Serial Monitor set to 115200 baud
```

**Fix:** Re-upload code if needed

#### E) GPIO 18 used by something else
```bash
Check:
✅ No other code using GPIO 18
✅ No jumper wires on GPIO 18 except our RX wire
```

**Fix:** Remove conflicting connections

### Problem 2: Display shows garbage characters

**Symptoms:**
- Display receives something, but looks like: "�����"
- Random characters

**Cause:** Baudrate mismatch

**Fix:**
```cpp
// Make sure BOTH codes have:
#define UART_BAUDRATE 115200

// And Serial Monitor is also 115200 baud
```

### Problem 3: Robot not transmitting

**Symptoms:**
- Robot serial shows nothing
- Or robot crashes/reboots

**Possible causes:**

#### A) GPIO 23 conflict
```bash
Check:
✅ No other code using GPIO 23
✅ GPIO 23 is not MODE_GND_PIN or similar
```

#### B) Insufficient power
```bash
Check:
✅ Robot has good USB cable
✅ USB port provides enough power
```

**Fix:** Try different USB port or powered USB hub

### Problem 4: Both ESP32s reboot randomly

**Symptoms:**
- ESP32 keeps restarting
- Brown-out detector triggered

**Cause:** Power supply issue

**Fix:**
```
✅ Use good quality USB cables
✅ Use powered USB hub
✅ Each ESP32 on separate USB port
✅ Display MUST have own power (not from robot!)
```

---

## ✅ Success Criteria

**Test passes if:**

1. ✅ Display serial shows: "📥 RX: HELLO 1", "📥 RX: HELLO 2", etc.
2. ✅ Messages appear every 2 seconds
3. ✅ Counter increments: 1, 2, 3, 4...
4. ✅ No garbage characters
5. ✅ No reboots or crashes

**If test passes:**
- ✅ UART connection is working!
- ✅ Hardware is correct!
- ✅ Now try full Roboter_Display_TFT.ino code

**If test fails:**
- ⚠️ Fix issues before trying full code
- ⚠️ Full code will also fail if basic UART doesn't work

---

## 🔬 Advanced Debugging

### Use Logic Analyzer or Oscilloscope

**If you have one:**
```
Probe GPIO 23 (robot TX):
- Should see pulses every 2 seconds
- Voltage: 0V (LOW) and 3.3V (HIGH)
- Pattern: UART data frames

Probe GPIO 18 (display RX):
- Should see same pulses as GPIO 23
- If not: wiring problem
```

### Check Voltage Levels

**With multimeter:**
```
Robot GPIO 23 (TX):
- Idle state: ~3.3V
- During transmission: pulses between 0V and 3.3V

Display GPIO 18 (RX):
- Should match GPIO 23 exactly
```

### Swap Roles

**Try reverse direction:**
```
1. Display TX (GPIO 19) → Robot RX (GPIO 22)
2. Modify code to send from display
3. If this works but other direction doesn't:
   → Robot TX or Display RX has hardware problem
```

---

## 📊 Test Results Table

Fill this out during testing:

| Test | Result | Notes |
|------|--------|-------|
| Display powers on (own USB) | ☐ Pass / ☐ Fail | |
| Display serial shows startup | ☐ Pass / ☐ Fail | |
| Robot serial shows startup | ☐ Pass / ☐ Fail | |
| Robot sends "TX: HELLO X" | ☐ Pass / ☐ Fail | |
| Display receives "RX: HELLO X" | ☐ Pass / ☐ Fail | |
| Counter increments correctly | ☐ Pass / ☐ Fail | |
| No garbage characters | ☐ Pass / ☐ Fail | |
| No reboots/crashes | ☐ Pass / ☐ Fail | |

---

## 🎯 Next Steps

### If test passes:
1. ✅ UART communication works!
2. Try full `Roboter_Display_TFT.ino` code
3. If that fails, problem is in display graphics code (not UART)

### If test fails:
1. ⚠️ Check power supply (display on own USB!)
2. ⚠️ Verify wiring with multimeter
3. ⚠️ Try different jumper wires
4. ⚠️ Try different GPIO pins (e.g., GPIO 22 instead of 18)
5. ⚠️ Check for hardware damage

---

## 💡 Common Mistakes

❌ **Mistake 1:** Display powered from robot
- Fix: Display needs own USB cable!

❌ **Mistake 2:** TX → TX and RX → RX (parallel)
- Fix: TX → RX (crossover)

❌ **Mistake 3:** Forgot GND connection
- Fix: GND must be connected for signal reference

❌ **Mistake 4:** Wrong GPIO pins
- Fix: Robot GPIO 23, Display GPIO 18

❌ **Mistake 5:** Different baudrates
- Fix: Both must use 115200

---

## 📞 Support

**Still not working?**

Report these details:
1. Display serial output (copy-paste)
2. Robot serial output (copy-paste)
3. Photo of wiring
4. Multimeter measurements (GPIO 23 and 18 voltage)
5. USB power setup (how each ESP32 is powered)

---

**Good luck testing! 🚀**

*Last updated: 2025-11-06*
