/*=====================================================================
  extended_telemetry.h - Extended Telemetry Data

  FEATURE 8: Extended Telemetry

  Adds extra diagnostic data to LoRa payload:
  - System uptime (seconds)
  - Free heap memory (KB)
  - Internal temperature (°C)
  - WiFi RSSI (if WiFi enabled)
  - Loop frequency (Hz)
  - Minimum free heap (memory leak detection)

  Benefits:
  - Remote system health monitoring
  - Early warning of issues (low memory, high temp)
  - Performance tracking over distance
  - Debugging deployment problems

  Trade-offs:
  - Larger payload size (~30-50 extra bytes)
  - Slightly longer air time
  - May reduce max range at SF12
  - More data to parse

  Payload Format (CSV):
  Original: SEQ:123,LED:1,TOUCH:0
  Extended: SEQ:123,LED:1,TOUCH:0,UP:3600,HEAP:245,TEMP:42,LOOP:450

  Use Cases:
  - Production monitoring
  - Long-term deployments
  - Temperature-sensitive environments
  - Memory leak detection
  - Performance profiling

  Configuration:
  - ENABLE_EXTENDED_TELEMETRY true/false
  - Data is added automatically to payload
  - CSV parser on receiver must handle extra fields

  Testing:
  1. Set ENABLE_EXTENDED_TELEMETRY true in config.h
  2. Upload to sender
  3. Check serial output: Should see extra fields
  4. Python scripts: Update parser to handle new fields

  Performance Impact:
  - Payload size: +35 bytes typical
  - CPU overhead: <1ms
  - Air time increase: ~10-15% at SF12
  - Range reduction: Negligible if payload <100 bytes

  Temperature Reading:
  - ESP32 has internal temperature sensor
  - Accuracy: ±5°C (not calibrated)
  - Useful for trend monitoring, not absolute values
  - Range: -40°C to 85°C
=======================================================================*/

#ifndef EXTENDED_TELEMETRY_H
#define EXTENDED_TELEMETRY_H

#include <Arduino.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ESP32 internal temperature sensor (if available)
uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

// Extended telemetry data structure
struct ExtendedTelemetry {
  unsigned long uptime;          // System uptime (seconds)
  int freeHeapKB;                // Free heap memory (KB)
  int minFreeHeapKB;             // Minimum free heap (KB)
  float temperature;             // Internal temperature (°C)
  int loopFrequency;             // Loop frequency (Hz)
  int wifiRSSI;                  // WiFi RSSI (if connected)
  unsigned long lastUpdate;      // Last telemetry update
  int updateCount;               // Number of updates
};

ExtendedTelemetry telemetry = {0, 0, 0, 0.0, 0, 0, 0, 0};

// Initialize extended telemetry
void initExtendedTelemetry() {
  #if ENABLE_EXTENDED_TELEMETRY
    telemetry.lastUpdate = millis();
    telemetry.freeHeapKB = ESP.getFreeHeap() / 1024;
    telemetry.minFreeHeapKB = ESP.getMinFreeHeap() / 1024;

    Serial.println("📊 Extended telemetry enabled");
    Serial.println("  Monitoring:");
    Serial.println("    - System uptime");
    Serial.println("    - Free heap memory");
    Serial.println("    - Internal temperature");
    Serial.println("    - Loop frequency");
    Serial.println("  ⚠️  Payload size increased by ~35 bytes");
  #endif
}

// Read internal temperature sensor
float readInternalTemperature() {
  #if ENABLE_EXTENDED_TELEMETRY
    // ESP32 has internal temperature sensor
    // Note: Accuracy is ±5°C, use for trends only
    uint8_t raw = temprature_sens_read();

    // Convert to Celsius (approximate formula)
    // Formula varies by ESP32 revision
    float tempC = (raw - 32) / 1.8;

    // Clamp to reasonable range
    if (tempC < -40.0) tempC = -40.0;
    if (tempC > 125.0) tempC = 125.0;

    return tempC;
  #else
    return 0.0;
  #endif
}

