/*=====================================================================
  current_monitor.h - Current and Power Monitoring

  FEATURE 13: Current Monitoring

  Monitors battery current, voltage, and power consumption using INA219
  current sensor. Tracks total energy usage and calculates battery runtime.

  Hardware Setup:
  - INA219 Current Sensor Module
  - Connect to ESP32 I2C bus (same bus as TCS34725 light sensor):
    * VCC → 3.3V (or 5V if module has regulator)
    * GND → GND
    * SDA → GPIO 21 (I2C Data)
    * SCL → GPIO 22 (I2C Clock)
  - Battery connection through INA219:
    * Battery (+) → VIN+ terminal
    * ESP32 VIN   → VIN- terminal
    * This measures current flowing from battery to ESP32

  INA219 Specifications:
  - I2C Address: 0x40 (default) or 0x41/0x42/0x43 (configurable)
  - Voltage Range: 0-26V (bus voltage)
  - Current Range: ±3.2A (with 0.1Ω shunt, default)
  - Resolution: 0.1mA current, 4mV voltage
  - Power calculation: Voltage × Current

  Library Required:
  - Adafruit INA219 Library
  - Install via Arduino Library Manager: "Adafruit INA219"
  - Also installs dependencies: Adafruit BusIO

  Features:
  - Real-time current monitoring (mA)
  - Battery voltage measurement (V)
  - Power consumption (mW)
  - Total energy used (mAh, Wh)
  - Peak current tracking
  - Average current calculation
  - Estimated battery runtime

  Testing:
  1. Install Adafruit_INA219 library in Arduino IDE
  2. Set ENABLE_CURRENT_MONITOR true in config.h
  3. Connect INA219 as described above
  4. Upload and check serial output every 10s
  5. Should see: "⚡ Current: 85mA, Voltage: 3.85V, Power: 327mW"

  Typical ESP32 Current Draw:
  - Deep sleep: 10-150 μA
  - Light sleep: 0.8 mA
  - CPU active: 20-50 mA
  - WiFi on: 80-170 mA
  - WiFi transmit: 170-260 mA
  - LoRa transmit: 120-140 mA (depends on TX power)

  Example Usage:
    if (current.current_mA > 200) {
      Serial.println("High current detected!");
    }

    float runtime_hours = current.energyUsed_mAh / current.current_mA;
=======================================================================*/

#ifndef CURRENT_MONITOR_H
#define CURRENT_MONITOR_H

#include <Arduino.h>
#include "config.h"

#if ENABLE_CURRENT_MONITOR
  #include <Wire.h>
  #include <Adafruit_INA219.h>
#endif

// Current monitoring state
struct CurrentStatus {
  float voltage;              // Bus voltage (V)
  float current_mA;           // Current (mA)
  float power_mW;             // Power (mW)
  float shuntVoltage_mV;      // Shunt voltage (mV) - for debugging

  // Statistics
  float currentMin;           // Minimum current this session (mA)
  float currentMax;           // Maximum current this session (mA)
  float currentAvg;           // Average current (mA)
  float powerMax;             // Peak power (mW)

  // Energy tracking
  float energyUsed_mAh;       // Total energy consumed (mAh)
  float energyUsed_Wh;        // Total energy consumed (Wh)
  unsigned long totalTime_ms; // Total tracking time

  // Timing
  unsigned long lastCheck;    // Last check timestamp
  unsigned long lastReset;    // Last statistics reset
  int checkCount;             // Number of checks performed

  // Warnings
  bool isHighCurrent;         // Current exceeds warning threshold
  bool isOverload;            // Current exceeds max safe current
};

CurrentStatus current = {0.0, 0.0, 0.0, 0.0, 999999.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, false, false};

#if ENABLE_CURRENT_MONITOR
  Adafruit_INA219 ina219;
#endif

