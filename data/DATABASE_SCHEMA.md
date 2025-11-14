# Database Schema for LoRa Data Logging

## Overview

This document defines the database schema for logging all sensor data and telemetry from the ESP32 LoRa devices.

## Main Data Table: `lora_messages`

Primary table for all LoRa communication data with extended telemetry.

```sql
CREATE TABLE lora_messages (
    -- Primary key
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    -- Timestamps
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,  -- PC timestamp (when logged)
    esp_timestamp INTEGER,                         -- ESP32 millis() since boot

    -- Device identification
    role TEXT,                                     -- 'RX' (receiver) or 'TX' (sender)
    device_address INTEGER,                        -- LoRa address (1=RX, 2=TX, 3=Display)

    -- ===== CORE LORA METRICS (always present) =====
    rssi INTEGER,                                  -- Signal strength (dBm, -120 to -40)
    snr INTEGER,                                   -- Signal-to-Noise Ratio (dB)
    sequence INTEGER,                              -- Packet sequence number
    message_count INTEGER,                         -- Total messages sent/received
    connection_state TEXT,                         -- 'OK', 'WEAK', 'LOST', 'UNKNOWN'
    packet_loss REAL,                              -- Packet loss percentage (0.00 - 100.00)

    -- ===== BASIC I/O (always present) =====
    led_state INTEGER,                             -- LED on/off (0/1)
    touch_state INTEGER,                           -- Touch sensor (0/1)

    -- ===== BATTERY MONITORING (ENABLE_BATTERY_MONITOR) =====
    battery_voltage REAL,                          -- Battery voltage (V)
    battery_percentage REAL,                       -- Battery charge estimate (%)
    battery_status TEXT,                           -- 'OK', 'LOW', 'CRITICAL'

    -- ===== CURRENT MONITORING (ENABLE_CURRENT_MONITOR - INA219) =====
    current_ma REAL,                               -- Current draw (mA)
    bus_voltage REAL,                              -- Bus voltage (V)
    shunt_voltage REAL,                            -- Shunt voltage (mV)
    power_mw REAL,                                 -- Power consumption (mW)
    energy_mah REAL,                               -- Cumulative energy used (mAh)
    energy_wh REAL,                                -- Cumulative energy used (Wh)

    -- ===== SYSTEM TELEMETRY (ENABLE_EXTENDED_TELEMETRY) =====
    uptime_seconds INTEGER,                        -- Device uptime (seconds)
    free_heap INTEGER,                             -- Free heap memory (bytes)
    heap_fragmentation REAL,                       -- Heap fragmentation (%)
    cpu_temperature REAL,                          -- CPU temperature (°C, if available)
    loop_frequency INTEGER,                        -- Main loop frequency (Hz)

    -- ===== FIRE ALARM DETECTION =====
    -- Audio detection (ENABLE_AUDIO_DETECTION)
    audio_detected INTEGER,                        -- Smoke alarm sound detected (0/1)
    audio_rms INTEGER,                             -- Audio RMS level
    audio_peak_count INTEGER,                      -- Peaks per second

    -- Light detection (ENABLE_LIGHT_DETECTION - TCS34725)
    light_detected INTEGER,                        -- Flashing red LED detected (0/1)
    light_red INTEGER,                             -- Red color value (0-65535)
    light_green INTEGER,                           -- Green color value
    light_blue INTEGER,                            -- Blue color value
    light_clear INTEGER,                           -- Clear/brightness value
    light_lux REAL,                                -- Calculated lux value

    -- ===== ADVANCED LORA METRICS (ENABLE_PACKET_STATS) =====
    retries INTEGER,                               -- Transmission retries
    duplicates INTEGER,                            -- Duplicate packets received
    out_of_order INTEGER,                          -- Out-of-order packets
    spreading_factor INTEGER,                      -- Current LoRa SF (7-12)
    bandwidth INTEGER,                             -- LoRa bandwidth (kHz)
    coding_rate TEXT,                              -- Error correction rate (e.g., '4/5')
    tx_power INTEGER,                              -- Transmission power (dBm)

    -- ===== PERFORMANCE MONITORING (ENABLE_PERFORMANCE_MONITOR) =====
    cpu_usage REAL,                                -- CPU usage estimate (%)
    task_stack_free INTEGER,                       -- Free stack for main task (bytes)
    wifi_status TEXT,                              -- WiFi AP status (if enabled)

    -- ===== ADAPTIVE FEATURES =====
    adaptive_sf_active INTEGER,                    -- Adaptive SF enabled (0/1)
    encryption_active INTEGER                      -- Data encryption enabled (0/1)
);
```

