#!/usr/bin/env python3
"""
LoRa Serial Monitor - Simple Data Reader

Reads serial data from ESP32 and displays it in real-time.
Shows both raw output and parsed CSV data.

Usage:
    python serial_monitor.py [port] [baudrate]

Example:
    python serial_monitor.py /dev/ttyUSB0 115200
    python serial_monitor.py COM3 115200
"""

import serial
import sys
import time
from datetime import datetime

# ANSI color codes for terminal
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def print_colored(text, color):
    """Print text in color"""
    print(f"{color}{text}{Colors.ENDC}")

def parse_csv_line(line):
    """Parse DATA_CSV line"""
    # Format: DATA_CSV,TIMESTAMP,ROLE,RSSI,SNR,SEQ,MSG_COUNT,CONN_STATE,PACKET_LOSS,LED,TOUCH
    parts = line.split(',')
    if len(parts) != 11:
        return None

    try:
        data = {
            'timestamp': int(parts[1]),
            'role': parts[2],
            'rssi': int(parts[3]),
            'snr': int(parts[4]),
            'seq': int(parts[5]),
            'msg_count': int(parts[6]),
            'conn_state': parts[7],
            'packet_loss': float(parts[8]),
            'led': int(parts[9]),
            'touch': int(parts[10])
        }
        return data
    except (ValueError, IndexError):
        return None

def display_data(data):
    """Display parsed data in a nice format"""
    timestamp = datetime.now().strftime("%H:%M:%S")

    # Connection state color
    state = data['conn_state']
    if state == 'CONNECTED':
        state_color = Colors.OKGREEN
    elif state == 'WEAK':
        state_color = Colors.WARNING
    elif state == 'LOST':
        state_color = Colors.FAIL
    else:
        state_color = Colors.OKCYAN

    # RSSI quality indicator
    rssi = data['rssi']
    if rssi > -60:
        rssi_indicator = "█████"
        rssi_color = Colors.OKGREEN
    elif rssi > -80:
        rssi_indicator = "████░"
        rssi_color = Colors.OKGREEN
    elif rssi > -100:
        rssi_indicator = "███░░"
        rssi_color = Colors.WARNING
    else:
        rssi_indicator = "██░░░"
        rssi_color = Colors.FAIL

    print(f"\n[{timestamp}] ", end='')
    print_colored(f"{data['role']}", Colors.OKBLUE)

    print(f"  State: {state_color}{state}{Colors.ENDC} | ", end='')
    print(f"RSSI: {rssi_color}{rssi} dBm {rssi_indicator}{Colors.ENDC} | ", end='')
    print(f"SNR: {data['snr']} dB")

    print(f"  Messages: {data['msg_count']} | ", end='')
    print(f"Seq: {data['seq']} | ", end='')
    print(f"Loss: {data['packet_loss']:.1f}% | ", end='')
    print(f"LED: {data['led']} | Touch: {data['touch']}")

def main():
    # Get port and baudrate from arguments or use defaults
    if len(sys.argv) > 1:
        port = sys.argv[1]
    else:
        # Try common ports
        import os
        if os.name == 'nt':  # Windows
            port = 'COM3'
        else:  # Linux/Mac
            port = '/dev/ttyUSB0'

    if len(sys.argv) > 2:
        baudrate = int(sys.argv[2])
    else:
        baudrate = 115200

    print_colored("╔═══════════════════════════════════════╗", Colors.HEADER)
    print_colored("║   LoRa Serial Monitor - v1.0         ║", Colors.HEADER)
    print_colored("╚═══════════════════════════════════════╝", Colors.HEADER)
    print(f"\nConnecting to: {port} @ {baudrate} baud")
    print("Press Ctrl+C to exit\n")

    try:
        # Open serial connection
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # Wait for connection

        print_colored("✓ Connected! Listening for data...\n", Colors.OKGREEN)

        last_data = None

        while True:
            try:
                # Read line from serial
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()

                    if not line:
                        continue

                    # Check if it's CSV data
                    if line.startswith('DATA_CSV,'):
                        data = parse_csv_line(line)
                        if data:
                            display_data(data)
                            last_data = data
                    else:
                        # Print raw line with color coding
                        if '✓' in line:
                            print_colored(line, Colors.OKGREEN)
                        elif '❌' in line or 'ERROR' in line or 'FAIL' in line:
                            print_colored(line, Colors.FAIL)
                        elif '⚠' in line or 'WARNING' in line or 'WEAK' in line:
                            print_colored(line, Colors.WARNING)
                        elif '║' in line or '╔' in line or '╚' in line or '╠' in line:
                            print_colored(line, Colors.OKCYAN)
                        elif '🔴' in line:
                            print_colored(line, Colors.FAIL)
                        else:
                            print(line)

            except UnicodeDecodeError:
                pass

    except serial.SerialException as e:
        print_colored(f"\n❌ Serial Error: {e}", Colors.FAIL)
        print(f"\nMake sure:")
        print(f"  1. ESP32 is connected to {port}")
        print(f"  2. Correct port selected")
        print(f"  3. No other program is using the port")
        sys.exit(1)

    except KeyboardInterrupt:
        print_colored("\n\n✓ Monitoring stopped", Colors.OKGREEN)
        ser.close()
        sys.exit(0)

if __name__ == "__main__":
    main()
