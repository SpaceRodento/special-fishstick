# DisplayClient Library

This is the **sender side** library for communicating with the TFT Display Station.

Copy `DisplayClient.h` to your project folder to use it!

---

## 📦 Installation

### Method 1: Copy to Project (Recommended)

```
YourProject/
├── YourProject.ino
└── DisplayClient.h       ← Copy this file here
```

### Method 2: Use Examples

The library is already included in all example folders - just use those!

---

## 🚀 Quick Start

```cpp
#include "DisplayClient.h"

DisplayClient display(17);  // TX pin number

void setup() {
  Serial.begin(115200);
  display.begin();
}

void loop() {
  // Method 1: Update single field
  display.update("Counter", counter++);

  // Method 2: Send multiple fields
  display.clear();
  display.set("Temp", 22.5);
  display.set("Status", "OK");
  display.send();

  delay(1000);
}
```

---

## 📖 API Reference

### Constructor

```cpp
DisplayClient(tx_pin)                    // TX only
DisplayClient(tx_pin, rx_pin)            // Full duplex
DisplayClient(tx_pin, rx_pin, baudrate)  // Custom baudrate
```

**Examples:**
```cpp
DisplayClient display(17);              // TX on GPIO 17
DisplayClient display(17, 16);          // TX=17, RX=16
DisplayClient display(17, -1, 9600);    // 9600 baud
```

### Basic Methods

#### `begin()`
Initialize the UART connection to display.
```cpp
void setup() {
  display.begin();
}
```

#### `set(key, value)`
Add a key-value pair to the message buffer.
```cpp
display.set("LED", "ON");
display.set("Temp", 42);
display.set("Voltage", 3.3);
```

#### `send()`
Send all buffered fields to display.
```cpp
display.send();
```

#### `clear()`
Clear the message buffer (start new message).
```cpp
display.clear();
```

### Convenience Methods

#### `update(key, value)`
Update a single field immediately (clear + set + send).
```cpp
display.update("Counter", count);
```

#### `alert(message)`
Show an alert banner on display.
```cpp
display.alert("Temperature too high!");
```

#### `clearAlert()`
Remove the alert banner.
```cpp
display.clearAlert();
```

#### `clearDisplay()`
Clear all fields from display.
```cpp
display.clearDisplay();
```

#### `sendRaw(message)`
Send raw CSV message directly.
```cpp
display.sendRaw("KEY1:VAL1,KEY2:VAL2");
```

---

## 💡 Usage Examples

### Example 1: Simple Counter

```cpp
int count = 0;

void loop() {
  display.update("Count", count++);
  delay(1000);
}
```

### Example 2: Multiple Sensors

```cpp
void loop() {
  float temp = readTemperature();
  float humid = readHumidity();

  display.clear();
  display.set("Temp", String(temp, 1) + "C");
  display.set("Humidity", String(humid, 0) + "%");
  display.send();

  delay(2000);
}
```

### Example 3: With Alerts

```cpp
void loop() {
  float voltage = readVoltage();

  display.clear();
  display.set("Voltage", String(voltage, 2) + "V");
  display.send();

  if (voltage < 3.2) {
    display.alert("Low battery!");
  } else {
    display.clearAlert();
  }

  delay(1000);
}
```

### Example 4: System Monitor

```cpp
void loop() {
  display.clear();
  display.set("Heap", String(ESP.getFreeHeap() / 1024) + "KB");
  display.set("Uptime", String(millis() / 1000) + "s");
  display.set("WiFi", WiFi.RSSI());
  display.send();

  delay(2000);
}
```

---

## 🔧 Configuration

### Change TX Pin

```cpp
DisplayClient display(25);  // Use GPIO 25 instead
```

### Change Baudrate

Default is 115200. To change:
```cpp
DisplayClient display(17, -1, 9600);  // 9600 baud
```

**Note:** Display device must use same baudrate!

### Full-Duplex Mode

If you need bi-directional communication:
```cpp
DisplayClient display(17, 16);  // TX=17, RX=16
```

---

## 📊 Data Format

The library sends CSV-formatted messages:

```
KEY:VALUE,KEY2:VALUE2,KEY3:VALUE3
```

**Supported value types:**
- String: `"ON"`, `"Running"`
- Integer: `42`, `1234`
- Float: `22.5`, `3.14`
- Boolean: `true`/`false` (sent as "true"/"false")

---

## ⚠️ Limitations

- **Max fields:** 20 per message
- **Max message length:** 256 characters
- **Max value length:** ~30 characters (longer gets truncated on display)
- **Field name length:** Keep under 20 characters for best results

---

## 🐛 Troubleshooting

### No Output on Display

1. Check wiring: TX (sender) → GPIO 3 (display)
2. Verify `display.begin()` called in setup
3. Check baudrate matches (default: 115200)
4. Look for "→ Display:" in Serial Monitor

### Garbled Text

- Baudrate mismatch between sender and display
- Check wiring quality
- Ensure common ground (GND connected)

### Some Fields Missing

- Too many fields (max 20)
- Message too long (max 256 chars)
- Call `send()` after `set()`

---

## 📝 Notes

- All debug output goes to `Serial` (USB)
- Display communication uses `Serial1` (UART1)
- Library automatically handles message formatting
- Values are converted to strings automatically

---

## 🎯 Best Practices

1. **Keep field names short** - Better display layout
2. **Format numbers yourself** - `String(temp, 1)` for 1 decimal
3. **Don't send too fast** - Min 100ms between updates
4. **Use alerts sparingly** - Only for important events
5. **Clear buffer** - Always `clear()` before building new message

---

**That's it!** Simple and powerful. 🚀

See [examples](../examples/) for complete working code!