## Indexes for Performance

```sql
-- Primary index on timestamp for time-series queries
CREATE INDEX idx_timestamp ON lora_messages(timestamp);

-- Index on role for filtering RX vs TX
CREATE INDEX idx_role ON lora_messages(role);

-- Index on connection state for quick state analysis
CREATE INDEX idx_connection_state ON lora_messages(connection_state);

-- Composite index for time-series analysis by role
CREATE INDEX idx_timestamp_role ON lora_messages(timestamp, role);
```

## Events Table

Separate table for logging important events, warnings, and state changes.

```sql
CREATE TABLE events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    event_type TEXT,                               -- 'START', 'STOP', 'ERROR', 'WARNING', 'STATE_CHANGE', etc.
    severity TEXT,                                 -- 'INFO', 'WARNING', 'ERROR', 'CRITICAL'
    description TEXT,                              -- Event description
    device_role TEXT,                              -- 'RX', 'TX', or 'BOTH'
    related_message_id INTEGER,                    -- Link to lora_messages table (optional)
    FOREIGN KEY (related_message_id) REFERENCES lora_messages(id)
);

CREATE INDEX idx_events_timestamp ON events(timestamp);
CREATE INDEX idx_events_type ON events(event_type);
CREATE INDEX idx_events_severity ON events(severity);
```

## Statistics Summary Table (Optional)

Pre-computed statistics for faster dashboard queries.

```sql
CREATE TABLE statistics_summary (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    period_start DATETIME,
    period_end DATETIME,
    role TEXT,

    -- Message statistics
    total_messages INTEGER,
    avg_rssi REAL,
    min_rssi INTEGER,
    max_rssi INTEGER,
    avg_snr REAL,
    avg_packet_loss REAL,

    -- Battery statistics (if enabled)
    avg_battery_voltage REAL,
    min_battery_voltage REAL,
    energy_consumed_mah REAL,

    -- Performance statistics
    avg_free_heap INTEGER,
    min_free_heap INTEGER,
    avg_loop_frequency INTEGER,

    -- Alert counts
    fire_alarm_detections INTEGER,
    low_battery_warnings INTEGER,
    connection_lost_count INTEGER
);

CREATE INDEX idx_stats_period ON statistics_summary(period_start, period_end);
```

## CSV Data Format (Extended)

### Core CSV Format (ENABLE_CSV_OUTPUT)

```
DATA_CSV,<timestamp>,<role>,<rssi>,<snr>,<seq>,<msg_count>,<conn_state>,<packet_loss>,<led>,<touch>
```

### Extended CSV Format (with all features enabled)

```
DATA_CSV_EXT,<timestamp>,<role>,<device_addr>,<rssi>,<snr>,<seq>,<msg_count>,<conn_state>,<packet_loss>,<led>,<touch>,<batt_v>,<batt_%>,<batt_status>,<current_ma>,<bus_v>,<power_mw>,<energy_mah>,<uptime_s>,<free_heap>,<cpu_temp>,<loop_freq>,<audio_det>,<audio_rms>,<light_det>,<light_r>,<light_g>,<light_b>,<light_lux>,<sf>,<tx_power>
```

### JSON Format (ENABLE_JSON_OUTPUT)

```json
{
  "ts": 123456,
  "role": "RX",
  "addr": 1,
  "lora": {
    "rssi": -85,
    "snr": 7,
    "seq": 42,
    "count": 142,
    "state": "OK",
    "loss": 0.5,
    "sf": 7,
    "power": 14
  },
  "io": {
    "led": 1,
    "touch": 0
  },
  "battery": {
    "voltage": 3.8,
    "percent": 75,
    "status": "OK",
    "current": 85.5,
    "power": 324.9,
    "energy_mah": 125.3
  },
  "system": {
    "uptime": 3600,
    "heap": 245000,
    "temp": 45.2,
    "freq": 100
  },
  "alarms": {
    "audio": 0,
    "light": 0
  }
}
```

