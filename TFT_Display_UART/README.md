# Universal TFT Display Station

**UART-based TFT display for ESP32-2432S022** (Landscape mode)

Simple, flexible, and easy to integrate into any ESP32 project!

---

## 🎯 Features

✅ **Universal** - No project-specific dependencies!
✅ **2 wires only** - TX→RX and GND→GND
✅ **Landscape display** - 320x240 horizontal layout
✅ **Easy-to-use API** - `display.set("LED", "ON")`
✅ **Auto-update** - Display updates immediately
✅ **Flexible protocol** - CSV-formatted data
✅ **Multiple examples** - Copy-paste ready!
✅ **Color-coded UI** - Green/Orange/Red status colors
✅ **Alert system** - Show important warnings
✅ **Uptime tracking** - Built-in uptime counter

---

## 📦 Hardware

**Display Device (ESP32-2432S022):**
- ESP32-WROOM-32
- 2.4" ST7789 TFT (240x320 pixels)
- 8-bit parallel interface
- Integrated - no wiring needed!

**Sender ESP32:**
- Any ESP32 model (ESP32, ESP32-S2, ESP32-C3, etc.)
- 1× free GPIO pin (TX)

---

## 🔌 Wiring

### ONLY 2 WIRES!

```
┌─────────────────┐           ┌──────────────────┐
│   Sender ESP32  │           │  Display ESP32   │
│                 │           │  (2432S022)      │
│                 │           │                  │
│  GPIO XX (TX) ──┼──────────►│─ GPIO 3 (RX)    │
│  GND          ──┼───────────│─ GND             │
│                 │           │                  │
└─────────────────┘           └──────────────────┘
```

**Note:** You can use any free GPIO as TX on the sender device!

**Power:** Each ESP32 needs its own power source (USB or 5V). Don't power one from the other!

---

## 🚀 Quick Start (5 minutes to running!)

### Step 1: Upload to Display Device

```bash
# Arduino IDE
File → Open → TFT_Display_UART/DisplayDevice/DisplayDevice.ino
Tools → Board → ESP32 Dev Module
Tools → Port → (select your ESP32-2432S022)
Tools → Upload
```

**Required library:**
- LovyanGFX (install via Library Manager)

### Step 2: Copy Library to Your Sender Project

Copy `DisplayClient.h` to your project folder:
```
YourProject/
├── YourProject.ino
└── DisplayClient.h       ← Copy this!
```

Or use one of the provided examples!

### Step 3: Add to Your Code

```cpp
#include "DisplayClient.h"

DisplayClient display(17);  // TX pin 17

void setup() {
  Serial.begin(115200);
  display.begin();
}

void loop() {
  display.set("LED", "ON");
  display.set("Temp", 42);
  display.send();

  delay(1000);
}
```

### Step 4: Wire and Upload!

Connect the 2 wires, upload code, and the display starts updating! 🎉

---

## 📖 API Documentation

### Basic Functions

```cpp
DisplayClient display(17);  // Create client (TX pin)

display.begin();            // Initialize connection

display.set("key", "value"); // Add field
display.send();              // Send all fields

display.update("key", 42);   // Update one field immediately

display.alert("Fire!");      // Show alert
display.clearAlert();        // Remove alert

display.clearDisplay();      // Clear all fields
```

### Examples

#### Example 1: Simplest

```cpp
void loop() {
  display.update("Counter", counter++);
  delay(1000);
}
```

#### Example 2: Multiple Fields

```cpp
void loop() {
  display.clear();
  display.set("Temp", 22.5);
  display.set("Humidity", 65);
  display.set("Status", "OK");
  display.send();

  delay(2000);
}
```

#### Example 3: Alert

```cpp
if (temperature > 30.0) {
  display.alert("High temperature!");
} else {
  display.clearAlert();
}
```

#### Example 4: System Metrics

```cpp
display.clear();
display.set("LED", digitalRead(LED_PIN) ? "ON" : "OFF");
display.set("Temp", String(temp, 1) + "C");
display.set("Heap", String(ESP.getFreeHeap() / 1024) + "KB");
display.set("Uptime", String(millis()/1000) + "s");
display.send();
```

---

## 🔧 Configuration

### Change TX Pin

```cpp
DisplayClient display(25);  // Use GPIO 25
```

### Change Baudrate

```cpp
DisplayClient display(17, -1, 9600);  // 9600 baud
```

