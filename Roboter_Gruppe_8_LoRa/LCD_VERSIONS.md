# LCD Display Versions Guide

This document shows all available LCD display versions. You can easily switch between them by commenting/uncommenting in the code.

## 🔄 How to Switch Versions

Open `Roboter_Gruppe_8_LoRa.ino` and find the `updateLCD()` function (around line 110).

You'll see this:
```cpp
void updateLCD() {
  if (millis() - timing.lastLCD >= 100) {
    timing.lastLCD = millis();

    // VERSION 1: WIDE VISUAL BAR + RSSI (RECOMMENDED) ⭐
    // Uncomment this version:
    updateLCD_Version1_WideBar();

    // VERSION 2: COMPACT WITH NUMBERS
    // Uncomment this version:
    // updateLCD_Version2_Compact();

    // VERSION 3: DETAILED INFO
    // Uncomment this version:
    // updateLCD_Version3_Detailed();

    // VERSION 4: ORIGINAL (Status only, no signal)
    // Uncomment this version:
    // updateLCD_Version4_Original();
  }
}
```

**To switch to Version 2:**
1. Comment out Version 1: `// updateLCD_Version1_WideBar();`
2. Uncomment Version 2: `updateLCD_Version2_Compact();`

---

## 📺 Version Previews

### VERSION 1: WIDE VISUAL BAR ⭐ (DEFAULT - RECOMMENDED)

```
┌────────────────┐
│[███████████]12 ^│  ← Wide bar (11 chars) + count + remote spinner
│-52dB L:1 T:0  v│  ← RSSI + Local LED + Touch + local spinner
└────────────────┘
```

**Shows:**
- **Line 1:** Wide visual signal strength bar (11 characters) + message counter + remote spinner (via LoRa, slow)
- **Line 2:** RSSI in dB, Local LED state, Touch state, local spinner (fast)

**Signal bar examples:**
- Strong: `[████████████] 45`  (-40 to -60 dBm)
- Good:   `[████████░░░░] 89`  (-60 to -80 dBm)
- Fair:   `[████░░░░░░░░] 12`  (-80 to -100 dBm)
- Weak:   `[██░░░░░░░░░░] 156` (-100 to -110 dBm)
- Poor:   `[░░░░░░░░░░░░] 201` (below -110 dBm)

**Best for:** Real-time signal monitoring, easy to see signal strength at a glance

**Connection lost warning:**
```
┌────────────────┐
│*** NO SIGNAL ***│
│Last: 23s ago    │
└────────────────┘
```

---

### VERSION 2: COMPACT

```
┌────────────────┐
│[████████] -52dB^│  ← Bar (8 chars) + RSSI + remote spinner
│S:8 L:1 R:0 #12v│  ← SNR + Local + Remote + Count + local spinner
└────────────────┘
```

**Shows:**
- **Line 1:** Signal bar (8 characters) + RSSI value + remote spinner (via LoRa, slow)
- **Line 2:** SNR, Local LED, Remote LED, Message count (last 2 digits), local spinner (fast)

**Best for:** All info on screen, both local and remote LED visible

---

### VERSION 3: DETAILED INFO

```
┌────────────────┐
│RX:123 -52dB =  │  ← Count + RSSI + Signal icon
│SNR:8 L:1 R:0  v│  ← SNR + Local + Remote + Spinner
└────────────────┘
```

**Shows:**
- **Line 1:** Message count, RSSI value, Signal quality icon
- **Line 2:** SNR value, Local LED, Remote LED, Spinner

**Signal quality icons:**
- `^` Excellent (> -50 dBm)
- `=` Good (> -80 dBm)
- `-` Fair (> -100 dBm)
- `v` Poor (> -110 dBm)
- `X` Critical (< -110 dBm)

**Best for:** Numerical values + quick quality indicator

---

### VERSION 4: ORIGINAL (No Signal Info)

