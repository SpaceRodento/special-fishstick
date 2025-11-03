#!/usr/bin/env python3
"""
LoRa Data Logger - Advanced Logger with Database

Reads serial data from ESP32 and logs to SQLite database.
Creates database if it doesn't exist.

Usage:
    python data_logger.py [port] [baudrate] [database_file]

Example:
    python data_logger.py /dev/ttyUSB0 115200 lora_data.db
    python data_logger.py COM3 115200
"""

import serial
import sqlite3
import sys
import time
from datetime import datetime

# ANSI color codes
class Colors:
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    CYAN = '\033[96m'

def print_colored(text, color):
    """Print text in color"""
    print(f"{color}{text}{Colors.ENDC}")

def create_database(db_file):
    """Create database and tables if they don't exist"""
    conn = sqlite3.connect(db_file)
    cursor = conn.cursor()

    cursor.execute('''
        CREATE TABLE IF NOT EXISTS lora_messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            esp_timestamp INTEGER,
            role TEXT,
            rssi INTEGER,
            snr INTEGER,
            sequence INTEGER,
            message_count INTEGER,
            connection_state TEXT,
            packet_loss REAL,
            led_state INTEGER,
            touch_state INTEGER
        )
    ''')

    cursor.execute('''
        CREATE TABLE IF NOT EXISTS events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            event_type TEXT,
            description TEXT
        )
    ''')

    # Create index for faster queries
    cursor.execute('''
        CREATE INDEX IF NOT EXISTS idx_timestamp
        ON lora_messages(timestamp)
    ''')

    conn.commit()
    return conn

def parse_csv_line(line):
    """Parse DATA_CSV line"""
    # Format: DATA_CSV,TIMESTAMP,ROLE,RSSI,SNR,SEQ,MSG_COUNT,CONN_STATE,PACKET_LOSS,LED,TOUCH
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
        return data
    except (ValueError, IndexError):
        return None

def log_data(conn, data):
    """Log data to database"""
    cursor = conn.cursor()
    cursor.execute('''
        INSERT INTO lora_messages
        (esp_timestamp, role, rssi, snr, sequence, message_count,
         connection_state, packet_loss, led_state, touch_state)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    ''', (
        data['esp_timestamp'],
        data['role'],
        data['rssi'],
        data['snr'],
        data['sequence'],
        data['message_count'],
        data['connection_state'],
        data['packet_loss'],
        data['led_state'],
        data['touch_state']
    ))
    conn.commit()

def log_event(conn, event_type, description):
    """Log event to database"""
    cursor = conn.cursor()
    cursor.execute('''
        INSERT INTO events (event_type, description)
        VALUES (?, ?)
    ''', (event_type, description))
    conn.commit()

def get_statistics(conn):
    """Get current statistics from database"""
    cursor = conn.cursor()

    # Total messages
    cursor.execute('SELECT COUNT(*) FROM lora_messages')
    total = cursor.fetchone()[0]

    # Average RSSI
    cursor.execute('SELECT AVG(rssi) FROM lora_messages WHERE rssi != 0')
    avg_rssi = cursor.fetchone()[0]

    # Connection state distribution
    cursor.execute('''
        SELECT connection_state, COUNT(*)
        FROM lora_messages
        GROUP BY connection_state
    ''')
    states = cursor.fetchall()

    return {
        'total_messages': total,
        'avg_rssi': avg_rssi if avg_rssi else 0,
        'states': states
    }

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
        db_file = 'lora_data.db'

    print_colored("╔═══════════════════════════════════════╗", Colors.CYAN)
    print_colored("║   LoRa Data Logger - v1.0            ║", Colors.CYAN)
    print_colored("╚═══════════════════════════════════════╝", Colors.CYAN)
    print(f"\nSerial port: {port} @ {baudrate} baud")
    print(f"Database:    {db_file}")
    print("Press Ctrl+C to exit\n")

    try:
        # Create/open database
        conn = create_database(db_file)
        print_colored(f"✓ Database ready: {db_file}", Colors.OKGREEN)

        # Open serial connection
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)

        print_colored("✓ Connected! Logging data...\n", Colors.OKGREEN)

        log_event(conn, 'START', 'Data logging started')

        message_count = 0
        last_stats_time = time.time()

        while True:
            try:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()

                    if not line:
                        continue

                    # Check if it's CSV data
                    if line.startswith('DATA_CSV,'):
                        data = parse_csv_line(line)
                        if data:
                            log_data(conn, data)
                            message_count += 1

                            # Print log entry
                            timestamp = datetime.now().strftime("%H:%M:%S")
                            state = data['connection_state']

                            if state == 'CONNECTED':
                                state_symbol = '✓'
                                color = Colors.OKGREEN
                            elif state == 'WEAK':
                                state_symbol = '!'
                                color = Colors.WARNING
                            else:
                                state_symbol = 'X'
                                color = Colors.FAIL

                            print(f"[{timestamp}] {color}{state_symbol}{Colors.ENDC} ", end='')
                            print(f"{data['role']} | ", end='')
                            print(f"RSSI: {data['rssi']:4d} dBm | ", end='')
                            print(f"Msg: {data['message_count']:4d} | ", end='')
                            print(f"Loss: {data['packet_loss']:4.1f}%")

                    # Log important events
                    elif '🔴 KILL SWITCH' in line:
                        log_event(conn, 'KILLSWITCH', line)
                        print_colored(f"\n{line}", Colors.FAIL)
                    elif 'CONNECTION STATE CHANGE' in line:
                        log_event(conn, 'STATE_CHANGE', line)
                        print_colored(f"\n{line}", Colors.WARNING)
                    elif 'Lost packets detected' in line:
                        log_event(conn, 'PACKET_LOSS', line)
                        print_colored(line, Colors.WARNING)

                # Print statistics every 30 seconds
                if time.time() - last_stats_time > 30:
                    last_stats_time = time.time()
                    stats = get_statistics(conn)
                    print(f"\n{'='*50}")
                    print(f"Statistics:")
                    print(f"  Total logged: {stats['total_messages']}")
                    print(f"  Avg RSSI:     {stats['avg_rssi']:.1f} dBm")
                    print(f"  States:       {dict(stats['states'])}")
                    print(f"{'='*50}\n")

            except UnicodeDecodeError:
                pass

    except serial.SerialException as e:
        print_colored(f"\n❌ Serial Error: {e}", Colors.FAIL)
        sys.exit(1)

    except KeyboardInterrupt:
        print_colored("\n\n✓ Logging stopped", Colors.OKGREEN)
        log_event(conn, 'STOP', 'Data logging stopped')

        # Final statistics
        stats = get_statistics(conn)
        print(f"\nFinal Statistics:")
        print(f"  Total logged: {stats['total_messages']}")
        print(f"  Avg RSSI:     {stats['avg_rssi']:.1f} dBm")
        print(f"  Database:     {db_file}")

        conn.close()
        ser.close()
        sys.exit(0)

if __name__ == "__main__":
    main()
