/*=====================================================================
  battery_monitor.h - Battery Voltage Monitoring

  FEATURE 1: Battery Monitoring

  Monitors battery voltage via ADC and reports warnings when low.
  Useful for portable/battery-powered deployments.

  Hardware Setup:
  - Connect battery positive (+) to voltage divider
  - Voltage divider: R1 (10kΩ) to GPIO 35, R2 (10kΩ) to GND
  - This creates 2:1 divider (max 6.6V input for 3.3V ADC)
  - Example: 4.2V LiPo → 2.1V at GPIO 35

  Pin Assignment:
  - GPIO 35 (ADC1_CH7) - Chosen because:
    * ADC1 channels work with WiFi (ADC2 does NOT!)
    * Input-only pin, no conflicts
    * 12-bit resolution (0-4095)

  Calibration:
  - Measure actual battery voltage with multimeter
  - Measure voltage at GPIO 35
  - Adjust BATTERY_VOLTAGE_DIVIDER in config.h

  Testing:
  1. Set ENABLE_BATTERY_MONITOR true in config.h
  2. Connect battery with voltage divider
  3. Upload and check serial output every 60s
  4. Should see: "🔋 Battery: 3.85V (OK)"

  Warning Levels:
  - > 3.3V: OK (green)
  - 3.0-3.3V: LOW (yellow warning)
  - < 3.0V: CRITICAL (red warning, consider shutdown)
=======================================================================*/

#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>
#include "config.h"

// Battery monitoring state
struct BatteryStatus {
  float voltage;              // Current voltage (V)
  float voltageMin;           // Minimum seen this session
  float voltageMax;           // Maximum seen this session
  int rawADC;                 // Raw ADC reading (0-4095)
  unsigned long lastCheck;    // Last check timestamp
  bool isLow;                 // Battery low warning
  bool isCritical;            // Battery critical warning
  int checkCount;             // Number of checks performed
};

BatteryStatus battery = {0.0, 999.0, 0.0, 0, 0, false, false, 0};

// Initialize battery monitoring
void initBatteryMonitor() {
  #if ENABLE_BATTERY_MONITOR
    pinMode(BATTERY_PIN, INPUT);

    // Configure ADC
    analogSetAttenuation(ADC_11db);  // 0-3.3V range
    analogReadResolution(12);         // 12-bit resolution (0-4095)

    Serial.println("✓ Battery monitor initialized");
    Serial.print("  Pin: GPIO ");
    Serial.println(BATTERY_PIN);
    Serial.print("  Voltage divider: 1:");
    Serial.println(BATTERY_VOLTAGE_DIVIDER);
    Serial.print("  Low threshold: ");
    Serial.print(BATTERY_LOW_THRESHOLD);
    Serial.println(" V");
    Serial.print("  Critical threshold: ");
    Serial.print(BATTERY_CRITICAL_THRESHOLD);
    Serial.println(" V");
  #endif
}

// Read battery voltage
float readBatteryVoltage() {
  #if ENABLE_BATTERY_MONITOR
    // Read ADC multiple times and average (reduce noise)
    const int samples = 10;
    int total = 0;

    for (int i = 0; i < samples; i++) {
      total += analogRead(BATTERY_PIN);
      delay(1);  // Small delay between readings
    }

    battery.rawADC = total / samples;

    // Convert ADC to voltage
    // ADC: 0-4095 = 0-3.3V at pin
    // Actual battery voltage = pin voltage × divider ratio
    float pinVoltage = (battery.rawADC / 4095.0) * 3.3;
    battery.voltage = pinVoltage * BATTERY_VOLTAGE_DIVIDER;

    // Update min/max
    if (battery.voltage < battery.voltageMin) {
      battery.voltageMin = battery.voltage;
    }
    if (battery.voltage > battery.voltageMax) {
      battery.voltageMax = battery.voltage;
    }

    // Check thresholds
    battery.isLow = (battery.voltage < BATTERY_LOW_THRESHOLD);
    battery.isCritical = (battery.voltage < BATTERY_CRITICAL_THRESHOLD);

    return battery.voltage;
  #else
    return 0.0;  // Feature disabled
  #endif
}

// Check battery and print status
void checkBattery() {
  #if ENABLE_BATTERY_MONITOR
    unsigned long now = millis();

    // Check at specified interval
    if (now - battery.lastCheck < BATTERY_CHECK_INTERVAL) {
      return;
    }

    battery.lastCheck = now;
    battery.checkCount++;

    // Read voltage
    float voltage = readBatteryVoltage();

    // Print status
    Serial.print("🔋 Battery ");
    Serial.print(battery.checkCount);
    Serial.print(": ");
    Serial.print(voltage, 2);
    Serial.print(" V");

    // Status indicator
    if (battery.isCritical) {
      Serial.print(" ⚠️ CRITICAL!");
      Serial.print(" (below ");
      Serial.print(BATTERY_CRITICAL_THRESHOLD);
      Serial.println(" V)");
    } else if (battery.isLow) {
      Serial.print(" ⚠️ LOW");
      Serial.print(" (below ");
      Serial.print(BATTERY_LOW_THRESHOLD);
      Serial.println(" V)");
    } else {
      Serial.println(" ✓ OK");
    }

    // Print raw ADC for debugging
    Serial.print("  Raw ADC: ");
    Serial.print(battery.rawADC);
    Serial.print(" / 4095");

    // Print range
    Serial.print(", Range: ");
    Serial.print(battery.voltageMin, 2);
    Serial.print(" - ");
    Serial.print(battery.voltageMax, 2);
    Serial.println(" V");
  #endif
}

// Get battery status string for CSV output
String getBatteryStatus() {
  #if ENABLE_BATTERY_MONITOR
    return String(battery.voltage, 2);
  #else
    return "0.00";
  #endif
}

// Get battery level percentage (0-100%)
// Assumes LiPo battery: 3.0V = 0%, 4.2V = 100%
int getBatteryPercentage() {
  #if ENABLE_BATTERY_MONITOR
    const float MIN_VOLTAGE = 3.0;
    const float MAX_VOLTAGE = 4.2;

    if (battery.voltage <= MIN_VOLTAGE) return 0;
    if (battery.voltage >= MAX_VOLTAGE) return 100;

    float percentage = ((battery.voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.0;
    return (int)percentage;
  #else
    return 0;
  #endif
}

// Check if battery is critically low (should shutdown)
bool shouldShutdownBattery() {
  #if ENABLE_BATTERY_MONITOR
    return battery.isCritical;
  #else
    return false;
  #endif
}

#endif // BATTERY_MONITOR_H
