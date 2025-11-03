# Future Development Plan

## ✅ Completed Features

- [x] Basic LoRa communication (RYLR896)
- [x] Auto role detection (GPIO 16 ↔ 17 jumper)
- [x] LCD display with 4 versions
- [x] RSSI/SNR signal monitoring
- [x] Dual spinner animations (local + remote)
- [x] Connection Watchdog & Health Monitoring
- [x] Packet loss tracking with sequence numbers
- [x] Automatic connection recovery

---

## 🎯 Next Priority Features

### 1. Bi-directional Communication ⭐ HIGH PRIORITY

**Current:** Sender → Receiver (one-way)
**Goal:** Sender ↔ Receiver (two-way)

**Benefits:**
- Receiver can send acknowledgments (ACK)
- Receiver can send its own sensor data back to sender
- Sender knows if messages are received
- Both devices monitor each other's health

**Implementation Plan:**

#### Phase 1: Basic Bi-directional
```cpp
// Sender: Send data + occasionally listen for ACK
if (millis() - lastSend >= 2000) {
  sendData();
  lastSend = millis();
}

// Check for response from receiver
if (LoRaSerial.available()) {
  receiveResponse();
}
```

```cpp
// Receiver: Listen primarily + send ACK occasionally
if (receiveLoRaMessage()) {
  processData();

  // Send ACK every 5th message
  if (messageCount % 5 == 0) {
    sendAck();
  }
}
```

#### Phase 2: Full Duplex
- Both devices send AND receive
- Equal priority for both directions
- Collision avoidance (time-slotting or CSMA)

**Development Steps:**
1. Add receive capability to sender
2. Add send capability to receiver
3. Define ACK message format
4. Implement collision detection
5. Test bidirectional flow
6. Update LCD to show bidirectional status

**Estimated Time:** 2-3 hours

---

### 2. Kill-Switch Feature ⚐ HIGH PRIORITY

**Goal:** Emergency stop and device restart capability

#### A. Physical Kill-Switch
**Hardware:**
- Button between GPIO 18 and GND
- Pull-up resistor (internal)
- Debouncing in software

**Behavior:**
- Press once: Emergency stop (halt all operations)
- Hold 3s: Restart device (ESP32 reset)

**Implementation:**
```cpp
#define KILLSWITCH_PIN 18
bool killSwitchActive = false;

void checkKillSwitch() {
  static unsigned long pressStart = 0;
  bool pressed = (digitalRead(KILLSWITCH_PIN) == LOW);

  if (pressed) {
    if (pressStart == 0) {
      pressStart = millis();
    }

    // Hold for 3 seconds = restart
    if (millis() - pressStart >= 3000) {
      Serial.println("🔴 KILL SWITCH: RESTARTING...");
      ESP.restart();
    }
  } else {
    // Short press = toggle emergency stop
    if (pressStart > 0 && millis() - pressStart < 3000) {
      killSwitchActive = !killSwitchActive;
      Serial.println(killSwitchActive ? "🔴 EMERGENCY STOP" : "✓ Resumed");
    }
    pressStart = 0;
  }
}
```

#### B. Remote Kill-Switch
**LoRa Command:**
- Sender can send `CMD:KILL` to receiver
- Receiver can send `CMD:KILL` to sender (if bi-directional)

**Commands:**
- `CMD:STOP` - Emergency stop
- `CMD:RESUME` - Resume operations
- `CMD:RESTART` - Restart device

**Implementation:**
```cpp
void parseCommand(String payload) {
  if (payload.indexOf("CMD:STOP") >= 0) {
    killSwitchActive = true;
    Serial.println("🔴 REMOTE STOP RECEIVED");
  }
  else if (payload.indexOf("CMD:RESUME") >= 0) {
    killSwitchActive = false;
    Serial.println("✓ REMOTE RESUME RECEIVED");
  }
  else if (payload.indexOf("CMD:RESTART") >= 0) {
    Serial.println("🔴 REMOTE RESTART RECEIVED");
    delay(100);
    ESP.restart();
  }
}
```