// Update telemetry data
void updateTelemetry() {
  #if ENABLE_EXTENDED_TELEMETRY
    unsigned long now = millis();

    // Uptime in seconds
    telemetry.uptime = now / 1000;

    // Memory stats
    telemetry.freeHeapKB = ESP.getFreeHeap() / 1024;
    telemetry.minFreeHeapKB = ESP.getMinFreeHeap() / 1024;

    // Temperature
    telemetry.temperature = readInternalTemperature();

    // Loop frequency (if performance monitor enabled)
    #if ENABLE_PERFORMANCE_MONITOR
      extern PerformanceMetrics perf;
      telemetry.loopFrequency = perf.loopFrequency;
    #else
      telemetry.loopFrequency = 0;
    #endif

    // WiFi RSSI (if WiFi enabled)
    #if ENABLE_WIFI_AP
      if (WiFi.status() == WL_CONNECTED) {
        telemetry.wifiRSSI = WiFi.RSSI();
      } else {
        telemetry.wifiRSSI = 0;
      }
    #else
      telemetry.wifiRSSI = 0;
    #endif

    telemetry.lastUpdate = now;
    telemetry.updateCount++;
  #endif
}

// Build extended telemetry string for payload
String getTelemetryString() {
  #if ENABLE_EXTENDED_TELEMETRY
    updateTelemetry();

    String telem = "";

    // Uptime
    telem += ",UP:" + String(telemetry.uptime);

    // Free heap
    telem += ",HEAP:" + String(telemetry.freeHeapKB);

    // Min heap (memory leak indicator)
    telem += ",MHEAP:" + String(telemetry.minFreeHeapKB);

    // Temperature
    telem += ",TEMP:" + String(telemetry.temperature, 1);

    // Loop frequency (if available)
    if (telemetry.loopFrequency > 0) {
      telem += ",LOOP:" + String(telemetry.loopFrequency);
    }

    // WiFi RSSI (if available)
    if (telemetry.wifiRSSI != 0) {
      telem += ",WIFI:" + String(telemetry.wifiRSSI);
    }

    return telem;
  #else
    return "";
  #endif
}

// Print telemetry to serial
void printTelemetry() {
  #if ENABLE_EXTENDED_TELEMETRY
    updateTelemetry();

    Serial.println("\n╔════════ EXTENDED TELEMETRY ════════╗");
    Serial.print("║ Uptime:          ");
    if (telemetry.uptime < 60) {
      Serial.print(telemetry.uptime);
      Serial.println(" seconds");
    } else if (telemetry.uptime < 3600) {
      Serial.print(telemetry.uptime / 60);
      Serial.print(" min ");
      Serial.print(telemetry.uptime % 60);
      Serial.println(" sec");
    } else {
      Serial.print(telemetry.uptime / 3600);
      Serial.print(" hours ");
      Serial.print((telemetry.uptime % 3600) / 60);
      Serial.println(" min");
    }

    Serial.print("║ Free heap:       ");
    Serial.print(telemetry.freeHeapKB);
    Serial.print(" KB");
    if (telemetry.freeHeapKB < 50) {
      Serial.print(" ⚠️  LOW!");
    }
    Serial.println();

    Serial.print("║ Min heap:        ");
    Serial.print(telemetry.minFreeHeapKB);
    Serial.println(" KB");

    // Memory leak check
    static int lastMinHeap = telemetry.minFreeHeapKB;
    if (telemetry.minFreeHeapKB < lastMinHeap - 5) {
      Serial.println("║ ⚠️  Memory leak detected!");
      lastMinHeap = telemetry.minFreeHeapKB;
    }

    Serial.print("║ Temperature:     ");
    Serial.print(telemetry.temperature, 1);
    Serial.print(" °C");
    if (telemetry.temperature > 80.0) {
      Serial.print(" ⚠️  HIGH!");
    }
    Serial.println();

    if (telemetry.loopFrequency > 0) {
      Serial.print("║ Loop freq:       ");
      Serial.print(telemetry.loopFrequency);
      Serial.print(" Hz");
      if (telemetry.loopFrequency < 10) {
        Serial.print(" ⚠️  SLOW!");
      }
      Serial.println();
    }

    if (telemetry.wifiRSSI != 0) {
      Serial.print("║ WiFi RSSI:       ");
      Serial.print(telemetry.wifiRSSI);
      Serial.println(" dBm");
    }

    Serial.print("║ Updates:         ");
    Serial.println(telemetry.updateCount);

    Serial.println("╚════════════════════════════════════╝\n");
  #endif
}

