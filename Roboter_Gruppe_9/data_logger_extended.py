#!/usr/bin/env python3
"""
LoRa Data Logger - Extended Version with Full Telemetry Support

Logs all sensor data from ESP32 to SQLite database with support for:
- Core LoRa metrics (RSSI, SNR, packet loss)
- Battery monitoring (voltage, percentage)
- Current monitoring (INA219)
- System telemetry (uptime, heap, temperature)
- Fire alarm detection (audio + light sensors)
- Advanced LoRa statistics

Auto-detects CSV format (basic or extended) and adapts accordingly.

Usage:
    python data_logger_extended.py [port] [baudrate] [database_file]

Example:
    python data_logger_extended.py /dev/ttyUSB0 115200 lora_extended.db
    python data_logger_extended.py COM3 115200
"""

import serial
import sqlite3
import sys
import time
from datetime import datetime
import re

# ANSI color codes
class Colors:
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    CYAN = '\033[96m'
    BLUE = '\033[94m'

def print_colored(text, color):
    """Print text in color"""
    print(f"{color}{text}{Colors.ENDC}")

def create_database(db_file):
    """Create database with extended schema"""
    conn = sqlite3.connect(db_file)
    cursor = conn.cursor()

    # Main data table with all possible fields
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS lora_messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            esp_timestamp INTEGER,
            role TEXT,
            device_address INTEGER,

            -- Core LoRa metrics
            rssi INTEGER,
            snr INTEGER,
            sequence INTEGER,
            message_count INTEGER,
            connection_state TEXT,
            packet_loss REAL,

            -- Basic I/O
            led_state INTEGER,
            touch_state INTEGER,

            -- Battery monitoring
            battery_voltage REAL,
            battery_percentage REAL,
            battery_status TEXT,

            -- Current monitoring (INA219)
            current_ma REAL,
            bus_voltage REAL,
            shunt_voltage REAL,
            power_mw REAL,
            energy_mah REAL,
            energy_wh REAL,

            -- System telemetry
            uptime_seconds INTEGER,
            free_heap INTEGER,
            heap_fragmentation REAL,
            cpu_temperature REAL,
            loop_frequency INTEGER,

            -- Fire alarm detection
            audio_detected INTEGER,
            audio_rms INTEGER,
            audio_peak_count INTEGER,
            light_detected INTEGER,
            light_red INTEGER,
            light_green INTEGER,
            light_blue INTEGER,
            light_clear INTEGER,
            light_lux REAL,

            -- Advanced LoRa metrics
            retries INTEGER,
            duplicates INTEGER,
            out_of_order INTEGER,
            spreading_factor INTEGER,
            bandwidth INTEGER,
            coding_rate TEXT,
            tx_power INTEGER,

            -- Performance monitoring
            cpu_usage REAL,
            task_stack_free INTEGER,
            wifi_status TEXT,

            -- Adaptive features
            adaptive_sf_active INTEGER,
            encryption_active INTEGER
        )
    ''')

    # Events table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            event_type TEXT,
            severity TEXT,
            description TEXT,
            device_role TEXT,
            related_message_id INTEGER,
            FOREIGN KEY (related_message_id) REFERENCES lora_messages(id)
        )
    ''')

    # Create indexes
    cursor.execute('CREATE INDEX IF NOT EXISTS idx_timestamp ON lora_messages(timestamp)')
    cursor.execute('CREATE INDEX IF NOT EXISTS idx_role ON lora_messages(role)')
    cursor.execute('CREATE INDEX IF NOT EXISTS idx_connection_state ON lora_messages(connection_state)')
    cursor.execute('CREATE INDEX IF NOT EXISTS idx_events_timestamp ON events(timestamp)')
    cursor.execute('CREATE INDEX IF NOT EXISTS idx_events_type ON events(event_type)')

    conn.commit()
    print_colored(f"✓ Database created with extended schema", Colors.OKGREEN)
    return conn