**Safety Features:**
- Confirmation required for restart
- LED indication when kill-switch active
- LCD warning display
- Log all kill-switch events

**Development Steps:**
1. Add GPIO 18 physical button support
2. Implement debouncing
3. Add emergency stop logic
4. Define remote command protocol
5. Add command parsing
6. Update LCD to show kill-switch status
7. Test all scenarios

**Estimated Time:** 2-3 hours

---

### 3. USB/Python Data Logging & Visualization 📊

**Goal:** Connect receiver to laptop via USB and create real-time graphs + database

#### Architecture
```
[Sender ESP32]  ─LoRa─>  [Receiver ESP32]  ─USB─>  [Python Script]
                                                            ↓
                                                      [SQLite DB]
                                                            ↓
                                                    [Real-time Graph]
```

#### A. ESP32 Serial Output Format
**CSV Format** (easy for Python to parse):
```
TIMESTAMP,RSSI,SNR,SEQ,LED,TOUCH,CONN_STATE,PACKET_LOSS
1698765432,-52,8,42,1,0,CONNECTED,2.5
```

**JSON Format** (more flexible):
```json
{"ts":1698765432,"rssi":-52,"snr":8,"seq":42,"led":1,"touch":0,"state":"CONNECTED","loss":2.5}
```

**Implementation in ESP32:**
```cpp
void printDataCSV() {
  Serial.print(millis());
  Serial.print(",");
  Serial.print(remote.rssi);
  Serial.print(",");
  Serial.print(remote.snr);
  Serial.print(",");
  Serial.print(remote.sequenceNumber);
  Serial.print(",");
  Serial.print(remote.ledState);
  Serial.print(",");
  Serial.print(remote.touchState);
  Serial.print(",");
  Serial.print(getConnectionStateString(health.state));
  Serial.print(",");
  Serial.println(getPacketLoss(health), 2);
}
```

#### B. Python Data Logger

**Requirements:**
```bash
pip install pyserial matplotlib pandas sqlite3
```

**Basic Logger Script:**
```python
#!/usr/bin/env python3
import serial
import sqlite3
import time
from datetime import datetime

# Connect to ESP32
ser = serial.Serial('/dev/ttyUSB0', 115200)  # Adjust port
conn = sqlite3.connect('lora_data.db')
cursor = conn.cursor()

# Create table
cursor.execute('''
    CREATE TABLE IF NOT EXISTS lora_messages (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        timestamp DATETIME,
        esp_millis INTEGER,
        rssi INTEGER,
        snr INTEGER,
        sequence INTEGER,
        led BOOLEAN,
        touch BOOLEAN,
        conn_state TEXT,
        packet_loss REAL
    )
''')

print("📡 Listening for data...")

while True:
    try:
        line = ser.readline().decode('utf-8').strip()

        # Parse CSV
        parts = line.split(',')
        if len(parts) == 8:
            cursor.execute('''
                INSERT INTO lora_messages
                (timestamp, esp_millis, rssi, snr, sequence, led, touch, conn_state, packet_loss)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (datetime.now(), *parts))
            conn.commit()
            print(f"✓ Logged: RSSI={parts[1]} SNR={parts[2]}")

    except KeyboardInterrupt:
        break

conn.close()
ser.close()
print("✓ Logger stopped")
```

#### C. Real-time Plotter