// Parse extended telemetry from received payload
void parseTelemetry(String payload, ExtendedTelemetry* remoteTelem) {
  #if ENABLE_EXTENDED_TELEMETRY
    // Parse UP:X
    int upIdx = payload.indexOf("UP:");
    if (upIdx >= 0) {
      int comma = payload.indexOf(',', upIdx);
      if (comma < 0) comma = payload.length();
      remoteTelem->uptime = payload.substring(upIdx + 3, comma).toInt();
    }

    // Parse HEAP:X
    int heapIdx = payload.indexOf("HEAP:");
    if (heapIdx >= 0) {
      int comma = payload.indexOf(',', heapIdx);
      if (comma < 0) comma = payload.length();
      remoteTelem->freeHeapKB = payload.substring(heapIdx + 5, comma).toInt();
    }

    // Parse MHEAP:X
    int mheapIdx = payload.indexOf("MHEAP:");
    if (mheapIdx >= 0) {
      int comma = payload.indexOf(',', mheapIdx);
      if (comma < 0) comma = payload.length();
      remoteTelem->minFreeHeapKB = payload.substring(mheapIdx + 6, comma).toInt();
    }

    // Parse TEMP:X
    int tempIdx = payload.indexOf("TEMP:");
    if (tempIdx >= 0) {
      int comma = payload.indexOf(',', tempIdx);
      if (comma < 0) comma = payload.length();
      remoteTelem->temperature = payload.substring(tempIdx + 5, comma).toFloat();
    }

    // Parse LOOP:X
    int loopIdx = payload.indexOf("LOOP:");
    if (loopIdx >= 0) {
      int comma = payload.indexOf(',', loopIdx);
      if (comma < 0) comma = payload.length();
      remoteTelem->loopFrequency = payload.substring(loopIdx + 5, comma).toInt();
    }

    // Parse WIFI:X
    int wifiIdx = payload.indexOf("WIFI:");
    if (wifiIdx >= 0) {
      int comma = payload.indexOf(',', wifiIdx);
      if (comma < 0) comma = payload.length();
      remoteTelem->wifiRSSI = payload.substring(wifiIdx + 5, comma).toInt();
    }
  #endif
}

// Get telemetry value by key (for generic parsing)
String getTelemetryValue(String key) {
  #if ENABLE_EXTENDED_TELEMETRY
    updateTelemetry();

    if (key == "UP" || key == "UPTIME") {
      return String(telemetry.uptime);
    }
    else if (key == "HEAP" || key == "FREE_HEAP") {
      return String(telemetry.freeHeapKB);
    }
    else if (key == "MHEAP" || key == "MIN_HEAP") {
      return String(telemetry.minFreeHeapKB);
    }
    else if (key == "TEMP" || key == "TEMPERATURE") {
      return String(telemetry.temperature, 1);
    }
    else if (key == "LOOP" || key == "LOOP_FREQ") {
      return String(telemetry.loopFrequency);
    }
    else if (key == "WIFI" || key == "WIFI_RSSI") {
      return String(telemetry.wifiRSSI);
    }
    else {
      return "";
    }
  #else
    return "";
  #endif
}

// Check if system health is good
bool isSystemHealthy() {
  #if ENABLE_EXTENDED_TELEMETRY
    updateTelemetry();

    // Check memory
    if (telemetry.freeHeapKB < 50) {
      return false;  // Low memory
    }

    // Check temperature
    if (telemetry.temperature > 85.0) {
      return false;  // Too hot
    }

    // Check loop frequency
    if (telemetry.loopFrequency > 0 && telemetry.loopFrequency < 10) {
      return false;  // Loop too slow
    }

    return true;  // All checks passed
  #else
    return true;  // Feature disabled, assume healthy
  #endif
}

// Get health status string
String getHealthStatus() {
  #if ENABLE_EXTENDED_TELEMETRY
    if (isSystemHealthy()) {
      return "HEALTHY";
    } else {
      String issues = "ISSUES:";
      if (telemetry.freeHeapKB < 50) issues += " LOW_MEM";
      if (telemetry.temperature > 85.0) issues += " HIGH_TEMP";
      if (telemetry.loopFrequency < 10 && telemetry.loopFrequency > 0) issues += " SLOW_LOOP";
      return issues;
    }
  #else
    return "DISABLED";
  #endif
}

#endif // EXTENDED_TELEMETRY_H
