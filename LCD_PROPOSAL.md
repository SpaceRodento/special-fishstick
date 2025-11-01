# LCD Display Improvement Proposal

## Current Display (16 characters x 2 lines)

```
REM:1 T:0      |
LOC:0 T:1      v
```

**Shows:**
- Remote LED state (0/1)
- Remote Touch state (0/1)
- Local LED state (0/1)
- Local Touch state (0/1)
- Spinner animation (|/^/>/v)

**Missing:**
- Signal strength (RSSI)
- Signal quality (SNR)
- Message counter
- Connection status

---

## Proposed Improvements

### Option A: Signal Quality Always Visible ⭐ RECOMMENDED

```
RX:123 -45dB 8▲
L:0 T:1       v
```

**Line 1:**
- `RX:123` - Messages received count
- `-45dB` - RSSI (signal strength)
- `8` - SNR (signal-to-noise ratio)
- `▲` - Signal quality indicator

**Line 2:**
- `L:0` - Local LED state
- `T:1` - Local touch state
- `v` - Activity spinner

**Signal indicators:**
- `▲` Excellent (RSSI > -50 dBm)
- `=` Good (RSSI > -80 dBm)
- `-` Fair (RSSI > -100 dBm)
- `▼` Poor (RSSI > -110 dBm)
- `X` Critical (RSSI < -110 dBm)

**Code changes:**
```cpp
void updateLCD() {
  if (millis() - timing.lastLCD >= 100) {
    timing.lastLCD = millis();

    // Line 1: Signal quality
    lcd.setCursor(0, 0);
    lcd.print("RX:");
    lcd.print(remote.messageCount);
    lcd.print(" ");
    lcd.print(remote.rssi);
    lcd.print("dB ");
    lcd.print(remote.snr);
    lcd.print(getSignalIcon(remote.rssi));
    lcd.print("  ");  // Clear rest

    // Line 2: Local status
    lcd.setCursor(0, 1);
    lcd.print("L:");
    lcd.print(local.ledState);
    lcd.print(" T:");
    lcd.print(local.touchState);
    lcd.print("       ");
    lcd.setCursor(15, 1);
    lcd.print(spinner.symbols[local.spinnerIndex]);
  }
}

char getSignalIcon(int rssi) {
  if (rssi > -50) return '^';      // Excellent
  if (rssi > -80) return '=';      // Good
  if (rssi > -100) return '-';     // Fair
  if (rssi > -110) return 'v';     // Poor
  return 'X';                      // Critical
}
```

---

### Option B: Rotating Display (Multiple Pages)

**Page 1: Status (3 seconds)**
```
REM:1 T:0  #123
LOC:0 T:1     v
```

**Page 2: Signal (3 seconds)**
```
RSSI: -45 dBm
SNR:   +8 dB  v
```

**Page 3: Quality (3 seconds)**
```
Signal: GOOD▲
RX: 123  TX: 45
```

**Code:**
```cpp
uint8_t lcdPage = 0;
unsigned long lastPageSwitch = 0;

void updateLCD() {
  // Switch page every 3 seconds
  if (millis() - lastPageSwitch >= 3000) {
    lastPageSwitch = millis();
    lcdPage++;
    if (lcdPage > 2) lcdPage = 0;
    lcd.clear();
  }

  if (millis() - timing.lastLCD >= 100) {
    timing.lastLCD = millis();

    switch(lcdPage) {
      case 0: displayStatusPage(); break;
      case 1: displaySignalPage(); break;
      case 2: displayQualityPage(); break;
    }
  }
}
```

---

### Option C: Compact All-In-One

```
-45dB S:8 RX:12
L:0 T:1 RM:1  v
```

**Line 1:**
- `-45dB` - RSSI
- `S:8` - SNR
- `RX:12` - Messages (abbreviated count)

**Line 2:**
- `L:0` - Local LED
- `T:1` - Local touch
- `RM:1` - Remote LED
- `v` - Spinner

---

### Option D: Visual Signal Bar

```
[████▒] -45 #123
L:0 T:1 R:1   v
```

**Line 1:**
- `[████▒]` - Visual signal strength bar (5 chars)
- `-45` - RSSI value
- `#123` - Message count

**Line 2:**
- `L:0` - Local LED
- `T:1` - Local touch
- `R:1` - Remote LED
- `v` - Spinner

**Signal bar function:**
```cpp
String getSignalBar(int rssi) {
  // RSSI typically ranges from -120 (worst) to -40 (best)
  int bars = map(rssi, -120, -40, 0, 5);
  bars = constrain(bars, 0, 5);

  String bar = "[";
  for (int i = 0; i < 5; i++) {
    bar += (i < bars) ? (char)0xFF : ' ';  // 0xFF = solid block
  }
  bar += "]";
  return bar;
}
```

---

## Comparison Table

