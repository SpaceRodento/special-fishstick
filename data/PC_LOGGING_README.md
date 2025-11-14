# PC Data Logging Guide

Python scripts for real-time monitoring and data logging from ESP32 via serial connection.

Supports both **basic** and **extended** telemetry modes with automatic format detection.

## 📋 Requirements

```bash
pip install pyserial matplotlib pandas
```

For database logging (data_logger.py only):
- SQLite3 (included in Python standard library)

For data analysis and visualization (analyze_data.py):
- pandas
- matplotlib

## 🚀 Quick Start

### 1. Enable CSV Output

In `config.h`, ensure CSV output is enabled:
```cpp
#define ENABLE_CSV_OUTPUT true       // Enable CSV data output
#define DATA_OUTPUT_INTERVAL 2000    // Output every 2 seconds
```

Upload the code to your ESP32.

### 2. Find Your Serial Port

**Linux/Mac:**
```bash
ls /dev/ttyUSB*    # or /dev/ttyACM*
```

**Windows:**
- Check Device Manager → Ports (COM & LPT)
- Usually COM3, COM4, etc.

### 3. Run Serial Monitor (Real-time Viewer)

```bash
python serial_monitor.py /dev/ttyUSB0 115200
```

**Output example:**
```
[10:23:45] RX
  State: OK | RSSI: -65 dBm █████ | SNR: 8 dB
  Messages: 142 | Seq: 142 | Loss: 0.0% | LED: 1 | Touch: 0
```

### 4. Run Data Logger (Database Storage)

```bash
python data_logger.py /dev/ttyUSB0 115200 lora_data.db
```

Creates SQLite database with all received data for later analysis.

## 📊 CSV Data Formats

### Basic Format (ENABLE_CSV_OUTPUT)

```
DATA_CSV,TIMESTAMP,ROLE,RSSI,SNR,SEQ,MSG_COUNT,CONN_STATE,PACKET_LOSS,LED,TOUCH
```

**Example:**
```
DATA_CSV,45632,RX,-67,9,142,142,OK,0.00,1,0
```

**Fields:**
- `TIMESTAMP`: Milliseconds since ESP32 boot
- `ROLE`: RX (receiver) or TX (sender)
- `RSSI`: Signal strength in dBm (-120 to -40)
- `SNR`: Signal-to-Noise Ratio in dB
- `SEQ`: Sequence number (for packet loss detection)
- `MSG_COUNT`: Total messages sent/received
- `CONN_STATE`: OK, WEAK, LOST, UNKNOWN
- `PACKET_LOSS`: Percentage (0.00 to 100.00)
- `LED`: LED state (0 or 1)
- `TOUCH`: Touch sensor state (0 or 1)

### Extended Format (with optional telemetry enabled)

When additional features are enabled in config.h, the CSV format automatically extends to include:

**Battery Monitoring** (ENABLE_BATTERY_MONITOR):
- `BATTERY_V`: Battery voltage (V)
- `BATTERY_%`: Battery percentage (0-100)
- `BATTERY_STATUS`: OK, LOW, CRITICAL

**Current Monitoring** (ENABLE_CURRENT_MONITOR):
- `CURRENT_MA`: Current draw (mA)
- `BUS_V`: Bus voltage (V)
- `POWER_MW`: Power consumption (mW)
- `ENERGY_MAH`: Cumulative energy (mAh)

**System Telemetry** (ENABLE_EXTENDED_TELEMETRY):
- `UPTIME_S`: Device uptime (seconds)
- `FREE_HEAP`: Free heap memory (bytes)
- `CPU_TEMP`: CPU temperature (°C)
- `LOOP_FREQ`: Main loop frequency (Hz)

**Fire Alarm Detection**:
- Audio (ENABLE_AUDIO_DETECTION): `AUDIO_DET`, `AUDIO_RMS`, `AUDIO_PEAKS`
- Light (ENABLE_LIGHT_DETECTION): `LIGHT_DET`, `LIGHT_R`, `LIGHT_G`, `LIGHT_B`, `LIGHT_LUX`

**Advanced LoRa** (ENABLE_PACKET_STATS):
- `SF`: Spreading Factor (7-12)
- `TX_POWER`: Transmission power (dBm)

**Example Extended CSV:**
```
DATA_CSV,45632,RX,1,-67,9,142,142,OK,0.00,1,0,3.85,75,OK,85.5,3.85,329.2,125.3,3600,245000,45.2,100,0,85,0,0,500,300,200,15.5,7,14
```

