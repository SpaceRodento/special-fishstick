#!/usr/bin/env python3
"""
analyze_data.py - LoRa Data Analysis Tool

Analyzes SQLite database created by data_logger.py and generates:
- Statistical summary (RSSI, SNR, packet loss, connection states)
- RSSI timeline plot with quality zones
- Packet loss over time plot
- RSSI distribution histogram
- Connection state timeline
- Optional PDF report

Requirements:
    pip install pandas matplotlib

Usage:
    python analyze_data.py lora_data.db
    python analyze_data.py lora_data.db --output report.pdf
    python analyze_data.py lora_data.db --no-plots   # Summary only
    python analyze_data.py lora_data.db --save-csv results.csv

Author: Roboter Gruppe 9
Date: 2025-11-05
"""

import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime
import argparse
import sys

# Color schemes for plots
COLORS = {
    'rssi_excellent': '#2ecc71',  # Green
    'rssi_good': '#f39c12',       # Orange
    'rssi_weak': '#e74c3c',       # Red
    'rssi_line': '#3498db',       # Blue
    'packet_loss': '#e74c3c',     # Red
    'snr': '#9b59b6',             # Purple
    'background': '#ecf0f1'       # Light gray
}

def load_data(db_path):
    """Load data from SQLite database."""
    try:
        conn = sqlite3.connect(db_path)
        df = pd.read_sql_query("""
            SELECT
                timestamp,
                esp_timestamp,
                role,
                rssi,
                snr,
                sequence,
                message_count,
                connection_state,
                packet_loss,
                led_state,
                touch_state
            FROM lora_messages
            ORDER BY timestamp
        """, conn)

        # Convert timestamp to datetime
        df['timestamp'] = pd.to_datetime(df['timestamp'])

        conn.close()
        return df
    except sqlite3.Error as e:
        print(f"❌ Database error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"❌ Error loading data: {e}")
        sys.exit(1)

def print_summary(df):
    """Print statistical summary of the data."""
    print("\n" + "="*60)
    print("📊 DATA ANALYSIS SUMMARY")
    print("="*60)

    # Basic info
    print(f"\n📅 Time Range:")
    print(f"   Start: {df['timestamp'].min()}")
    print(f"   End:   {df['timestamp'].max()}")
    duration = (df['timestamp'].max() - df['timestamp'].min()).total_seconds()
    print(f"   Duration: {duration/3600:.1f} hours ({duration/60:.0f} minutes)")

    print(f"\n📦 Data Points:")
    print(f"   Total messages: {len(df)}")
    print(f"   Role: {df['role'].iloc[0]}")

    # RSSI statistics
    print(f"\n📡 RSSI Statistics:")
    print(f"   Average: {df['rssi'].mean():.1f} dBm")
    print(f"   Minimum: {df['rssi'].min()} dBm")
    print(f"   Maximum: {df['rssi'].max()} dBm")
    print(f"   Std Dev: {df['rssi'].std():.1f} dBm")

    # Quality distribution
    excellent = len(df[df['rssi'] >= -80])
    good = len(df[(df['rssi'] >= -100) & (df['rssi'] < -80)])
    weak = len(df[df['rssi'] < -100])
    total = len(df)

    print(f"\n   Quality Distribution:")
    print(f"   Excellent (≥-80 dBm):  {excellent:4d} ({excellent/total*100:5.1f}%)")
    print(f"   Good (-100 to -80):    {good:4d} ({good/total*100:5.1f}%)")
    print(f"   Weak (<-100 dBm):      {weak:4d} ({weak/total*100:5.1f}%)")

    # SNR statistics
    print(f"\n📊 SNR Statistics:")
    print(f"   Average: {df['snr'].mean():.1f} dB")
    print(f"   Minimum: {df['snr'].min()} dB")
    print(f"   Maximum: {df['snr'].max()} dB")

    # Packet loss
    print(f"\n📉 Packet Loss:")
    print(f"   Average: {df['packet_loss'].mean():.2f}%")
    print(f"   Maximum: {df['packet_loss'].max():.2f}%")
    print(f"   Final: {df['packet_loss'].iloc[-1]:.2f}%")

    # Connection states
    print(f"\n🔗 Connection States:")
    state_counts = df['connection_state'].value_counts()
    for state, count in state_counts.items():
        print(f"   {state:10s}: {count:4d} ({count/total*100:5.1f}%)")

    # Messages
    print(f"\n📨 Messages:")
    print(f"   Total sent: {df['message_count'].max()}")
    print(f"   Rate: {df['message_count'].max() / (duration/60):.1f} msg/min")

    # LED & Touch
    led_on = len(df[df['led_state'] == 1])
    touch_active = len(df[df['touch_state'] == 1])
    print(f"\n💡 LED State:")
    print(f"   ON:  {led_on:4d} ({led_on/total*100:5.1f}%)")
    print(f"   OFF: {total-led_on:4d} ({(total-led_on)/total*100:5.1f}%)")
    print(f"\n👆 Touch Sensor:")
    print(f"   Active:   {touch_active:4d} ({touch_active/total*100:5.1f}%)")
    print(f"   Inactive: {total-touch_active:4d} ({(total-touch_active)/total*100:5.1f}%)")

    print("\n" + "="*60 + "\n")

def plot_rssi_timeline(df, ax):
    """Plot RSSI over time with quality zones."""
    ax.set_title('RSSI Signal Strength Over Time', fontsize=14, fontweight='bold')

    # Background zones
    ax.axhspan(-120, -100, alpha=0.2, color=COLORS['rssi_weak'], label='Weak (<-100 dBm)')
    ax.axhspan(-100, -80, alpha=0.2, color=COLORS['rssi_good'], label='Good (-100 to -80 dBm)')
    ax.axhspan(-80, -40, alpha=0.2, color=COLORS['rssi_excellent'], label='Excellent (≥-80 dBm)')

    # Plot RSSI line
    ax.plot(df['timestamp'], df['rssi'], color=COLORS['rssi_line'], linewidth=1.5, label='RSSI')

    # Format x-axis
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax.xaxis.set_major_locator(mdates.AutoDateLocator())
    plt.setp(ax.xaxis.get_majorticklabels(), rotation=45, ha='right')

    ax.set_xlabel('Time', fontsize=11)
    ax.set_ylabel('RSSI (dBm)', fontsize=11)
    ax.set_ylim(-120, -40)
    ax.grid(True, alpha=0.3, linestyle='--')
    ax.legend(loc='lower left', fontsize=9)

def plot_packet_loss(df, ax):
    """Plot packet loss over time."""
    ax.set_title('Packet Loss Over Time', fontsize=14, fontweight='bold')

    ax.plot(df['timestamp'], df['packet_loss'], color=COLORS['packet_loss'], linewidth=2, label='Packet Loss')
    ax.fill_between(df['timestamp'], df['packet_loss'], alpha=0.3, color=COLORS['packet_loss'])

    # Format x-axis
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax.xaxis.set_major_locator(mdates.AutoDateLocator())
    plt.setp(ax.xaxis.get_majorticklabels(), rotation=45, ha='right')

    ax.set_xlabel('Time', fontsize=11)
    ax.set_ylabel('Packet Loss (%)', fontsize=11)
    ax.set_ylim(0, max(5, df['packet_loss'].max() * 1.1))  # At least 0-5% range
    ax.grid(True, alpha=0.3, linestyle='--')
    ax.legend(loc='upper left', fontsize=9)

def plot_rssi_histogram(df, ax):
    """Plot RSSI distribution histogram."""
    ax.set_title('RSSI Distribution', fontsize=14, fontweight='bold')

    # Create histogram
    n, bins, patches = ax.hist(df['rssi'], bins=30, edgecolor='black', alpha=0.7)

    # Color bars by quality zone
    for i, patch in enumerate(patches):
        if bins[i] >= -80:
            patch.set_facecolor(COLORS['rssi_excellent'])
        elif bins[i] >= -100:
            patch.set_facecolor(COLORS['rssi_good'])
        else:
            patch.set_facecolor(COLORS['rssi_weak'])

    # Add vertical lines for mean and median
    mean_rssi = df['rssi'].mean()
    median_rssi = df['rssi'].median()
    ax.axvline(mean_rssi, color='red', linestyle='--', linewidth=2, label=f'Mean: {mean_rssi:.1f} dBm')
    ax.axvline(median_rssi, color='blue', linestyle='--', linewidth=2, label=f'Median: {median_rssi:.1f} dBm')

    ax.set_xlabel('RSSI (dBm)', fontsize=11)
    ax.set_ylabel('Frequency', fontsize=11)
    ax.grid(True, alpha=0.3, linestyle='--', axis='y')
    ax.legend(loc='upper left', fontsize=9)

def plot_connection_states(df, ax):
    """Plot connection state timeline."""
    ax.set_title('Connection State Timeline', fontsize=14, fontweight='bold')

    # Map states to numeric values for plotting
    state_map = {'UNKNOWN': 0, 'CONNECT': 1, 'OK': 2, 'WEAK': 1, 'LOST': 0}
    state_colors = {'UNKNOWN': '#95a5a6', 'CONNECT': '#3498db', 'OK': '#2ecc71', 'WEAK': '#f39c12', 'LOST': '#e74c3c'}

    df['state_numeric'] = df['connection_state'].map(state_map)

    # Plot as filled area
    for state in df['connection_state'].unique():
        mask = df['connection_state'] == state
        ax.fill_between(df[mask]['timestamp'], 0, df[mask]['state_numeric'],
                        color=state_colors.get(state, '#95a5a6'), alpha=0.6, label=state)

    # Format x-axis
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax.xaxis.set_major_locator(mdates.AutoDateLocator())
    plt.setp(ax.xaxis.get_majorticklabels(), rotation=45, ha='right')

    ax.set_xlabel('Time', fontsize=11)
    ax.set_ylabel('Connection Quality', fontsize=11)
    ax.set_ylim(-0.1, 2.5)
    ax.set_yticks([0, 1, 2])
    ax.set_yticklabels(['Lost', 'Weak', 'Good'])
    ax.grid(True, alpha=0.3, linestyle='--', axis='x')
    ax.legend(loc='upper left', fontsize=9, ncol=5)

def generate_plots(df, output_path=None):
    """Generate all plots."""
    print("📊 Generating plots...")

    # Create figure with 4 subplots
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    fig.suptitle('LoRa Communication Analysis Report', fontsize=16, fontweight='bold')

    # Generate each plot
    plot_rssi_timeline(df, axes[0, 0])
    plot_packet_loss(df, axes[0, 1])
    plot_rssi_histogram(df, axes[1, 0])
    plot_connection_states(df, axes[1, 1])

    # Add timestamp
    fig.text(0.99, 0.01, f'Generated: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}',
             ha='right', fontsize=8, style='italic')

    plt.tight_layout()

    # Save or show
    if output_path:
        plt.savefig(output_path, dpi=300, bbox_inches='tight')
        print(f"✓ Plots saved to: {output_path}")
    else:
        print("✓ Displaying plots (close window to continue)...")
        plt.show()

def main():
    parser = argparse.ArgumentParser(
        description='Analyze LoRa data from SQLite database',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python analyze_data.py lora_data.db
  python analyze_data.py lora_data.db --output report.pdf
  python analyze_data.py lora_data.db --no-plots
  python analyze_data.py lora_data.db --save-csv results.csv
        """
    )

    parser.add_argument('database', help='Path to SQLite database file')
    parser.add_argument('--output', '-o', help='Save plots to file (PDF, PNG, SVG)')
    parser.add_argument('--no-plots', action='store_true', help='Skip plot generation')
    parser.add_argument('--save-csv', help='Export data to CSV file')

    args = parser.parse_args()

    # Check if database exists
    try:
        with open(args.database, 'r'):
            pass
    except FileNotFoundError:
        print(f"❌ Error: Database file not found: {args.database}")
        sys.exit(1)

    print(f"📂 Loading data from: {args.database}")
    df = load_data(args.database)

    if len(df) == 0:
        print("❌ Error: No data found in database")
        sys.exit(1)

    print(f"✓ Loaded {len(df)} data points")

    # Print summary
    print_summary(df)

    # Export to CSV if requested
    if args.save_csv:
        df.to_csv(args.save_csv, index=False)
        print(f"✓ Data exported to: {args.save_csv}")

    # Generate plots unless disabled
    if not args.no_plots:
        generate_plots(df, args.output)

    print("\n✅ Analysis complete!")

if __name__ == '__main__':
    main()