// Initialize current monitoring
void initCurrentMonitor() {
  #if ENABLE_CURRENT_MONITOR
    Serial.println("\n=== Initializing Current Monitor ===");

    // Initialize I2C (if not already done by light sensor)
    // Wire.begin() is safe to call multiple times
    Wire.begin();

    // Initialize INA219
    if (!ina219.begin()) {
      Serial.println("❌ Failed to find INA219 chip!");
      Serial.println("   Check wiring:");
      Serial.println("   - SDA → GPIO 21");
      Serial.println("   - SCL → GPIO 22");
      Serial.println("   - VCC → 3.3V");
      Serial.println("   - GND → GND");
      Serial.println("   Current monitoring DISABLED");
      return;
    }

    // Configure INA219 for 32V, 2A range (default)
    // This provides good resolution for ESP32 applications
    // Range: 0-32V bus voltage, ±3.2A current (with 0.1Ω shunt)
    ina219.setCalibration_32V_2A();

    Serial.println("✓ INA219 current monitor initialized");
    Serial.print("  I2C Address: 0x");
    Serial.println(CURRENT_MONITOR_I2C_ADDR, HEX);
    Serial.println("  Calibration: 32V, 2A range");
    Serial.print("  Check interval: ");
    Serial.print(CURRENT_CHECK_INTERVAL / 1000);
    Serial.println(" seconds");
    Serial.print("  High current warning: >");
    Serial.print(CURRENT_HIGH_THRESHOLD);
    Serial.println(" mA");
    Serial.print("  Overload warning: >");
    Serial.print(CURRENT_MAX_THRESHOLD);
    Serial.println(" mA");

    // Initialize statistics
    current.lastReset = millis();

  #endif
}

// Read current sensor values
bool readCurrentSensor() {
  #if ENABLE_CURRENT_MONITOR
    // Read values from INA219
    current.shuntVoltage_mV = ina219.getShuntVoltage_mV();
    current.voltage = ina219.getBusVoltage_V();
    current.current_mA = ina219.getCurrent_mA();
    current.power_mW = ina219.getPower_mW();

    // Handle negative current (can happen with noise or calibration)
    if (current.current_mA < 0) {
      current.current_mA = 0;
    }

    // Update statistics
    if (current.current_mA < current.currentMin) {
      current.currentMin = current.current_mA;
    }
    if (current.current_mA > current.currentMax) {
      current.currentMax = current.current_mA;
    }
    if (current.power_mW > current.powerMax) {
      current.powerMax = current.power_mW;
    }

    // Calculate average current (exponential moving average)
    // EMA = (new_value * alpha) + (old_value * (1 - alpha))
    // alpha = 0.1 provides smooth average with ~10 sample memory
    const float alpha = 0.1;
    if (current.checkCount == 0) {
      current.currentAvg = current.current_mA;  // Initialize on first read
    } else {
      current.currentAvg = (current.current_mA * alpha) + (current.currentAvg * (1.0 - alpha));
    }

    // Update energy consumption
    // Energy (mAh) = Current (mA) × Time (h)
    // Energy (Wh) = Power (W) × Time (h)
    unsigned long now = millis();
    if (current.lastCheck > 0) {
      float deltaTime_hours = (now - current.lastCheck) / 3600000.0;
      current.energyUsed_mAh += current.current_mA * deltaTime_hours;
      current.energyUsed_Wh += (current.power_mW / 1000.0) * deltaTime_hours;
    }
    current.totalTime_ms = now - current.lastReset;

    // Check thresholds
    current.isHighCurrent = (current.current_mA > CURRENT_HIGH_THRESHOLD);
    current.isOverload = (current.current_mA > CURRENT_MAX_THRESHOLD);

    return true;
  #else
    return false;
  #endif
}

