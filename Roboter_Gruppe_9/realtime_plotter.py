#!/usr/bin/env python3
"""
realtime_plotter.py - Real-time LoRa Data Visualization

Connects to ESP32 via USB serial and displays live graphs of:
- RSSI signal strength over time
- SNR (Signal-to-Noise Ratio) over time
- Packet loss percentage
- Connection state timeline

Requirements:
    pip install pyserial matplotlib

Usage:
    python realtime_plotter.py /dev/ttyUSB0
    python realtime_plotter.py COM3              # Windows
    python realtime_plotter.py /dev/ttyUSB0 --baud 115200
    python realtime_plotter.py /dev/ttyUSB0 --window 200  # Show last 200 points

Controls:
    - Close window to stop
    - Plots update automatically every 100ms

Author: Roboter Gruppe 9
Date: 2025-11-05
"""

import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.patches import Rectangle
from collections import deque
import argparse
import sys
import time

# Data buffers (use deque for efficient rolling window)
timestamps = deque(maxlen=100)
rssi_values = deque(maxlen=100)
snr_values = deque(maxlen=100)
packet_loss = deque(maxlen=100)
connection_states = deque(maxlen=100)

# State mapping for visualization
STATE_MAP = {
    'UNKNOWN': 0,
    'CONNECT': 1,
    'OK': 2,
    'WEAK': 1.5,
    'LOST': 0
}

STATE_COLORS = {
    'UNKNOWN': '#95a5a6',  # Gray
    'CONNECT': '#3498db',  # Blue
    'OK': '#2ecc71',       # Green
    'WEAK': '#f39c12',     # Orange
    'LOST': '#e74c3c'      # Red
}

# Statistics
stats = {
    'packets_received': 0,
    'parse_errors': 0,
    'start_time': time.time(),
    'last_rssi': 0,
    'last_snr': 0,
    'last_loss': 0.0,
    'last_state': 'UNKNOWN'
}

def parse_csv_line(line):
    """Parse CSV line from ESP32."""
    try:
        # Expected format: TIMESTAMP,RSSI,SNR,SEQ,LED,TOUCH,CONN_STATE,PACKET_LOSS
        # Example: 12345,-85,7,42,1,0,OK,1.23
        parts = line.strip().split(',')

        if len(parts) < 8:
            return None

        data = {
            'timestamp': int(parts[0]) / 1000.0,  # Convert ms to seconds
            'rssi': int(parts[1]),
            'snr': int(parts[2]),
            'sequence': int(parts[3]),
            'led': int(parts[4]),
            'touch': int(parts[5]),
            'state': parts[6],
            'loss': float(parts[7])
        }

        return data
    except (ValueError, IndexError) as e:
        stats['parse_errors'] += 1
        return None

def update_plot(frame, ser, ax1, ax2, ax3, ax4, text_status):
    """Update all plots with new data."""
    try:
        # Read all available lines (non-blocking)
        while ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()

                # Skip empty lines and debug messages
                if not line or line.startswith('#') or line.startswith('✓') or line.startswith('❌'):
                    continue

                # Parse CSV data
                data = parse_csv_line(line)
                if data is None:
                    continue

                # Add to buffers
                timestamps.append(data['timestamp'])
                rssi_values.append(data['rssi'])
                snr_values.append(data['snr'])
                packet_loss.append(data['loss'])
                connection_states.append(STATE_MAP.get(data['state'], 0))

                # Update statistics
                stats['packets_received'] += 1
                stats['last_rssi'] = data['rssi']
                stats['last_snr'] = data['snr']
                stats['last_loss'] = data['loss']
                stats['last_state'] = data['state']

            except UnicodeDecodeError:
                continue

        # Only update plots if we have data
        if len(timestamps) == 0:
            return

        # Convert deques to lists for plotting
        ts_list = list(timestamps)
        rssi_list = list(rssi_values)
        snr_list = list(snr_values)
        loss_list = list(packet_loss)
        state_list = list(connection_states)

        # Calculate relative time (last point = 0)
        if len(ts_list) > 0:
            time_offset = ts_list[-1]
            rel_times = [t - time_offset for t in ts_list]
        else:
            rel_times = []

        # --- Plot 1: RSSI ---
        ax1.clear()
        ax1.plot(rel_times, rssi_list, 'b-', linewidth=1.5, label='RSSI')

        # Quality zones
        ax1.axhspan(-120, -100, alpha=0.15, color='red', label='Weak')
        ax1.axhspan(-100, -80, alpha=0.15, color='orange', label='Good')
        ax1.axhspan(-80, -40, alpha=0.15, color='green', label='Excellent')

        ax1.set_ylabel('RSSI (dBm)', fontsize=10, fontweight='bold')
        ax1.set_ylim(-120, -40)
        ax1.grid(True, alpha=0.3, linestyle='--')
        ax1.legend(loc='lower left', fontsize=8, ncol=4)
        ax1.set_title('Signal Strength (RSSI)', fontsize=11, fontweight='bold')

        # --- Plot 2: SNR ---
        ax2.clear()
        ax2.plot(rel_times, snr_list, 'g-', linewidth=1.5, label='SNR')
        ax2.axhline(y=0, color='red', linestyle='--', linewidth=1, alpha=0.5)
        ax2.set_ylabel('SNR (dB)', fontsize=10, fontweight='bold')
        ax2.grid(True, alpha=0.3, linestyle='--')
        ax2.legend(loc='upper left', fontsize=8)
        ax2.set_title('Signal-to-Noise Ratio', fontsize=11, fontweight='bold')

        # --- Plot 3: Packet Loss ---
        ax3.clear()
        ax3.plot(rel_times, loss_list, 'r-', linewidth=1.5, label='Packet Loss')
        ax3.fill_between(rel_times, loss_list, alpha=0.3, color='red')
        ax3.set_ylabel('Loss (%)', fontsize=10, fontweight='bold')
        ax3.set_ylim(0, max(5, max(loss_list) * 1.1) if loss_list else 5)
        ax3.grid(True, alpha=0.3, linestyle='--')
        ax3.legend(loc='upper left', fontsize=8)
        ax3.set_title('Packet Loss', fontsize=11, fontweight='bold')

        # --- Plot 4: Connection State ---
        ax4.clear()

        # Color segments by state
        for i in range(len(state_list)):
            if i < len(state_list) - 1:
                color = STATE_COLORS.get(stats['last_state'], '#95a5a6')
                ax4.plot(rel_times[i:i+2], state_list[i:i+2],
                        color=color, linewidth=3, alpha=0.7)

        ax4.set_ylabel('State', fontsize=10, fontweight='bold')
        ax4.set_xlabel('Time (seconds ago)', fontsize=10, fontweight='bold')
        ax4.set_ylim(-0.2, 2.5)
        ax4.set_yticks([0, 1, 2])
        ax4.set_yticklabels(['LOST', 'WEAK', 'OK'])
        ax4.grid(True, alpha=0.3, linestyle='--', axis='x')
        ax4.set_title('Connection State', fontsize=11, fontweight='bold')

        # Update status text
        uptime = int(time.time() - stats['start_time'])
        status_text = (
            f"📊 Live Data Monitor  |  "
            f"Packets: {stats['packets_received']}  |  "
            f"Errors: {stats['parse_errors']}  |  "
            f"Uptime: {uptime}s  |  "
            f"RSSI: {stats['last_rssi']} dBm  |  "
            f"SNR: {stats['last_snr']} dB  |  "
            f"Loss: {stats['last_loss']:.1f}%  |  "
            f"State: {stats['last_state']}"
        )
        text_status.set_text(status_text)

    except Exception as e:
        print(f"Error in update: {e}")