```
┌────────────────┐
│REM:1 T:0      |│  ← Remote LED + Touch + Spinner
│LOC:0 T:1      v│  ← Local LED + Touch + Spinner
└────────────────┘
```

**Shows:**
- **Line 1:** Remote device LED and Touch state
- **Line 2:** Local device LED and Touch state
- **Both:** Activity spinners

**Best for:** Basic status monitoring without signal info

---

## 🎨 Visual Comparison

| Feature | V1 Wide Bar | V2 Compact | V3 Detailed | V4 Original |
|---------|-------------|------------|-------------|-------------|
| **Signal Bar** | ✅ 12 chars | ✅ 8 chars | ❌ | ❌ |
| **RSSI Value** | ✅ | ✅ | ✅ | ❌ |
| **SNR Value** | ❌ | ✅ | ✅ | ❌ |
| **Message Count** | ✅ 3 digits | ✅ 2 digits | ✅ Full | ❌ |
| **Local LED** | ✅ | ✅ | ✅ | ✅ |
| **Remote LED** | ❌ | ✅ | ✅ | ✅ |
| **Touch State** | ✅ Local | ❌ | ❌ | ✅ Both |
| **Signal Icon** | ❌ | ❌ | ✅ | ❌ |
| **Spinner** | ✅ | ✅ | ✅ | ✅ Both |
| **No Signal Warning** | ✅ | ✅ | ✅ | ❌ |

---

## 🔧 Customization

### Change Bar Width (Version 1)

In `updateLCD_Version1_WideBar()`:
```cpp
lcd.print(getSignalBar(remote.rssi, 12));  // ← Change 12 to desired width
```

**Examples:**
- Super wide: `14` chars → `[██████████████]`
- Full width: `16` chars → `[████████████████]`
- Narrow: `8` chars → `[████████]`

**Note:** Adjust other text on the line to fit!

### Change Bar Style

In `getSignalBar()` function:
```cpp
if (i < filled) {
  bar += (char)0xFF;  // ← Filled character (solid block)
} else {
  bar += (char)0xA1;  // ← Empty character (light shade)
}
```

**Alternative characters:**
- Solid block: `0xFF` → `█`
- Medium shade: `0xDB` → `█`
- Light shade: `0xB0` → `░`
- Very light shade: `0xA1` → `░`
- Hyphen: `'-'` → `-`
- Space: `' '` → ` `

**Example combinations:**
```cpp
// Style 1: Solid + Light
bar += (char)0xFF;  // █
bar += (char)0xA1;  // ░
// Result: [████░░░░]

// Style 2: Solid + Empty
bar += (char)0xFF;  // █
bar += ' ';         // (space)
// Result: [████    ]

// Style 3: Hash + Hyphen
bar += '#';
bar += '-';
// Result: [####----]
```

---

## 📊 Signal Strength Reference

Understanding RSSI values:

| RSSI Range | Quality | Distance (approx) | Version 1 Bar | Version 3 Icon |
|------------|---------|-------------------|---------------|----------------|
| -30 to -50 dBm | **Excellent** | 0-50m | `[████████████]` | `^` |
| -50 to -70 dBm | **Very Good** | 50-100m | `[██████████░░]` | `^` |
| -70 to -80 dBm | **Good** | 100-200m | `[████████░░░░]` | `=` |
| -80 to -90 dBm | **Fair** | 200-400m | `[██████░░░░░░]` | `=` |
| -90 to -100 dBm | **Marginal** | 400-800m | `[████░░░░░░░░]` | `-` |
| -100 to -110 dBm | **Poor** | 800-1500m | `[██░░░░░░░░░░]` | `v` |
| -110 to -120 dBm | **Critical** | 1500m+ | `[░░░░░░░░░░░░]` | `X` |

**SNR (Signal-to-Noise Ratio) reference:**
- **> 10 dB:** Excellent - very clean signal
- **5-10 dB:** Good - minor interference
- **0-5 dB:** Fair - noticeable interference
- **-5-0 dB:** Poor - significant interference
- **< -5 dB:** Critical - high packet loss likely

