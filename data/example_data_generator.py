#!/usr/bin/env python3
"""
Example Data Generator for LoRa Extended Telemetry

Generates realistic synthetic data for testing data analysis scripts.
Creates both CSV files and SQLite databases with sample data.

Simulates:
- Normal operation with varying signal quality
- Battery discharge over time
- Fire alarm detection events
- Connection quality changes
- System performance metrics

Usage:
    python example_data_generator.py [duration_minutes] [output_file]

Example:
    python example_data_generator.py 60 example_data.db    # 1 hour of data
    python example_data_generator.py 1440 day_test.db      # 24 hours of data
"""

import sqlite3
import random
import math
import sys
from datetime import datetime, timedelta

class LoRaDataGenerator:
    def __init__(self, db_file):
        self.db_file = db_file
        self.conn = None

        # Simulation state
        self.esp_timestamp = 0
        self.message_count = 0
        self.sequence = 0
        self.battery_voltage = 4.2  # Start fully charged
        self.energy_consumed = 0.0
        self.connection_state = 'OK'
        self.rssi_base = -75  # Base RSSI (varies around this)
        self.uptime = 0

        # Fire alarm simulation
        self.fire_alarm_active = False
        self.fire_alarm_start = None
        self.fire_alarm_duration = 0

    def create_database(self):
        """Create database with extended schema"""
        self.conn = sqlite3.connect(self.db_file)
        cursor = self.conn.cursor()

        # Use same schema as data_logger_extended.py
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS lora_messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                esp_timestamp INTEGER,
                role TEXT,
                device_address INTEGER,
                rssi INTEGER,
                snr INTEGER,
                sequence INTEGER,
                message_count INTEGER,
                connection_state TEXT,
                packet_loss REAL,
                led_state INTEGER,
                touch_state INTEGER,
                battery_voltage REAL,
                battery_percentage REAL,
                battery_status TEXT,
                current_ma REAL,
                bus_voltage REAL,
                shunt_voltage REAL,
                power_mw REAL,
                energy_mah REAL,
                energy_wh REAL,
                uptime_seconds INTEGER,
                free_heap INTEGER,
                heap_fragmentation REAL,
                cpu_temperature REAL,
                loop_frequency INTEGER,
                audio_detected INTEGER,
                audio_rms INTEGER,
                audio_peak_count INTEGER,
                light_detected INTEGER,
                light_red INTEGER,
                light_green INTEGER,
                light_blue INTEGER,
                light_clear INTEGER,
                light_lux REAL,
                retries INTEGER,
                duplicates INTEGER,
                out_of_order INTEGER,
                spreading_factor INTEGER,
                bandwidth INTEGER,
                coding_rate TEXT,
                tx_power INTEGER,
                cpu_usage REAL,
                task_stack_free INTEGER,
                wifi_status TEXT,
                adaptive_sf_active INTEGER,
                encryption_active INTEGER
            )
        ''')

        cursor.execute('''
            CREATE TABLE IF NOT EXISTS events (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                event_type TEXT,
                severity TEXT,
                description TEXT,
                device_role TEXT
            )
        ''')

        self.conn.commit()
        print(f"✓ Database created: {self.db_file}")

    def generate_rssi(self, distance_factor=1.0):
        """Generate realistic RSSI with noise and fading"""
        # Path loss + random fading + noise
        path_loss = -75 * distance_factor
        fading = random.gauss(0, 5)  # Slow fading
        noise = random.gauss(0, 2)   # Fast noise
        rssi = int(path_loss + fading + noise)
        return max(-120, min(-40, rssi))  # Clamp to valid range

    def generate_snr(self, rssi):
        """Generate SNR correlated with RSSI"""
        # Better RSSI typically means better SNR
        base_snr = (rssi + 100) / 5  # -100 dBm -> 0 dB, -50 dBm -> 10 dB
        snr = int(base_snr + random.gauss(0, 2))
        return max(-10, min(15, snr))

    def update_battery(self, current_ma, interval_seconds):
        """Update battery voltage based on current draw"""
        # Discharge battery (simplified model)
        energy_delta = (current_ma * interval_seconds) / 3600.0  # mAh
        self.energy_consumed += energy_delta

        # Battery capacity: ~3000 mAh (typical 18650 cell)
        battery_capacity = 3000.0
        discharge_percent = self.energy_consumed / battery_capacity

        # Voltage curve (LiPo/Li-ion)
        # 4.2V (100%) -> 3.7V (50%) -> 3.0V (0%)
        if discharge_percent < 0.5:
            self.battery_voltage = 4.2 - (discharge_percent * 1.0)
        else:
            self.battery_voltage = 3.7 - ((discharge_percent - 0.5) * 1.4)

        self.battery_voltage = max(3.0, self.battery_voltage)

        # Battery status
        if self.battery_voltage > 3.5:
            battery_status = 'OK'
        elif self.battery_voltage > 3.2:
            battery_status = 'LOW'
        else:
            battery_status = 'CRITICAL'

        battery_percentage = max(0, min(100, (self.battery_voltage - 3.0) / 1.2 * 100))

        return battery_status, battery_percentage

    def trigger_fire_alarm(self):
        """Simulate fire alarm detection"""
        self.fire_alarm_active = True
        self.fire_alarm_start = self.uptime
        self.fire_alarm_duration = random.randint(10, 60)  # 10-60 seconds

        # Log event
        cursor = self.conn.cursor()
        cursor.execute('''
            INSERT INTO events (event_type, severity, description, device_role)
            VALUES (?, ?, ?, ?)
        ''', ('FIRE_ALARM', 'CRITICAL', 'Fire alarm detected (audio + light)', 'RX'))
        self.conn.commit()

        print(f"🔥 Fire alarm triggered at t={self.uptime}s (duration: {self.fire_alarm_duration}s)")

    def check_fire_alarm(self):
        """Check if fire alarm should still be active"""
        if self.fire_alarm_active:
            if self.uptime - self.fire_alarm_start > self.fire_alarm_duration:
                self.fire_alarm_active = False
                print(f"✓ Fire alarm cleared at t={self.uptime}s")

    def generate_datapoint(self, start_time, interval=2):
        """Generate one datapoint"""
        self.uptime += interval
        self.esp_timestamp += interval * 1000  # milliseconds

        # Current timestamp
        current_time = start_time + timedelta(seconds=self.uptime)

        # Simulate distance variations (signal quality changes)
        time_factor = self.uptime / 3600.0  # hours
        distance_factor = 1.0 + 0.3 * math.sin(time_factor * math.pi / 2)  # Varies 0.7 - 1.3

        # Generate metrics
        rssi = self.generate_rssi(distance_factor)
        snr = self.generate_snr(rssi)

        # Connection state based on RSSI
        if rssi > -80:
            self.connection_state = 'OK'
        elif rssi > -100:
            self.connection_state = 'WEAK'
        else:
            self.connection_state = 'LOST'

        # Current draw (varies with transmission activity)
        base_current = 45  # Idle current
        tx_current = random.choice([0, 120])  # TX spike (occurs ~50% of time)
        current_ma = base_current + tx_current + random.gauss(0, 5)

        # Update battery
        battery_status, battery_percentage = self.update_battery(current_ma, interval)
        battery_voltage = self.battery_voltage  # Get updated voltage

        # Power metrics
        bus_voltage = battery_voltage
        shunt_voltage = (current_ma / 1000.0) * 0.1  # 0.1 ohm shunt
        power_mw = bus_voltage * current_ma
        energy_wh = self.energy_consumed * bus_voltage / 1000.0

        # System telemetry
        free_heap = 245000 - random.randint(0, 20000)  # Varies 225-245 KB
        heap_fragmentation = random.uniform(0, 15)
        cpu_temperature = 35 + (current_ma / 10) + random.gauss(0, 2)
        loop_frequency = 100 + random.randint(-5, 5)

        # Fire alarm detection (random events)
        self.check_fire_alarm()
        if not self.fire_alarm_active and random.random() < 0.001:  # 0.1% chance per sample
            self.trigger_fire_alarm()

        audio_detected = 1 if self.fire_alarm_active else 0
        audio_rms = random.randint(250, 400) if self.fire_alarm_active else random.randint(50, 100)
        audio_peak_count = random.randint(3, 5) if self.fire_alarm_active else 0

        light_detected = 1 if self.fire_alarm_active else 0
        light_red = random.randint(50000, 65000) if self.fire_alarm_active else random.randint(100, 1000)
        light_green = random.randint(100, 1000)
        light_blue = random.randint(100, 1000)
        light_clear = (light_red + light_green + light_blue) // 3
        light_lux = light_clear / 500.0

        # LoRa advanced metrics
        spreading_factor = 7  # Could vary with adaptive SF
        tx_power = 14
        bandwidth = 125
        coding_rate = '4/5'

        # Packet loss (increases with poor signal)
        if rssi > -80:
            packet_loss = random.uniform(0, 0.5)
        elif rssi > -100:
            packet_loss = random.uniform(0.5, 5.0)
        else:
            packet_loss = random.uniform(5.0, 20.0)

        # I/O states
        led_state = self.message_count % 2  # Toggles
        touch_state = 1 if random.random() < 0.1 else 0  # 10% touched

        # Increment counters
        self.sequence += 1
        self.message_count += 1

        # Insert into database
        cursor = self.conn.cursor()
        cursor.execute('''
            INSERT INTO lora_messages (
                timestamp, esp_timestamp, role, device_address,
                rssi, snr, sequence, message_count, connection_state, packet_loss,
                led_state, touch_state,
                battery_voltage, battery_percentage, battery_status,
                current_ma, bus_voltage, shunt_voltage, power_mw, energy_mah, energy_wh,
                uptime_seconds, free_heap, heap_fragmentation, cpu_temperature, loop_frequency,
                audio_detected, audio_rms, audio_peak_count,
                light_detected, light_red, light_green, light_blue, light_clear, light_lux,
                spreading_factor, bandwidth, coding_rate, tx_power,
                retries, duplicates, out_of_order
            ) VALUES (
                ?, ?, ?, ?,
                ?, ?, ?, ?, ?, ?,
                ?, ?,
                ?, ?, ?,
                ?, ?, ?, ?, ?, ?,
                ?, ?, ?, ?, ?,
                ?, ?, ?,
                ?, ?, ?, ?, ?, ?,
                ?, ?, ?, ?,
                ?, ?, ?
            )
        ''', (
            current_time.strftime('%Y-%m-%d %H:%M:%S'), self.esp_timestamp, 'RX', 1,
            rssi, snr, self.sequence, self.message_count, self.connection_state, packet_loss,
            led_state, touch_state,
            battery_voltage, battery_percentage, battery_status,
            current_ma, bus_voltage, shunt_voltage, power_mw, self.energy_consumed, energy_wh,
            self.uptime, free_heap, heap_fragmentation, cpu_temperature, loop_frequency,
            audio_detected, audio_rms, audio_peak_count,
            light_detected, light_red, light_green, light_blue, light_clear, light_lux,
            spreading_factor, bandwidth, coding_rate, tx_power,
            0, 0, 0  # retries, duplicates, out_of_order
        ))

    def generate(self, duration_minutes, interval=2):
        """Generate dataset for specified duration"""
        total_samples = int((duration_minutes * 60) / interval)
        start_time = datetime.now() - timedelta(minutes=duration_minutes)

        print(f"Generating {total_samples} samples ({duration_minutes} min @ {interval}s interval)...")

        # Generate data
        for i in range(total_samples):
            self.generate_datapoint(start_time, interval)

            # Commit every 100 samples
            if i % 100 == 0:
                self.conn.commit()
                progress = (i / total_samples) * 100
                print(f"  Progress: {progress:.1f}% ({i}/{total_samples})", end='\r')

        # Final commit
        self.conn.commit()
        print(f"\n✓ Generated {total_samples} samples")
        print(f"  Uptime:          {self.uptime/3600:.1f} hours")
        print(f"  Messages:        {self.message_count}")
        print(f"  Energy consumed: {self.energy_consumed:.1f} mAh ({self.energy_consumed * self.battery_voltage / 1000:.2f} Wh)")
        print(f"  Final battery:   {self.battery_voltage:.2f}V ({(self.battery_voltage - 3.0) / 1.2 * 100:.0f}%)")

    def export_csv(self, csv_file):
        """Export database to CSV"""
        import csv

        cursor = self.conn.cursor()
        cursor.execute('SELECT * FROM lora_messages')

        with open(csv_file, 'w', newline='') as f:
            writer = csv.writer(f)

            # Header
            writer.writerow([desc[0] for desc in cursor.description])

            # Data
            writer.writerows(cursor.fetchall())

        print(f"✓ Exported to CSV: {csv_file}")

def main():
    # Get parameters
    if len(sys.argv) > 1:
        duration_minutes = int(sys.argv[1])
    else:
        duration_minutes = 60  # Default: 1 hour

    if len(sys.argv) > 2:
        db_file = sys.argv[2]
    else:
        db_file = f'example_data_{duration_minutes}min.db'

    csv_file = db_file.replace('.db', '.csv')

    print("╔═══════════════════════════════════════════╗")
    print("║  LoRa Example Data Generator             ║")
    print("╚═══════════════════════════════════════════╝")
    print(f"\nDuration:     {duration_minutes} minutes")
    print(f"Database:     {db_file}")
    print(f"CSV export:   {csv_file}\n")

    # Generate data
    generator = LoRaDataGenerator(db_file)
    generator.create_database()

    # Add start event
    cursor = generator.conn.cursor()
    cursor.execute('''
        INSERT INTO events (event_type, severity, description, device_role)
        VALUES (?, ?, ?, ?)
    ''', ('START', 'INFO', 'Example data generation started', 'RX'))
    generator.conn.commit()

    # Generate data
    generator.generate(duration_minutes)

    # Add stop event
    cursor.execute('''
        INSERT INTO events (event_type, severity, description, device_role)
        VALUES (?, ?, ?, ?)
    ''', ('STOP', 'INFO', 'Example data generation completed', 'RX'))
    generator.conn.commit()

    # Export CSV
    generator.export_csv(csv_file)

    print(f"\n✅ Done! Use with:")
    print(f"   python analyze_data.py {db_file}")
    print(f"   sqlite3 {db_file}")

if __name__ == '__main__':
    main()