def main():
    parser = argparse.ArgumentParser(
        description='Real-time LoRa data visualization from ESP32',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python realtime_plotter.py /dev/ttyUSB0
  python realtime_plotter.py COM3
  python realtime_plotter.py /dev/ttyUSB0 --baud 115200
  python realtime_plotter.py /dev/ttyUSB0 --window 200

Controls:
  Close the plot window to stop the program.
        """
    )

    parser.add_argument('port', help='Serial port (e.g., /dev/ttyUSB0, COM3)')
    parser.add_argument('--baud', '-b', type=int, default=115200,
                       help='Baud rate (default: 115200)')
    parser.add_argument('--window', '-w', type=int, default=100,
                       help='Number of data points to display (default: 100)')

    args = parser.parse_args()

    # Update buffer sizes
    global timestamps, rssi_values, snr_values, packet_loss, connection_states
    timestamps = deque(maxlen=args.window)
    rssi_values = deque(maxlen=args.window)
    snr_values = deque(maxlen=args.window)
    packet_loss = deque(maxlen=args.window)
    connection_states = deque(maxlen=args.window)

    # Connect to serial port
    print(f"📡 Connecting to {args.port} at {args.baud} baud...")
    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.1)
        time.sleep(2)  # Wait for connection to stabilize
        print(f"✓ Connected to {args.port}")
        print(f"✓ Waiting for data...")
        print(f"ℹ️  Expected format: TIMESTAMP,RSSI,SNR,SEQ,LED,TOUCH,STATE,LOSS")
        print(f"ℹ️  Close the plot window to stop\n")
    except serial.SerialException as e:
        print(f"❌ Error: Could not open serial port {args.port}")
        print(f"   {e}")
        print(f"\nTry:")
        print(f"  - Check if port exists: ls /dev/tty*")
        print(f"  - Check permissions: sudo chmod 666 {args.port}")
        print(f"  - Check if another program is using the port")
        sys.exit(1)

    # Create figure with 4 subplots
    fig = plt.figure(figsize=(14, 10))
    fig.canvas.manager.set_window_title('LoRa Real-time Monitor - Roboter Gruppe 9')

    # Create grid layout
    gs = fig.add_gridspec(4, 1, hspace=0.3, top=0.94, bottom=0.08, left=0.08, right=0.96)

    ax1 = fig.add_subplot(gs[0, 0])  # RSSI
    ax2 = fig.add_subplot(gs[1, 0])  # SNR
    ax3 = fig.add_subplot(gs[2, 0])  # Packet Loss
    ax4 = fig.add_subplot(gs[3, 0])  # Connection State

    # Status text at top
    text_status = fig.text(0.5, 0.97, 'Waiting for data...',
                          ha='center', va='top', fontsize=9,
                          family='monospace', bbox=dict(boxstyle='round',
                          facecolor='wheat', alpha=0.5))

    # Title
    fig.suptitle('📡 LoRa Communication - Real-time Monitor',
                fontsize=14, fontweight='bold')

    # Animation
    ani = animation.FuncAnimation(
        fig,
        update_plot,
        fargs=(ser, ax1, ax2, ax3, ax4, text_status),
        interval=100,  # Update every 100ms
        cache_frame_data=False
    )

    try:
        plt.show()
    except KeyboardInterrupt:
        print("\n⚠️  Interrupted by user")
    finally:
        ser.close()
        print("✓ Serial port closed")
        print(f"✓ Total packets received: {stats['packets_received']}")
        print(f"✓ Parse errors: {stats['parse_errors']}")
        print("✓ Real-time monitor stopped")

if __name__ == '__main__':
    main()