**Matplotlib Live Plot:**
```python
#!/usr/bin/env python3
import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# Data buffers
timestamps = deque(maxlen=100)
rssi_values = deque(maxlen=100)
snr_values = deque(maxlen=100)

# Serial connection
ser = serial.Serial('/dev/ttyUSB0', 115200)

# Create figure
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

def update(frame):
    try:
        line = ser.readline().decode('utf-8').strip()
        parts = line.split(',')

        if len(parts) == 8:
            timestamps.append(int(parts[0]) / 1000.0)  # Convert to seconds
            rssi_values.append(int(parts[1]))
            snr_values.append(int(parts[2]))

            # Clear and redraw
            ax1.clear()
            ax2.clear()

            ax1.plot(list(timestamps), list(rssi_values), 'b-', label='RSSI')
            ax1.set_ylabel('RSSI (dBm)')
            ax1.set_title('LoRa Signal Quality - Real-time')
            ax1.legend()
            ax1.grid(True)

            ax2.plot(list(timestamps), list(snr_values), 'g-', label='SNR')
            ax2.set_xlabel('Time (s)')
            ax2.set_ylabel('SNR (dB)')
            ax2.legend()
            ax2.grid(True)

    except Exception as e:
        print(f"Error: {e}")

ani = animation.FuncAnimation(fig, update, interval=100)
plt.show()
```

#### D. Advanced Features

**Dashboard with Dash/Plotly:**
- Real-time web dashboard
- Multiple graphs (RSSI, SNR, packet loss)
- Connection state history
- Statistics panel

**Export Options:**
- Export to CSV
- Export to Excel
- Generate PDF reports

**Alerts:**
- Email notification on connection loss
- Telegram bot for remote monitoring

**Development Steps:**
1. Implement CSV/JSON serial output on ESP32
2. Create basic Python serial reader
3. Set up SQLite database
4. Build data logger script
5. Create real-time matplotlib plotter
6. Add export functions
7. Optional: Build web dashboard
8. Document setup instructions

**Estimated Time:** 4-6 hours

---

## 🔮 Additional Future Ideas

### 4. Power Management
- Deep sleep mode when idle
- Battery voltage monitoring
- Low-power LoRa settings
- Wake on button or timer

### 5. Enhanced Data Format
- Compressed binary protocol (more efficient)
- CRC checksum for data integrity
- Message encryption (secure communication)

### 6. Multiple Devices
- Support 3+ devices on same network
- Broadcast mode (one-to-many)
- Mesh networking (relay messages)

### 7. Web Interface
- ESP32 WiFi + web server
- Monitor LoRa data via browser
- Configuration GUI
- OTA firmware updates

### 8. Advanced Sensors
- GPS module (location tracking)
- Temperature/humidity sensors
- Motion detection (PIR)
- Camera module

### 9. Cloud Integration
- MQTT bridge to cloud
- AWS IoT / Azure IoT Hub
- Data analytics
- Historical dashboards

---

## 📋 Implementation Priority

### Immediate (Next Week):
1. ✅ Connection Watchdog & Health Monitoring - DONE!
2. 🔲 Bi-directional Communication
3. 🔲 Kill-Switch (Physical + Remote)

### Short-term (Next Month):
1. 🔲 USB/Python Data Logging
2. 🔲 Real-time Graphing
3. 🔲 SQLite Database Storage

### Medium-term (1-2 Months):
1. 🔲 Power Management
2. 🔲 Enhanced Data Format
3. 🔲 Web Dashboard

### Long-term (Future):
1. 🔲 Multiple Device Support
2. 🔲 Mesh Networking
3. 🔲 Cloud Integration

---

## 🎓 Learning Resources

**LoRa:**
- [RYLR896 Datasheet](https://reyax.com/products/rylr896/)
- [LoRa Parameters Guide](https://lora-developers.semtech.com/documentation/)

**ESP32:**
- [ESP32 Deep Sleep Guide](https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/)
- [ESP32 OTA Updates](https://randomnerdtutorials.com/esp32-over-the-air-ota-programming/)

**Python Data Visualization:**
- [Matplotlib Animation](https://matplotlib.org/stable/api/animation_api.html)
- [Plotly Dash](https://dash.plotly.com/)
- [Pandas for Data Analysis](https://pandas.pydata.org/docs/)

---

## 📞 Next Steps

**Choose what to implement next!**

1. ⭐ **Bi-directional Communication** - Most useful, enables full interaction
2. ⚐ **Kill-Switch** - Safety feature, good for robot control
3. 📊 **USB/Python Logging** - Data analysis and visualization

**What would you like to focus on?**
