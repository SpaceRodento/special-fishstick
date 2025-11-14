# Data Logging & Analysis

Quick-start guide for collecting and analyzing LoRa telemetry data from ESP32 devices.

## 🚀 Quick Start

### 1. Connect ESP32 via USB

```bash
# Find your serial port
ls /dev/ttyUSB*    # Linux/Mac
# or check Device Manager on Windows (COM3, COM4, etc.)
```

### 2. Start Logging Data

**For extended telemetry (recommended):**
```bash
python data/data_logger_extended.py /dev/ttyUSB0 115200 my_data.db
```

**For basic logging:**
```bash
python data/data_logger.py /dev/ttyUSB0 115200 my_data.db
```

**For real-time monitoring:**
```bash
python data/serial_monitor.py /dev/ttyUSB0 115200
```

### 3. Analyze Your Data

```bash
python data/analyze_data.py my_data.db
```

This generates:
- Signal quality graphs (RSSI, SNR over time)
- Packet loss statistics
- Battery discharge curves (if enabled)
- Fire alarm detection timeline
- System performance metrics

## 📊 What Gets Logged

### Always Available
- **LoRa metrics**: RSSI, SNR, packet loss, sequence numbers
- **I/O states**: LED, touch sensor
- **Timestamps**: PC time and ESP32 uptime

### Optional Features (if enabled in ESP32)
- **Battery**: Voltage, percentage, status (GPIO 35)
- **Current**: mA, power (mW), energy (mAh) via INA219
- **System**: Heap memory, CPU temperature, loop frequency
- **Fire alarm**: Audio (3kHz beeps) and light (red LED flash) detection
- **LoRa advanced**: Spreading factor, TX power, retries

The logger **auto-detects** which features are enabled!

## 🧪 Testing Without Hardware

Generate realistic test data:

```bash
# 1 hour of example data
python data/example_data_generator.py 60 test_1h.db

# 24 hours of data
python data/example_data_generator.py 1440 test_24h.db

# Analyze it
python data/analyze_data.py test_1h.db
```

The generator simulates:
- Battery discharge over time
- Signal quality variations
- Random fire alarm events
- Realistic sensor noise

## 📁 Files in this Directory

### Core Scripts
- **`data_logger_extended.py`** - Full-featured logger with auto-detection (use this!)
- **`data_logger.py`** - Basic logger (backward compatible)
- **`serial_monitor.py`** - Real-time colored terminal viewer
- **`analyze_data.py`** - Analyze logged data, generate graphs
- **`realtime_plotter.py`** - Live plotting during logging

### Tools
- **`example_data_generator.py`** - Generate synthetic test data

### Documentation
- **`PC_LOGGING_README.md`** - Complete documentation (formats, troubleshooting, examples)
- **`DATABASE_SCHEMA.md`** - Database structure and SQL queries

## 🔧 ESP32 Configuration

Enable features in `Roboter_Gruppe_9/config.h`:

```cpp
#define ENABLE_CSV_OUTPUT true           // Required for logging
#define DATA_OUTPUT_INTERVAL 2000        // Log every 2 seconds

// Optional features (auto-detected by Python logger)
#define ENABLE_BATTERY_MONITOR true      // Battery voltage
#define ENABLE_CURRENT_MONITOR true      // INA219 current sensor
#define ENABLE_EXTENDED_TELEMETRY true   // Heap, temp, uptime
#define ENABLE_AUDIO_DETECTION true      // Smoke alarm sound
#define ENABLE_LIGHT_DETECTION true      // LED flash detection
```

After changing config, upload to ESP32 again.

## 📈 Example Workflow

```bash
# 1. Test connection
python data/serial_monitor.py /dev/ttyUSB0 115200

# 2. Start long-term logging
python data/data_logger_extended.py /dev/ttyUSB0 115200 experiment_$(date +%Y%m%d).db

# 3. Let it run (hours/days)...

# 4. Analyze results
python data/analyze_data.py experiment_20251114.db

# 5. Advanced analysis with Python/SQL
python
>>> import sqlite3, pandas as pd
>>> conn = sqlite3.connect('experiment_20251114.db')
>>> df = pd.read_sql_query("SELECT * FROM lora_messages", conn)
>>> print(df.describe())
>>> df.plot(x='timestamp', y='rssi')
```

## 🗄️ Database Structure

SQLite database with two main tables:

**`lora_messages`** - All telemetry data
- Timestamps (PC + ESP32)
- LoRa metrics (RSSI, SNR, packet loss)
- Battery data (voltage, current, energy)
- System metrics (heap, temperature)
- Sensor data (audio, light detection)

**`events`** - Important events
- Fire alarm detections
- Battery warnings
- Connection state changes
- Errors and warnings

See `DATABASE_SCHEMA.md` for complete field descriptions and example queries.

## 🐛 Troubleshooting

**Permission denied on /dev/ttyUSB0:**
```bash
sudo chmod 666 /dev/ttyUSB0
# or add user to dialout group
sudo usermod -a -G dialout $USER
```

**No CSV data appearing:**
1. Check `ENABLE_CSV_OUTPUT true` in config.h
2. Verify ESP32 is running (LED blinking?)
3. Try different baud rate (usually 115200)
4. Check USB cable connection

**Wrong serial port:**
```bash
# Linux: Try all USB ports
ls /dev/ttyUSB* /dev/ttyACM*

# Mac: Look for usbserial
ls /dev/cu.usbserial*
```

## 📚 More Information

- **Complete guide**: See `PC_LOGGING_README.md`
- **Database schema**: See `DATABASE_SCHEMA.md`
- **Firmware manual**: See `../Roboter_Gruppe_9/MANUAL.md`
- **Main project**: See `../README.md`

## 💡 Tips

1. Use descriptive database names: `lora_field_test_2025_11_14.db`
2. Keep `DATA_OUTPUT_INTERVAL` at 2000ms for best balance
3. Generate example data first to test your analysis scripts
4. Use `data_logger_extended.py` - it works with all configurations
5. Back up your databases regularly!

## 📋 Requirements

```bash
pip install pyserial matplotlib pandas
```

SQLite3 is included in Python standard library.

---

**Need help?** Check the detailed documentation in `PC_LOGGING_README.md`