def parse_csv_basic(line):
    """Parse basic CSV format: DATA_CSV,TIMESTAMP,ROLE,RSSI,SNR,SEQ,MSG_COUNT,CONN_STATE,PACKET_LOSS,LED,TOUCH"""
    parts = line.split(',')
    if len(parts) != 11:
        return None

    try:
        data = {
            'esp_timestamp': int(parts[1]),
            'role': parts[2],
            'rssi': int(parts[3]),
            'snr': int(parts[4]),
            'sequence': int(parts[5]),
            'message_count': int(parts[6]),
            'connection_state': parts[7],
            'packet_loss': float(parts[8]),
            'led_state': int(parts[9]),
            'touch_state': int(parts[10])
        }
        return ('basic', data)
    except (ValueError, IndexError):
        return None

def parse_csv_extended(line):
    """Parse extended CSV format with all telemetry fields"""
    parts = line.split(',')

    # Extended format can vary in length depending on which features are enabled
    # Minimum: same as basic (11 fields)
    if len(parts) < 11:
        return None

    try:
        data = {
            'esp_timestamp': int(parts[1]),
            'role': parts[2],
            'device_address': int(parts[3]) if len(parts) > 11 and parts[3].isdigit() else None,
        }

        # Determine offset based on whether device_address is present
        offset = 4 if data['device_address'] is not None else 3

        # Core metrics (always present)
        data['rssi'] = int(parts[offset]) if offset < len(parts) else None
        data['snr'] = int(parts[offset+1]) if offset+1 < len(parts) else None
        data['sequence'] = int(parts[offset+2]) if offset+2 < len(parts) else None
        data['message_count'] = int(parts[offset+3]) if offset+3 < len(parts) else None
        data['connection_state'] = parts[offset+4] if offset+4 < len(parts) else None
        data['packet_loss'] = float(parts[offset+5]) if offset+5 < len(parts) else None
        data['led_state'] = int(parts[offset+6]) if offset+6 < len(parts) else None
        data['touch_state'] = int(parts[offset+7]) if offset+7 < len(parts) else None

        # Extended fields (optional, parse if present)
        idx = offset + 8

        # Battery monitoring
        data['battery_voltage'] = float(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['battery_percentage'] = float(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['battery_status'] = parts[idx] if idx < len(parts) and parts[idx] else None
        idx += 1

        # Current monitoring
        data['current_ma'] = float(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['bus_voltage'] = float(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['power_mw'] = float(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['energy_mah'] = float(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1

        # System telemetry
        data['uptime_seconds'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['free_heap'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['cpu_temperature'] = float(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['loop_frequency'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1

        # Fire alarm detection
        data['audio_detected'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['audio_rms'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['light_detected'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['light_red'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['light_green'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['light_blue'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['light_lux'] = float(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1

        # Advanced LoRa metrics
        data['spreading_factor'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1
        data['tx_power'] = int(parts[idx]) if idx < len(parts) and parts[idx] else None
        idx += 1

        return ('extended', data)

    except (ValueError, IndexError) as e:
        return None

def log_data(conn, data, format_type):
    """Log data to database (handles both basic and extended formats)"""
    cursor = conn.cursor()

    # Build column list and values based on available data
    columns = []
    values = []

    for key, value in data.items():
        if value is not None:
            columns.append(key)
            values.append(value)

    if not columns:
        return

    placeholders = ','.join(['?' for _ in values])
    column_str = ','.join(columns)

    query = f'INSERT INTO lora_messages ({column_str}) VALUES ({placeholders})'
    cursor.execute(query, values)
    conn.commit()

def log_event(conn, event_type, description, severity='INFO', device_role=None):
    """Log event to database"""
    cursor = conn.cursor()
    cursor.execute('''
        INSERT INTO events (event_type, severity, description, device_role)
        VALUES (?, ?, ?, ?)
    ''', (event_type, severity, description, device_role))
    conn.commit()

def get_statistics(conn):
    """Get current statistics from database"""
    cursor = conn.cursor()

    stats = {}

    # Total messages
    cursor.execute('SELECT COUNT(*) FROM lora_messages')
    stats['total_messages'] = cursor.fetchone()[0]

    # Average RSSI
    cursor.execute('SELECT AVG(rssi) FROM lora_messages WHERE rssi IS NOT NULL')
    avg_rssi = cursor.fetchone()[0]
    stats['avg_rssi'] = avg_rssi if avg_rssi else 0

    # Battery stats (if available)
    cursor.execute('SELECT AVG(battery_voltage), MIN(battery_voltage) FROM lora_messages WHERE battery_voltage IS NOT NULL')
    batt_stats = cursor.fetchone()
    stats['avg_battery'] = batt_stats[0] if batt_stats[0] else None
    stats['min_battery'] = batt_stats[1] if batt_stats[1] else None

    # Energy consumption (if available)
    cursor.execute('SELECT MAX(energy_mah) FROM lora_messages WHERE energy_mah IS NOT NULL')
    energy = cursor.fetchone()[0]
    stats['total_energy_mah'] = energy if energy else None

    # Fire alarm detections
    cursor.execute('SELECT COUNT(*) FROM lora_messages WHERE audio_detected = 1 OR light_detected = 1')
    stats['fire_alarms'] = cursor.fetchone()[0]

    # Connection states
    cursor.execute('''
        SELECT connection_state, COUNT(*)
        FROM lora_messages
        WHERE connection_state IS NOT NULL
        GROUP BY connection_state
    ''')
    stats['states'] = cursor.fetchall()

    return stats

def print_data_summary(data, format_type):
    """Print a one-line summary of received data"""
    timestamp = datetime.now().strftime("%H:%M:%S")

    # Connection state color
    state = data.get('connection_state', 'UNKNOWN')
    if state == 'OK':
        state_symbol = '✓'
        color = Colors.OKGREEN
    elif state == 'WEAK':
        state_symbol = '!'
        color = Colors.WARNING
    else:
        state_symbol = 'X'
        color = Colors.FAIL

    print(f"[{timestamp}] {color}{state_symbol}{Colors.ENDC} ", end='')
    print(f"{data.get('role', '??')} | ", end='')
    print(f"RSSI: {data.get('rssi', 0):4d} dBm | ", end='')
    print(f"Msg: {data.get('message_count', 0):4d} | ", end='')
    print(f"Loss: {data.get('packet_loss', 0):4.1f}%", end='')

    # Extended info if available
    if format_type == 'extended':
        if data.get('battery_voltage'):
            print(f" | Batt: {data['battery_voltage']:.2f}V", end='')
        if data.get('current_ma'):
            print(f" | I: {data['current_ma']:.1f}mA", end='')
        if data.get('audio_detected'):
            print(f" | 🔊 AUDIO", end='')
        if data.get('light_detected'):
            print(f" | 💡 LIGHT", end='')

    print()  # Newline

def main():
    # Get parameters from arguments
    if len(sys.argv) > 1:
        port = sys.argv[1]
    else:
        import os
        if os.name == 'nt':
            port = 'COM3'
        else:
            port = '/dev/ttyUSB0'

    if len(sys.argv) > 2:
        baudrate = int(sys.argv[2])
    else:
        baudrate = 115200

    if len(sys.argv) > 3:
        db_file = sys.argv[3]
    else:
        db_file = 'lora_extended.db'

    print_colored("╔═══════════════════════════════════════╗", Colors.CYAN)
    print_colored("║   LoRa Data Logger - Extended v2.0   ║", Colors.CYAN)
    print_colored("╚═══════════════════════════════════════╝", Colors.CYAN)
    print(f"\nSerial port: {port} @ {baudrate} baud")
    print(f"Database:    {db_file}")
    print("Press Ctrl+C to exit\n")

    try:
        # Create/open database
        conn = create_database(db_file)

        # Open serial connection
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)

        print_colored("✓ Connected! Logging data...", Colors.OKGREEN)
        print_colored("ℹ️  Auto-detecting CSV format (basic or extended)\n", Colors.BLUE)

        log_event(conn, 'START', 'Extended data logging started', 'INFO')

        message_count = 0
        last_stats_time = time.time()
        format_detected = None

        while True:
            try:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()

                    if not line:
                        continue

                    # Check if it's CSV data
                    if line.startswith('DATA_CSV'):
                        # Try extended format first, then basic
                        result = parse_csv_extended(line)
                        if not result:
                            result = parse_csv_basic(line)

                        if result:
                            format_type, data = result

                            # Notify on first format detection
                            if format_detected is None:
                                format_detected = format_type
                                print_colored(f"✓ CSV format detected: {format_type.upper()}", Colors.OKGREEN)
                                log_event(conn, 'FORMAT_DETECT', f'CSV format: {format_type}', 'INFO')

                            log_data(conn, data, format_type)
                            message_count += 1
                            print_data_summary(data, format_type)

                    # Log important events
                    elif '🔴 KILL SWITCH' in line or 'RESTART' in line:
                        log_event(conn, 'KILLSWITCH', line, 'CRITICAL')
                        print_colored(f"\n{line}", Colors.FAIL)
                    elif 'CONNECTION STATE CHANGE' in line or 'LOST' in line:
                        log_event(conn, 'STATE_CHANGE', line, 'WARNING')
                        print_colored(f"\n{line}", Colors.WARNING)
                    elif 'FIRE' in line or 'ALARM' in line or 'SMOKE' in line:
                        log_event(conn, 'FIRE_ALARM', line, 'CRITICAL')
                        print_colored(f"\n🔥 {line}", Colors.FAIL)
                    elif 'BATTERY' in line and ('LOW' in line or 'CRITICAL' in line):
                        log_event(conn, 'BATTERY_LOW', line, 'WARNING')
                        print_colored(f"\n🔋 {line}", Colors.WARNING)

                # Print statistics every 30 seconds
                if time.time() - last_stats_time > 30:
                    last_stats_time = time.time()
                    stats = get_statistics(conn)
                    print(f"\n{'='*60}")
                    print(f"Statistics:")
                    print(f"  Total logged:     {stats['total_messages']}")
                    print(f"  Avg RSSI:         {stats['avg_rssi']:.1f} dBm")
                    if stats['avg_battery']:
                        print(f"  Avg Battery:      {stats['avg_battery']:.2f}V (min: {stats['min_battery']:.2f}V)")
                    if stats['total_energy_mah']:
                        print(f"  Energy consumed:  {stats['total_energy_mah']:.1f} mAh")
                    if stats['fire_alarms'] > 0:
                        print(f"  Fire alarms:      {stats['fire_alarms']}")
                    print(f"  States:           {dict(stats['states'])}")
                    print(f"{'='*60}\n")

            except UnicodeDecodeError:
                pass

    except serial.SerialException as e:
        print_colored(f"\n❌ Serial Error: {e}", Colors.FAIL)
        sys.exit(1)

    except KeyboardInterrupt:
        print_colored("\n\n✓ Logging stopped", Colors.OKGREEN)
        log_event(conn, 'STOP', 'Extended data logging stopped', 'INFO')

        # Final statistics
        stats = get_statistics(conn)
        print(f"\nFinal Statistics:")
        print(f"  Total logged:     {stats['total_messages']}")
        print(f"  Avg RSSI:         {stats['avg_rssi']:.1f} dBm")
        if stats['avg_battery']:
            print(f"  Battery:          {stats['avg_battery']:.2f}V (min: {stats['min_battery']:.2f}V)")
        if stats['total_energy_mah']:
            print(f"  Energy consumed:  {stats['total_energy_mah']:.1f} mAh")
        if stats['fire_alarms'] > 0:
            print(f"  Fire alarms:      {stats['fire_alarms']}")
        print(f"  Database:         {db_file}")

        conn.close()
        ser.close()
        sys.exit(0)

if __name__ == "__main__":
    main()