### Full-Duplex (TX + RX)

```cpp
DisplayClient display(17, 16);  // TX=17, RX=16
```

### Change Display Brightness

In Display Device (`DisplayDevice.ino`):
```cpp
#define BACKLIGHT_BRIGHTNESS 200  // 0-255
```

---

## 📊 Protocol

### Format

CSV-formatted data:
```
KEY:VALUE,KEY2:VALUE2,KEY3:VALUE3,...
```

### Examples

```
LED:ON,Temp:22.5,Status:OK
Counter:123,Voltage:3.85,Heap:245
Uptime:3600,LED:1,Status:RUNNING
```

### Special Commands

| Command | Description | Example |
|---------|-------------|---------|
| `ALERT:message` | Show alert | `ALERT:Fire detected!` |
| `CLEAR_ALERT` | Remove alert | `CLEAR_ALERT` |
| `CLEAR` | Clear all fields | `CLEAR` |

### Limitations

- Max 20 fields
- Max 256 characters per message
- Field names max ~20 characters
- Values max ~30 characters (longer text gets truncated)

---

## 💡 Example Projects

Three complete examples are provided:

### 1. Basic (`01_Basic`)

Simplest example with counter and uptime.

```cpp
#include "DisplayClient.h"
DisplayClient display(17);

void loop() {
  display.clear();
  display.set("Counter", counter++);
  display.set("Uptime", getUptimeString());
  display.send();
  delay(1000);
}
```

### 2. Sensors (`02_Sensors`)

Reading and displaying sensor data (DHT22, light sensor).

```cpp
display.clear();
display.set("Temp", String(temperature, 1) + "C");
display.set("Humidity", String(humidity, 0) + "%");
display.set("Light", String(lightLevel) + "%");
display.send();
```

### 3. Advanced (`03_Advanced`)

Complete example with system metrics, LED status, alerts.

```cpp
display.clear();
display.set("Count", messageCount);
display.set("LED", ledState ? "ON" : "OFF");
display.set("Uptime", uptimeStr);
display.set("Heap", String(freeHeap / 1024) + "KB");
display.set("Voltage", String(voltage, 2) + "V");
display.send();
```

---

## 🎨 Customization

### Change Colors

In `DisplayDevice.ino`:

```cpp
#define COLOR_BG           0x0000    // Background (black)
#define COLOR_PRIMARY      0x001F    // Header (blue)
#define COLOR_TEXT_PRIMARY 0xFFFF    // Text (white)
#define COLOR_SUCCESS      0x07E0    // Good (green)
#define COLOR_WARNING      0xFD20    // Warning (orange)
#define COLOR_ERROR        0xF800    // Error (red)
```

RGB565 color codes:
- Red: `0xF800`
- Green: `0x07E0`
- Blue: `0x001F`
- White: `0xFFFF`
- Black: `0x0000`
- Yellow: `0xFFE0`
- Cyan: `0x07FF`
- Magenta: `0xF81F`

### Change Font Size

```cpp
#define FONT_SMALL 1     // Small (8px)
#define FONT_NORMAL 2    // Normal (16px)
#define FONT_LARGE 3     // Large (24px)
```

---

## 🐛 Troubleshooting

### Display Not Responding

1. **Check wiring:**
   - TX (sender) → RX (display GPIO 3)
   - GND → GND
   - Wires intact?

2. **Check baudrate:**
   - Both at 115200?
   - `display.begin()` called?

3. **Check GPIO:**
   - TX pin correct?
   - Pin not used elsewhere?

4. **Serial Monitor:**
   - Open sender Serial Monitor
   - See "→ Display: ..." messages?

### Text Not Updating

1. **Call `send()`:**
   ```cpp
   display.set("LED", "ON");
   display.send();  // ← Important!
   ```

2. **Check field name:**
   - Same name overwrites old value
   - Different name creates new field

3. **Text too long:**
   - Max 30 characters per value
   - Gets truncated automatically

### "NO SIGNAL" on Display

1. **No data for 5 seconds:**
   - Sending often enough?
   - `delay()` too long?

2. **Wrong RX pin:**
   - Display uses GPIO 3
   - Check `UART_RX_PIN`

### Display Flickering

1. **Update less frequently:**
   ```cpp
   delay(500);  // At least 100ms between updates
   ```

