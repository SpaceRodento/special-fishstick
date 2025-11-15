# Display Device

This is the **display side** of the TFT Display Station.

Upload this code to your ESP32-2432S022 display device.

---

## 📦 Hardware Required

- **ESP32-2432S022** - Makerfabs TFT display board
  - ESP32-WROOM-32
  - 2.4" ST7789 TFT display (240x320)
  - Built-in 8-bit parallel interface

---

## 📚 Dependencies

Install via Arduino IDE Library Manager:

- **LovyanGFX** - TFT display graphics library

```
Arduino IDE → Tools → Manage Libraries → Search "LovyanGFX" → Install
```

---

## ⚙️ Configuration

### Baudrate

Change in `DisplayDevice.ino`:
```cpp
#define UART_BAUDRATE 115200  // Change to match sender
```

### Display Brightness

```cpp
#define BACKLIGHT_BRIGHTNESS 200  // 0-255 (0=off, 255=max)
```

### Font Sizes

```cpp
#define FONT_SMALL 1     // 8px
#define FONT_NORMAL 2    // 16px
#define FONT_LARGE 3     // 24px
```

### Colors (RGB565)

```cpp
#define COLOR_BG           0x0000    // Black
#define COLOR_PRIMARY      0x001F    // Blue
#define COLOR_SUCCESS      0x07E0    // Green
#define COLOR_WARNING      0xFD20    // Orange
#define COLOR_ERROR        0xF800    // Red
```

Use [RGB565 Calculator](https://rgbcolorpicker.com/565) to get color codes.

---

## 🔌 Hardware Connections

The display uses **UART0** which shares pins with USB Serial:

- **RX:** GPIO 3 (physical connector)
- **TX:** GPIO 1 (not used)

**Wiring to sender ESP32:**
```
Sender TX → Display GPIO 3 (RX)
Sender GND → Display GND
```

⚠️ **Important:** Each ESP32 needs its own power source!

---

## 📊 Data Protocol

The display accepts CSV-formatted messages via UART:

```
KEY:VALUE,KEY2:VALUE2,KEY3:VALUE3
```

**Examples:**
```
LED:ON,Temp:22.5
Counter:42,Status:RUNNING
Uptime:3600,Heap:245KB
```

**Special Commands:**
- `CLEAR` - Clear all fields
- `ALERT:message` - Show alert banner
- `CLEAR_ALERT` - Remove alert

---

## 🚀 Upload Instructions

1. **Connect** ESP32-2432S022 via USB
2. **Select Board:**
   - `Tools → Board → ESP32 Dev Module`
3. **Select Port:**
   - `Tools → Port → (your port)`
4. **Upload:**
   - Click Upload button

---

## 🐛 Troubleshooting

### Upload Failed

- **Press BOOT button** during upload if needed
- Check USB cable supports data (not just power)
- Try different USB port

### Display Blank

- Check backlight brightness setting
- Verify LovyanGFX library installed
- Check Serial Monitor for error messages

### No Data Received

- Verify wiring: TX→RX, GND→GND
- Check baudrate matches sender
- Look for "📥 RX" messages in Serial Monitor

---

## 📺 Display Layout

```
┌────────────────────────────────────┐
│  HEADER (30px)                     │ <- Title & status
├────────────────────────────────────┤
│                                    │
│  DATA AREA (180px)                 │ <- Key:Value pairs
│  - Shows received fields           │
│  - Scrolls if >10 fields           │
│                                    │
├────────────────────────────────────┤
│  FOOTER (30px)                     │ <- Uptime / Alerts
└────────────────────────────────────┘
```

---

## ⚡ Performance

- **Update Rate:** 2 Hz (500ms interval)
- **Data Timeout:** 5 seconds (shows "NO SIGNAL")
- **Max Fields:** 20 simultaneous fields
- **Max Message:** 256 characters

---

## 🎨 Customization Ideas

### Change Title

```cpp
tft.drawString("Your Title", 5, HEADER_Y + 7);
```

### Adjust Layout

```cpp
#define HEADER_H 30      // Header height
#define DATA_H 180       // Data area height
#define FOOTER_H 30      // Footer height
```

### Add Animations

The display updates every 500ms - you can add simple animations in `updateDisplay()`.

---

**Ready!** Upload this to your display device and start receiving data! 🎉

See main [README.md](../README.md) for sender setup.
