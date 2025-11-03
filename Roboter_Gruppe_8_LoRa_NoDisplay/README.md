# LoRa NoDisplay Development Version

Development version without LCD for single-device testing with PC connection.

## Features

### ✅ Kill-Switch
- **Physical button**: Connect GPIO 12 ↔ GPIO 14
- **Hold 3 seconds** to restart device
- Countdown displayed in Serial Monitor
- Safety feature for emergency stop

### ✅ Connection Watchdog
- Automatic connection state tracking
- RSSI statistics (min/max/average)
- Packet loss detection with sequence numbers
- Auto-recovery on connection loss

### ✅ Structured Serial Output
- **CSV format** for easy parsing
- **JSON format** available
- Human-readable status updates
- Health monitoring reports

### ✅ Python Scripts for PC
- **serial_monitor.py** - Real-time data viewer
- **data_logger.py** - SQLite database logger

## Hardware Setup

### LoRa Module
```
RYLR896 -> ESP32
-----------------
TX  -> GPIO 25 (RXD2)
RX  -> GPIO 26 (TXD2)
VCC -> 3.3V
GND -> GND
```

### Role Detection
```
GPIO 17 (MODE_GND_PIN)    -> OUTPUT LOW (GND)
GPIO 16 (MODE_SELECT_PIN) -> INPUT_PULLUP

RECEIVER: GPIO 16 ↔ GPIO 17 (jumper wire)
SENDER:   GPIO 16 floating (no connection)
```

### Kill-Switch
```
GPIO 14 (KILLSWITCH_GND_PIN)  -> OUTPUT LOW (GND)
GPIO 12 (KILLSWITCH_READ_PIN) -> INPUT_PULLUP

To activate: Connect GPIO 12 ↔ GPIO 14 and hold 3 seconds
```

## Quick Start

### 1. Upload Code to ESP32
```bash
# Open Arduino IDE
# Select Board: ESP32 Dev Module
# Select Port: /dev/ttyUSB0 (or COM3 on Windows)
# Upload LoRa_NoDisplay_Dev.ino
```

### 2. Basic Monitoring (No Python)
```bash
# Use Arduino IDE Serial Monitor
# Set baud rate: 115200
# You'll see structured output
```

### 3. Python Monitoring (Recommended)

#### Install Requirements
```bash
pip install pyserial
```

#### Run Serial Monitor
```bash
# Linux/Mac
python serial_monitor.py /dev/ttyUSB0 115200

# Windows
python serial_monitor.py COM3 115200
```

**Output:**
```
╔═══════════════════════════════════════╗
║   LoRa Serial Monitor - v1.0         ║
╚═══════════════════════════════════════╝

Connecting to: /dev/ttyUSB0 @ 115200 baud
✓ Connected! Listening for data...

[14:35:12] RX
  State: CONNECTED | RSSI: -52 dBm █████ | SNR: 8 dB
  Messages: 45 | Seq: 42 | Loss: 2.3% | LED: 1 | Touch: 0
```

#### Run Data Logger
```bash
# Start logging to database
python data_logger.py /dev/ttyUSB0 115200 lora_data.db
```

**Creates SQLite database with:**
- All received messages
- RSSI/SNR history
- Packet loss tracking
- Event log (kill-switch, state changes, etc.)

## Serial Output Format

### CSV Format
```
DATA_CSV,TIMESTAMP,ROLE,RSSI,SNR,SEQ,MSG_COUNT,CONN_STATE,PACKET_LOSS,LED,TOUCH
DATA_CSV,125430,RX,-52,8,42,45,CONNECTED,2.3,1,0
```

**Fields:**
- `TIMESTAMP`: ESP32 millis()
- `ROLE`: RX (receiver) or TX (sender)
- `RSSI`: Signal strength (dBm)
- `SNR`: Signal-to-noise ratio (dB)
- `SEQ`: Sequence number
- `MSG_COUNT`: Total messages sent/received
- `CONN_STATE`: CONNECTED/WEAK/LOST/UNKNOWN
- `PACKET_LOSS`: Lost packets percentage
- `LED`: LED state (0/1)
- `TOUCH`: Touch sensor state (0/1)

### JSON Format (optional)
```json
{"type":"data","ts":125430,"role":"RX","rssi":-52,"snr":8,"seq":42,"msgCount":45,"state":"CONNECTED","loss":2.3,"led":1,"touch":0}
```

## Kill-Switch Usage

### Testing Kill-Switch

1. **Connect jumper wire** between GPIO 12 and GPIO 14
2. **Hold for 3 seconds**
3. Watch Serial Monitor:
```
🔴 Kill-switch pressed...
🔴 Hold for 2 more seconds to restart...
🔴 Hold for 1 more second to restart...

╔════════════════════════════════╗
║  🔴 KILL SWITCH ACTIVATED 🔴  ║
║     RESTARTING DEVICE...      ║
╚════════════════════════════════╝

[Device restarts]
```