2. **Only send when value changes:**
   ```cpp
   static int lastValue = 0;
   if (value != lastValue) {
     display.update("Val", value);
     lastValue = value;
   }
   ```

---

## 📐 Technical Details

### Display Device (ESP32-2432S022)

| Component | Details |
|-----------|---------|
| MCU | ESP32-WROOM-32 |
| Display | ST7789 2.4" 240x320 |
| Interface | 8-bit Parallel (MCU8080) |
| Touch | CST820 (I2C) - not used |
| UART | RX=GPIO3, TX=GPIO1 |
| Baudrate | 115200 (configurable) |
| Power | USB-C, min 500mA |

### Pin Allocations

**Display (integrated):**
- TFT Data: GPIO 12,13,14,15,25,27,32,33
- TFT Control: GPIO 2,4,16,17
- Touch I2C: GPIO 21,22 (optional)
- Backlight: GPIO 0

**Available:**
- GPIO 3 (UART RX) ← Used!
- GPIO 1 (UART TX) - optional
- GPIO 5,18,19,23,26,34-39

### Performance

- Update rate: 10 Hz (100ms)
- Latency: <50ms
- Max fields: 20
- Max message length: 256 characters
- RAM usage: ~2KB

---

## 🔄 Version History

### v1.0 (2025-11-15)
- ✅ Universal UART-based display
- ✅ DisplayClient library
- ✅ Three example projects
- ✅ Comprehensive documentation
- ✅ Alert system
- ✅ Uptime tracking
- ✅ Color-coded UI

---

## 💾 File Structure

```
TFT_Display_UART/
├── DisplayDevice/              # Display side code
│   ├── DisplayDevice.ino       # Main display program
│   └── display_config.h        # TFT configuration
│
├── SenderLibrary/              # Sender side library
│   └── DisplayClient.h         # Library for sender
│
├── examples/                   # Example projects
│   ├── 01_Basic/               # Simple counter + uptime
│   │   ├── Basic.ino
│   │   └── DisplayClient.h
│   ├── 02_Sensors/             # Sensor data display
│   │   ├── Sensors.ino
│   │   └── DisplayClient.h
│   └── 03_Advanced/            # Advanced features
│       ├── Advanced.ino
│       └── DisplayClient.h
│
└── README.md                   # This file
```

---

## 🤝 Integration with Existing Projects

### Simple Integration

1. Copy `DisplayClient.h` to your project folder
2. Include and create client in your code
3. Call `begin()` in setup
4. Send data with `set()` + `send()`

### Example Integration

```cpp
#include "DisplayClient.h"

DisplayClient display(17);

void setup() {
  Serial.begin(115200);

  // ... your existing setup code ...

  display.begin();
}

void loop() {
  // ... your existing code ...

  // Add display updates
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay >= 2000) {
    lastDisplay = millis();

    display.clear();
    display.set("YourData1", value1);
    display.set("YourData2", value2);
    display.send();
  }
}
```

---

## 📞 Support

**Problem?**
1. Read troubleshooting section
2. Check examples
3. Test Basic example first
4. Check Serial Monitor for debug info

**Tips:**
- Start simple
- Test one thing at a time
- Use `Serial.println()` for debugging
- Check wires with multimeter

---

## 📚 Dependencies

**Display Device:**
- [LovyanGFX](https://github.com/lovyan03/LovyanGFX) - TFT display library

**Sender Device:**
- None! Just copy DisplayClient.h

**Installation:**
```
Arduino IDE → Tools → Manage Libraries → Search "LovyanGFX" → Install
```

---

## 🎓 Learning Resources

### Documentation Links

- [LovyanGFX Documentation](https://github.com/lovyan03/LovyanGFX)
- [ESP32 UART Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)
- [ESP32-2432S022 Info](https://github.com/makerfabs/ESP32-2432S022)

### Example Use Cases

- **IoT Projects** - Display sensor readings, system status
- **Robotics** - Show robot telemetry, battery level
- **Home Automation** - Display temperature, humidity, time
- **Development** - Debug output, system monitoring
- **Education** - Learn ESP32 UART communication

---

**Ready to use!** 🚀

Start with `01_Basic` example and expand from there.

Good luck with your project! 🎉

---

## 📄 License

This project is provided as-is for educational and hobbyist use.
Free to use, modify, and distribute.

## 👤 Author

SpaceRodento - 2025

---

**Questions or improvements?** Feel free to modify and adapt to your needs!