| Option | Pros | Cons | Best For |
|--------|------|------|----------|
| **A: Always Visible** | All info always visible, no waiting | Compact display | **Real-time monitoring** |
| **B: Rotating** | Detailed info, easy to read | Need to wait for page rotation | Detailed analysis |
| **C: Compact** | Maximum info density | Harder to read | Space-constrained |
| **D: Visual Bar** | Intuitive visual feedback | Takes screen space | Quick glance monitoring |

---

## Recommendation: **Option A** + Enhancements

**Recommended display:**
```
RX:123 -45dB 8▲
L:0 T:1 R:1   v
```

**Why:**
1. ✅ All critical info visible at once
2. ✅ No waiting for page rotation
3. ✅ Signal icon provides quick visual feedback
4. ✅ Simple to implement
5. ✅ Shows both local AND remote LED state

**Enhanced version with connection status:**
```
RX:123 -45▲ SNR8
L:0 T:1 R:1   v
```

Or if connection lost:
```
NO SIGNAL!  X
Last: 5s ago
```

---

## Additional Features to Add

### 1. Connection Timeout Warning
```cpp
// In loop(), check if no messages received
if (bRECEIVER && (millis() - remote.lastMessageTime > 10000)) {
  // No message for 10 seconds
  lcd.setCursor(0, 0);
  lcd.print("NO SIGNAL!     ");
}
```

### 2. Signal Quality Color Coding (if RGB LCD)
- Green: RSSI > -70 dBm
- Yellow: RSSI -70 to -100 dBm
- Red: RSSI < -100 dBm

### 3. Peak/Min RSSI Tracking
```cpp
int rssiMin = 0;
int rssiMax = -120;

// Update when message received
if (remote.rssi > rssiMax) rssiMax = remote.rssi;
if (remote.rssi < rssiMin) rssiMin = remote.rssi;
```

### 4. Message Rate Display
```cpp
// Calculate messages per minute
float messageRate = (remote.messageCount * 60000.0) / millis();
```

---

## Implementation Steps

### Step 1: Update `updateLCD()` function
- Add RSSI/SNR display
- Add signal quality icon
- Keep spinner animation

### Step 2: Add helper function
- `getSignalIcon(int rssi)` - Returns icon based on RSSI

### Step 3: Test and adjust
- Check if all text fits (16 chars)
- Adjust formatting as needed
- Test with different RSSI values

### Step 4: Optional enhancements
- Add timeout warning
- Add min/max tracking
- Add message rate

---

## Example Full Implementation

```cpp
void updateLCD() {
  if (millis() - timing.lastLCD >= 200) {  // Update every 200ms
    timing.lastLCD = millis();

    // Check for connection timeout (10 seconds)
    bool connectionLost = (millis() - remote.lastMessageTime > 10000);

    if (connectionLost) {
      // Show warning
      lcd.setCursor(0, 0);
      lcd.print("NO SIGNAL!     ");
      lcd.setCursor(0, 1);
      unsigned long seconds = (millis() - remote.lastMessageTime) / 1000;
      lcd.print("Last: ");
      lcd.print(seconds);
      lcd.print("s ago    ");
    } else {
      // Line 1: Signal quality info
      lcd.setCursor(0, 0);
      lcd.print("RX:");
      lcd.print(remote.messageCount);
      lcd.print(" ");

      // RSSI with sign
      if (remote.rssi >= 0) lcd.print("+");
      lcd.print(remote.rssi);
      lcd.print("dB");

      // Signal icon
      lcd.print(" ");
      lcd.print(getSignalIcon(remote.rssi));

      // Clear rest of line
      lcd.print("   ");

      // Line 2: Local and remote status
      lcd.setCursor(0, 1);
      lcd.print("L:");
      lcd.print(local.ledState);
      lcd.print(" T:");
      lcd.print(local.touchState);
      lcd.print(" R:");
      lcd.print(remote.ledState);
      lcd.print("  ");

      // Spinner at end
      lcd.setCursor(15, 1);
      lcd.print(spinner.symbols[local.spinnerIndex]);
    }
  }
}

char getSignalIcon(int rssi) {
  if (rssi > -50) return '^';       // Excellent: ▲
  else if (rssi > -80) return '=';  // Good: =
  else if (rssi > -100) return '-'; // Fair: -
  else if (rssi > -110) return 'v'; // Poor: ▼
  else return 'X';                  // Critical: X
}
```

**Display examples:**
```
RX:45 -52dB =
L:0 T:1 R:1  v
```

```
RX:123 -95dB v
L:1 T:0 R:1  ^
```

```
NO SIGNAL!
Last: 23s ago
```

---

## What would you like to implement?

1. **Option A** - Always visible (recommended) ⭐
2. **Option B** - Rotating pages
3. **Option C** - Compact
4. **Option D** - Visual bar
5. **Custom** - Mix and match features

Let me know and I'll implement it!