## Field Descriptions

### Core LoRa Metrics
- **rssi**: Received Signal Strength Indicator in dBm (-120 = worst, -40 = best)
- **snr**: Signal-to-Noise Ratio in dB (higher is better)
- **sequence**: Incrementing packet number for loss detection
- **packet_loss**: Percentage of lost packets based on sequence gaps

### Battery Monitoring (GPIO 35 + voltage divider)
- **battery_voltage**: Direct ADC reading converted to voltage
- **battery_percentage**: Estimated charge level (100% = 4.2V, 0% = 3.0V)
- **battery_status**: OK (>3.3V), LOW (3.0-3.3V), CRITICAL (<3.0V)

### Current Monitoring (INA219 I2C sensor)
- **current_ma**: Instantaneous current draw
- **bus_voltage**: Battery voltage (more accurate than ADC)
- **power_mw**: Instantaneous power (V × I)
- **energy_mah/wh**: Cumulative energy consumption (integrates over time)

### System Telemetry
- **uptime_seconds**: Time since ESP32 boot
- **free_heap**: Available RAM (monitor for memory leaks)
- **cpu_temperature**: Internal temperature sensor (if available)
- **loop_frequency**: Main loop execution rate (should be ~100 Hz)

### Fire Alarm Detection
- **audio_detected**: Boolean flag for smoke alarm sound (3 kHz beeps)
- **audio_rms**: Root-Mean-Square audio level for threshold comparison
- **light_detected**: Boolean flag for flashing red LED
- **light_red/green/blue**: RGB values from TCS34725 color sensor
- **light_lux**: Calculated brightness in lux

### LoRa Advanced Stats
- **spreading_factor**: SF7-SF12 (higher = longer range, slower)
- **tx_power**: Transmission power in dBm (higher = longer range, more power)
- **retries/duplicates**: Protocol-level statistics

## Data Collection Strategies

### High-Frequency Data (every 2 seconds)
- Core LoRa metrics (RSSI, SNR, packets)
- I/O states (LED, touch)
- Current monitoring (for real-time power profiling)

### Medium-Frequency Data (every 10-30 seconds)
- Battery voltage (slow-changing)
- System telemetry (heap, temperature)
- Fire alarm monitoring

### Low-Frequency Data (every 60 seconds)
- Statistics summaries
- Performance reports
- Energy consumption totals

## Use Cases

### 1. Signal Quality Analysis
```sql
SELECT timestamp, rssi, snr, packet_loss
FROM lora_messages
WHERE role = 'RX'
ORDER BY timestamp;
```

### 2. Battery Life Prediction
```sql
SELECT
  timestamp,
  battery_voltage,
  current_ma,
  energy_mah,
  (3000.0 - energy_mah) / (current_ma / 60.0) AS estimated_hours_remaining
FROM lora_messages
WHERE role = 'TX' AND current_ma > 0
ORDER BY timestamp DESC
LIMIT 1;
```

### 3. Fire Alarm Detection Log
```sql
SELECT timestamp, audio_detected, light_detected, description
FROM lora_messages
LEFT JOIN events ON events.related_message_id = lora_messages.id
WHERE audio_detected = 1 OR light_detected = 1
ORDER BY timestamp;
```

### 4. Performance Profiling
```sql
SELECT
  AVG(loop_frequency) as avg_freq,
  MIN(free_heap) as min_heap,
  AVG(cpu_temperature) as avg_temp,
  AVG(current_ma) as avg_current
FROM lora_messages
WHERE timestamp > datetime('now', '-1 hour');
```

## Migration Path

The schema is designed to be backward-compatible:
- All optional fields can be NULL
- Existing `lora_messages` table can be extended with `ALTER TABLE ADD COLUMN`
- Python logger can detect which features are enabled based on CSV format

## Next Steps

1. ✅ Update ESP32 code to output extended CSV format
2. ✅ Update `data_logger.py` to handle extended schema
3. ✅ Update `analyze_data.py` to visualize new metrics
4. ✅ Create example data generator for testing
5. ✅ Update PC_LOGGING_README.md with new format documentation