## 🛠 Python Scripts

### serial_monitor.py

**Real-time colored terminal output**

Features:
- ✅ Color-coded connection states (green=OK, yellow=WEAK, red=LOST)
- ✅ RSSI quality bars (visual signal strength)
- ✅ Live data updates
- ✅ Highlights warnings and errors
- ✅ Shows raw serial output + parsed CSV data

**Usage:**
```bash
python serial_monitor.py [PORT] [BAUDRATE]

# Examples:
python serial_monitor.py /dev/ttyUSB0 115200
python serial_monitor.py COM3 115200
```

**When to use:**
- Quick testing and debugging
- Real-time monitoring during development
- Checking connection quality
- Verifying data transmission

### data_logger.py

**Basic SQLite database logger**

Features:
- ✅ Automatic database creation
- ✅ Stores all CSV data with timestamps
- ✅ Event logging (errors, warnings, state changes)
- ✅ Indexed for fast queries
- ✅ Color-coded terminal feedback

**Usage:**
```bash
python data_logger.py [PORT] [BAUDRATE] [DATABASE]

# Examples:
python data_logger.py /dev/ttyUSB0 115200 lora_data.db
python data_logger.py COM3 115200 my_experiment.db
```

### data_logger_extended.py ⭐ NEW

**Extended telemetry logger with full feature support**

Features:
- ✅ All features of basic logger
- ✅ **Auto-detects CSV format** (basic or extended)
- ✅ Supports all optional telemetry fields
- ✅ Battery monitoring (voltage, percentage, status)
- ✅ Current monitoring (INA219 sensor)
- ✅ System telemetry (uptime, heap, temperature)
- ✅ Fire alarm detection (audio + light sensors)
- ✅ Advanced LoRa statistics
- ✅ Extended database schema

**Usage:**
```bash
python data_logger_extended.py [PORT] [BAUDRATE] [DATABASE]

# Examples:
python data_logger_extended.py /dev/ttyUSB0 115200 lora_extended.db
python data_logger_extended.py COM3 115200 full_test.db
```

**Auto-detection:**
The logger automatically detects which features are enabled in your ESP32 and adapts the database schema. No configuration needed!

**Database schema:**
```sql
CREATE TABLE lora_messages (
    id INTEGER PRIMARY KEY,
    timestamp DATETIME,           -- PC time
    esp_timestamp INTEGER,        -- ESP32 millis()
    role TEXT,                    -- RX or TX
    rssi INTEGER,
    snr INTEGER,
    sequence INTEGER,
    message_count INTEGER,
    connection_state TEXT,
    packet_loss REAL,
    led_state INTEGER,
    touch_state INTEGER
);

CREATE TABLE events (
    id INTEGER PRIMARY KEY,
    timestamp DATETIME,
    event_type TEXT,
    description TEXT
);
```

**When to use:**
- Long-term data collection
- Performance analysis
- Signal quality studies
- Packet loss statistics
- Creating graphs and reports

## 📈 Analyzing Logged Data

### Using SQLite Command Line

```bash
sqlite3 lora_data.db

# Count total messages
SELECT COUNT(*) FROM lora_messages;

# Average RSSI
SELECT AVG(rssi) FROM lora_messages;

# Packet loss over time
SELECT timestamp, packet_loss FROM lora_messages ORDER BY timestamp;

# Connection state changes
SELECT timestamp, connection_state FROM lora_messages
WHERE connection_state != 'OK';
```

### Using Python

```python
import sqlite3
import pandas as pd

conn = sqlite3.connect('lora_data.db')
df = pd.read_sql_query("SELECT * FROM lora_messages", conn)

print(df.describe())  # Statistics
df.plot(x='timestamp', y='rssi')  # Plot RSSI over time
```

## 🔧 Configuration Options

### Change Output Format

In `config.h`:
```cpp
#define ENABLE_CSV_OUTPUT true       // CSV format (recommended)
#define ENABLE_JSON_OUTPUT false     // JSON format (alternative)
#define DATA_OUTPUT_INTERVAL 2000    // Interval in milliseconds
```

### Change Output Interval

```cpp
#define DATA_OUTPUT_INTERVAL 1000    // Every 1 second (more frequent)
#define DATA_OUTPUT_INTERVAL 5000    // Every 5 seconds (less data)
```

### Disable PC Logging

To reduce serial output clutter during development:
```cpp
#define ENABLE_CSV_OUTPUT false
```

## 🐛 Troubleshooting