---

## 🎯 Recommended Setup by Use Case

### 1. Field Testing / Range Testing
**Use:** Version 1 (Wide Bar) ⭐
**Why:** Immediate visual feedback, easy to see signal changes while moving

### 2. Fixed Installation / Monitoring
**Use:** Version 2 (Compact) or Version 3 (Detailed)
**Why:** All numerical values visible, good for logging/documentation

### 3. Demonstration / Education
**Use:** Version 1 (Wide Bar) ⭐
**Why:** Most intuitive, easy to understand at a glance

### 4. Basic Operation (No Signal Interest)
**Use:** Version 4 (Original)
**Why:** Shows only device status, no extra info

---

## 💡 Pro Tips

### Tip 1: Watch the Signal Bar While Moving
Use Version 1 and walk around to find optimal antenna placement or best operating locations.

### Tip 2: Monitor SNR for Interference
If RSSI is good but SNR is low, there's interference. Use Version 2 or 3 to monitor SNR.

### Tip 3: Message Counter Shows Activity
If the counter stops incrementing, check the sender or LoRa connection.

### Tip 4: Connection Timeout Warning
All versions (except V4) show "NO SIGNAL" if no message for 10 seconds.

### Tip 5: Customize for Your Display
If your LCD has different characters available, adjust the bar style to match.

---

## 🐛 Troubleshooting

**Q: Bar shows all empty `[░░░░░░░░░░░░]`**
- RSSI is very weak (< -120 dBm)
- Check LoRa connection
- Check antenna

**Q: Display shows garbage characters**
- LCD might not support 0xFF character
- Try changing to `#` or `*` in getSignalBar()

**Q: Text doesn't fit on screen**
- Adjust message counter format: `remote.messageCount % 100` for 2 digits
- Reduce bar width in Version 1

**Q: "NO SIGNAL" appears immediately**
- Check that sender is running
- Verify both devices have same Network ID
- Check GPIO 16-17 jumper for correct role

**Q: Numbers look wrong**
- Check that `remote.rssi` and `remote.snr` are being updated
- Verify LoRa message parsing in `receiveLoRaMessage()`

---

## 📝 Quick Reference Card

Print this and keep near your device:

```
VERSION 1: WIDE BAR (Recommended)
  [███████████]12 ^     ← Bar + Count + Remote Spinner
  -52dB L:1 T:0   v     ← RSSI + Status + Local Spinner

VERSION 2: COMPACT
  [████████] -52dB^     ← Bar + RSSI + Remote Spinner
  S:8 L:1 R:0 #12v      ← SNR + All Status + Local Spinner

VERSION 3: DETAILED
  RX:123 -52dB =        ← Count + RSSI + Icon
  SNR:8 L:1 R:0  v      ← SNR + Status

VERSION 4: ORIGINAL
  REM:1 T:0      |      ← Remote Status
  LOC:0 T:1      v      ← Local Status

SPINNERS:
  Upper Right (^) = Remote spinner (via LoRa, slow updates)
  Lower Right (v) = Local spinner (fast animation)

SIGNAL ICONS (V3):
  ^ = Excellent  = = Good  - = Fair
  v = Poor       X = Critical
```

---

## 🚀 Future Ideas

Possible additions for future versions:

1. **Auto-switching display** - Rotate between versions every 5 seconds
2. **Peak/Min RSSI** - Track and display best/worst signal seen
3. **Average RSSI** - Show running average over last N messages
4. **Packet loss %** - Track missing sequence numbers
5. **Link quality score** - Combined RSSI+SNR metric (0-100%)
6. **Historical graph** - Show signal trend over time (if custom LCD chars)
7. **Battery voltage** - If running on battery
8. **Distance estimate** - Based on RSSI (requires calibration)

---

**Choose the version that works best for you!** 🎉