### Use Cases
- Emergency stop during testing
- Quick restart without re-uploading code
- Safety feature for robot control
- Remote restart (future: via LoRa command)

## Database Usage

### View Logged Data
```bash
# Install SQLite browser or use command line
sqlite3 lora_data.db

# Query examples:
SELECT * FROM lora_messages ORDER BY timestamp DESC LIMIT 10;
SELECT AVG(rssi), MIN(rssi), MAX(rssi) FROM lora_messages;
SELECT connection_state, COUNT(*) FROM lora_messages GROUP BY connection_state;
```

### Export to CSV
```bash
sqlite3 lora_data.db
.mode csv
.output export.csv
SELECT * FROM lora_messages;
.quit
```

## Python Script Details

### serial_monitor.py

**Features:**
- Real-time colored output
- Parses CSV data automatically
- Shows RSSI quality bars
- Highlights errors/warnings
- No database, just live view

**Color Coding:**
- 🟢 Green: Connected, good signal, success messages
- 🟡 Yellow: Weak connection, warnings
- 🔴 Red: Lost connection, errors, kill-switch
- 🔵 Cyan: Info messages, headers

### data_logger.py

**Features:**
- Logs everything to SQLite database
- Tracks events (kill-switch, state changes)
- Shows statistics every 30s
- Minimal console output (focused on logging)
- Creates database automatically

**Database Tables:**
- `lora_messages` - All data messages
- `events` - Important events (kill-switch, connection loss, etc.)

## Troubleshooting

### Serial Port Issues

**Linux:**
```bash
# Find device
ls /dev/ttyUSB*

# Add user to dialout group (logout required)
sudo usermod -a -G dialout $USER

# Or use sudo
sudo python serial_monitor.py /dev/ttyUSB0 115200
```

**Windows:**
```
# Check Device Manager -> Ports (COM & LPT)
# Usually COM3, COM4, etc.

python serial_monitor.py COM3 115200
```

**Mac:**
```bash
# Find device
ls /dev/tty.usb*

python serial_monitor.py /dev/tty.usbserial-0001 115200
```

### Python Dependencies

```bash
# If pyserial not found
pip install pyserial

# Or use pip3
pip3 install pyserial
```

### Kill-Switch Not Working

1. **Check wiring:**
   - GPIO 14 should be LOW (provides GND)
   - GPIO 12 should be HIGH (pull-up) when not pressed
   - GPIO 12 should go LOW when connected to GPIO 14

2. **Check in Serial Monitor:**
```
✓ Kill-switch initialized (GPIO12↔14, hold 3s to restart)
```

3. **Test with multimeter:**
   - GPIO 14 = 0V
   - GPIO 12 (no connection) = 3.3V
   - GPIO 12 (connected to 14) = 0V

### No Data in Python

1. **Check Serial Monitor in Arduino IDE first**
   - Data should appear there
   - If not, check ESP32 code

2. **Check port and baudrate**
   - Must match (115200)
   - Port must be correct

3. **Close other programs**
   - Only one program can use serial port at a time
   - Close Arduino Serial Monitor before running Python

## Next Steps

### Planned Features
- [ ] Bi-directional communication
- [ ] Remote kill-switch via LoRa
- [ ] Real-time graphing (matplotlib)
- [ ] Web dashboard
- [ ] Data export tools
- [ ] Configuration via serial commands

### See Also
- `WATCHDOG_GUIDE.md` - Connection watchdog details
- `FUTURE_DEVELOPMENT.md` - Development roadmap
- `../Roboter_Gruppe_8_LoRa/` - Full version with LCD

## Quick Reference

### GPIO Pins Used
```
GPIO 2:  LED (built-in)
GPIO 4:  Touch sensor (T0)
GPIO 12: Kill-switch READ
GPIO 14: Kill-switch GND
GPIO 16: Mode select
GPIO 17: Mode GND
GPIO 25: LoRa RX (from RYLR896 TX)
GPIO 26: LoRa TX (to RYLR896 RX)
```

### Connection States
- `*` CONNECTED - Good (< 3s since last message, RSSI > -100 dBm)
- `!` WEAK - Warning (3-8s OR RSSI -100 to -110 dBm)
- `X` LOST - Error (> 8s since last message)
- `~` CONNECTING - Initializing
- `?` UNKNOWN - No data yet

### Useful Commands
```bash
# Monitor live
python serial_monitor.py /dev/ttyUSB0 115200

# Log to database
python data_logger.py /dev/ttyUSB0 115200 mydata.db

# View database
sqlite3 lora_data.db "SELECT * FROM lora_messages LIMIT 10;"

# Export CSV
sqlite3 lora_data.db -csv -header "SELECT * FROM lora_messages;" > export.csv
```

## Support

For issues or questions:
1. Check Serial Monitor output for errors
2. Review this README
3. Check `WATCHDOG_GUIDE.md` for connection issues
4. Create GitHub issue with:
   - Hardware setup
   - Serial Monitor output
   - Python error messages