**Port not found:**
```bash
# Linux: Check permissions
sudo chmod 666 /dev/ttyUSB0

# Or add user to dialout group
sudo usermod -a -G dialout $USER
# Then logout and login
```

**No CSV data appearing:**
1. Check `ENABLE_CSV_OUTPUT true` in config.h
2. Verify ESP32 is running (check boot messages)
3. Check baudrate matches (115200)
4. Ensure LoRa module is connected (receiver role)

**Garbled output:**
- Wrong baudrate - check both ESP32 code (115200) and Python command
- Cable issue - try different USB cable/port

**Database file locked:**
- Close other programs accessing the database
- Use only one data_logger.py instance at a time

## 💡 Tips

**Best practices:**
1. Use `serial_monitor.py` for initial testing
2. Switch to `data_logger.py` for experiments
3. Keep DATA_OUTPUT_INTERVAL at 2000ms (2s) for balance
4. Name database files with date: `lora_2025_11_03.db`
5. Backup databases regularly

**Performance:**
- CSV output adds minimal overhead (~200 bytes every 2s)
- Does not affect LoRa communication
- Works on both receiver and sender (but receiver has more data)

## 📚 Example Workflow

```bash
# 1. Development and testing
python serial_monitor.py /dev/ttyUSB0 115200

# 2. When ready for data collection
python data_logger.py /dev/ttyUSB0 115200 experiment_1.db

# 3. Analyze results
sqlite3 experiment_1.db
> SELECT AVG(rssi), AVG(packet_loss) FROM lora_messages WHERE role='RX';

# 4. Create report
python
>>> import sqlite3, pandas as pd
>>> conn = sqlite3.connect('experiment_1.db')
>>> df = pd.read_sql_query("SELECT * FROM lora_messages", conn)
>>> df.to_csv('results.csv')
```

## 🧪 Testing with Example Data

Generate realistic synthetic data for testing:

```bash
# Generate 1 hour of example data
python example_data_generator.py 60 example_1h.db

# Generate 24 hours of data
python example_data_generator.py 1440 example_24h.db

# Analyze the example data
python analyze_data.py example_1h.db
```

The generator creates:
- ✅ Realistic signal variations
- ✅ Battery discharge over time
- ✅ Random fire alarm events
- ✅ Connection quality changes
- ✅ Full telemetry data

**Output:** Both SQLite database (.db) and CSV file (.csv)

## 📚 Database Schema

See [DATABASE_SCHEMA.md](DATABASE_SCHEMA.md) for complete documentation of:
- Table structures
- Field descriptions
- Indexes
- Example queries
- Use cases

## 🔧 Configuration in ESP32

Enable features in `config.h`:

```cpp
// Basic logging (always on)
#define ENABLE_CSV_OUTPUT true       // Enable CSV data output
#define DATA_OUTPUT_INTERVAL 2000    // Output every 2 seconds

// Extended telemetry (optional)
#define ENABLE_BATTERY_MONITOR true       // Battery voltage (GPIO 35)
#define ENABLE_CURRENT_MONITOR true       // INA219 current sensor (I2C)
#define ENABLE_EXTENDED_TELEMETRY true    // Uptime, heap, temp
#define ENABLE_AUDIO_DETECTION true       // Smoke alarm sound (GPIO 34)
#define ENABLE_LIGHT_DETECTION true       // Flashing LED (TCS34725 I2C)
#define ENABLE_PACKET_STATS true          // Advanced LoRa stats
```

Each feature adds corresponding fields to the CSV output. The Python loggers detect this automatically!

## 🎯 Next Steps

1. ✅ Upload code with `ENABLE_CSV_OUTPUT true`
2. ✅ Test with `serial_monitor.py`
3. ✅ Collect data with `data_logger_extended.py` (recommended) or `data_logger.py`
4. ✅ Analyze your results with `analyze_data.py`
5. ✅ Generate example data for testing Python scripts

## 🆕 What's New in Extended Version

- **Auto-detection**: Automatically adapts to enabled features
- **Full telemetry**: Battery, current, temperature, fire alarms
- **Better events**: Tracks fire alarms, battery warnings, state changes
- **Example data**: Generate realistic test data for development
- **Complete schema**: Documented database structure
- **Backward compatible**: Works with basic CSV format too

---

**Questions?** Check:
- [DATABASE_SCHEMA.md](DATABASE_SCHEMA.md) - Complete database documentation
- Main README.md - Project overview
- Python script source code - Implementation details