// Check current sensor and print status
void checkCurrentMonitor() {
  #if ENABLE_CURRENT_MONITOR
    unsigned long now = millis();

    // Check at specified interval
    if (now - current.lastCheck < CURRENT_CHECK_INTERVAL) {
      return;
    }

    current.lastCheck = now;
    current.checkCount++;

    // Read sensor
    if (!readCurrentSensor()) {
      Serial.println("❌ Failed to read INA219 sensor");
      return;
    }

    // Print status
    Serial.print("⚡ Current #");
    Serial.print(current.checkCount);
    Serial.print(": ");
    Serial.print(current.current_mA, 1);
    Serial.print(" mA, ");
    Serial.print(current.voltage, 2);
    Serial.print(" V, ");
    Serial.print(current.power_mW, 0);
    Serial.print(" mW");

    // Warning indicators
    if (current.isOverload) {
      Serial.print(" ⚠️ OVERLOAD!");
    } else if (current.isHighCurrent) {
      Serial.print(" ⚠️ HIGH");
    } else {
      Serial.print(" ✓");
    }
    Serial.println();

    // Print statistics every 10 readings
    if (current.checkCount % 10 == 0) {
      Serial.println("  --- Current Statistics ---");
      Serial.print("  Average: ");
      Serial.print(current.currentAvg, 1);
      Serial.println(" mA");
      Serial.print("  Range: ");
      Serial.print(current.currentMin, 1);
      Serial.print(" - ");
      Serial.print(current.currentMax, 1);
      Serial.println(" mA");
      Serial.print("  Peak power: ");
      Serial.print(current.powerMax, 0);
      Serial.println(" mW");
      Serial.print("  Energy used: ");
      Serial.print(current.energyUsed_mAh, 1);
      Serial.print(" mAh (");
      Serial.print(current.energyUsed_Wh, 3);
      Serial.println(" Wh)");

      // Estimate runtime with typical battery
      // Example: 2000 mAh battery
      float batteryCapacity_mAh = 2000.0;
      if (current.currentAvg > 0) {
        float estimatedRuntime_hours = (batteryCapacity_mAh - current.energyUsed_mAh) / current.currentAvg;
        Serial.print("  Est. runtime (2000mAh): ");
        if (estimatedRuntime_hours > 0) {
          Serial.print(estimatedRuntime_hours, 1);
          Serial.println(" hours");
        } else {
          Serial.println("Battery depleted");
        }
      }
      Serial.print("  Uptime: ");
      Serial.print(current.totalTime_ms / 1000);
      Serial.println(" seconds");
    }
  #endif
}

// Get current status string for CSV output
String getCurrentStatus() {
  #if ENABLE_CURRENT_MONITOR
    return String(current.current_mA, 1);
  #else
    return "0.0";
  #endif
}

// Get voltage from INA219 (more accurate than ADC for battery monitor)
String getCurrentVoltage() {
  #if ENABLE_CURRENT_MONITOR
    return String(current.voltage, 2);
  #else
    return "0.00";
  #endif
}

// Get power consumption string
String getPowerStatus() {
  #if ENABLE_CURRENT_MONITOR
    return String(current.power_mW, 0);
  #else
    return "0";
  #endif
}

// Get energy consumed string
String getEnergyStatus() {
  #if ENABLE_CURRENT_MONITOR
    return String(current.energyUsed_mAh, 1);
  #else
    return "0.0";
  #endif
}

// Reset energy statistics (useful when starting new test)
void resetCurrentStats() {
  #if ENABLE_CURRENT_MONITOR
    current.energyUsed_mAh = 0.0;
    current.energyUsed_Wh = 0.0;
    current.currentMin = 999999.0;
    current.currentMax = 0.0;
    current.powerMax = 0.0;
    current.lastReset = millis();
    current.checkCount = 0;
    Serial.println("✓ Current statistics reset");
  #endif
}

// Check if current is critically high
bool isCurrentOverload() {
  #if ENABLE_CURRENT_MONITOR
    return current.isOverload;
  #else
    return false;
  #endif
}

// Get estimated battery runtime in hours (requires battery capacity)
float getEstimatedRuntime(float batteryCapacity_mAh) {
  #if ENABLE_CURRENT_MONITOR
    if (current.currentAvg <= 0) return 0.0;

    float remainingCapacity = batteryCapacity_mAh - current.energyUsed_mAh;
    if (remainingCapacity <= 0) return 0.0;

    return remainingCapacity / current.currentAvg;
  #else
    return 0.0;
  #endif
}

#endif // CURRENT_MONITOR_H
