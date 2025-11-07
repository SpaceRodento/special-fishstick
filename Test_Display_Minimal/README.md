# Test 1: Minimal Serial Test

**Purpose:** Verify that Arduino IDE can upload code and Serial Monitor works.

**This test has NO TFT, NO UART - just Serial.println()**

---

## ⚡ Quick Test

1. Open `Test_Display_Minimal.ino`
2. Board: "ESP32 Dev Module"
3. Select correct COM port
4. Upload
5. Open Serial Monitor (115200 baud)
6. Press RESET button

**Expected output:**
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

---

## ✅ If this works:

Arduino IDE is configured correctly!

**Next:** Try Test_Display_TFT_Only (test the display)

---

## ❌ If this DOESN'T work (no output):

**Problem:** Arduino IDE settings are wrong!

**Check:**
1. Board = "ESP32 Dev Module" (NOT ESP32-S2 or ESP32-C3!)
2. COM port is correct (unplug/replug USB to find it)
3. Serial Monitor baud rate = 115200
4. USB cable supports DATA (not just power)

**See:** `TROUBLESHOOTING_DISPLAY.md` for detailed help

---

**This is the FIRST test - if this doesn't work, nothing else will!**
