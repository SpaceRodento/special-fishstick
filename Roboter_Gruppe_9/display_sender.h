/*=====================================================================
  display_sender.h - Display Station Data Sender (UART version)

  Sends real-time status updates to TFT display station via Serial.
  NO LoRa required - simple UART connection!

  Hardware connection:
  - Main ESP32 TX (GPIO 17) → Display RX (GPIO 18)
  - GND → GND

  Features:
  - Simple UART communication
  - Automatically includes enabled features
  - Configurable update interval
  - No dependencies on LoRa

  Usage:
  1. Set ENABLE_DISPLAY_OUTPUT true in config.h
  2. Call initDisplaySender() in setup()
  3. Call sendDisplayUpdate() in loop()

  Example:
    #if ENABLE_DISPLAY_OUTPUT
      initDisplaySender();
    #endif

    // In loop:
    sendDisplayUpdate();
=======================================================================*/

#ifndef DISPLAY_SENDER_H
#define DISPLAY_SENDER_H

#include <Arduino.h>
#include "config.h"
#include "DisplayClient.h"

// External declarations
extern DeviceState local;
extern DeviceState remote;
extern bool bRECEIVER;
extern HealthMonitor health;

#if ENABLE_BATTERY_MONITOR
  extern float readBatteryVoltage();
#endif

#if ENABLE_AUDIO_DETECTION
  extern bool isFireAlarmActive();
  extern struct AudioDetector {
    int currentRMS;
    int peakCount;
    bool alarmDetected;
    int alertCount;
  } audio;
#endif

#if ENABLE_LIGHT_DETECTION
  extern bool isFireLightActive();
  extern struct LightDetector {
    uint16_t red;
    int flashCount;
    bool alarmDetected;
    int alertCount;
  } light;
#endif

#if ENABLE_CURRENT_MONITOR
  extern struct CurrentStatus {
    float voltage;
    float current_mA;
    float power_mW;
    float currentAvg;
    float energyUsed_mAh;
  } current;
#endif

// Global display client
#if ENABLE_DISPLAY_OUTPUT
  DisplayClient display(DISPLAY_TX_PIN);
  unsigned long lastDisplayUpdate = 0;
#endif

/**
 * Initialize display sender
 * Call this in setup()
 */
void initDisplaySender() {
  #if ENABLE_DISPLAY_OUTPUT
    display.begin();
    delay(100);

    // Send welcome message
    display.alert("Roboter 9 online");
    delay(2000);
    display.clearAlert();

    Serial.println("\n📺 Display output enabled:");
    Serial.print("  TX pin: GPIO ");
    Serial.println(DISPLAY_TX_PIN);
    Serial.print("  Update interval: ");
    Serial.print(DISPLAY_UPDATE_INTERVAL);
    Serial.println(" ms");
    Serial.println("  Connection: TX → Display RX (GPIO 18)");
  #else
    Serial.println("\n📺 Display output: Disabled");
    Serial.println("  (Set ENABLE_DISPLAY_OUTPUT true to enable)");
  #endif
}

/**
 * Send status update to display
 * Call this regularly from loop()
 */
void sendDisplayUpdate() {
  #if ENABLE_DISPLAY_OUTPUT
    unsigned long now = millis();

    // Check if it's time to update
    if (now - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL) {
      return;
    }

    lastDisplayUpdate = now;

    // Start building message
    display.clear();

    // Role
    display.set("Mode", bRECEIVER ? "RECEIVER" : "SENDER");

    // Basic status
    display.set("SEQ", local.sequenceNumber);
    display.set("LED", local.ledState ? "ON" : "OFF");
    display.set("TOUCH", local.touchState ? "YES" : "NO");
    display.set("Count", local.messageCount);

    // Remote data (if receiver)
    if (bRECEIVER && remote.messageCount > 0) {
      display.set("R_LED", remote.ledState ? "ON" : "OFF");
      display.set("R_TOUCH", remote.touchState ? "YES" : "NO");
    }

    // Connection state (always send, even if UNKNOWN)
    display.set("ConnState", getConnectionStateString(health.state));

    // RSSI (if available)
    if (remote.rssi != 0) {
      display.set("RSSI", String(remote.rssi) + "dBm");
    }

    // SNR (if available)
    if (remote.snr != 0) {
      display.set("SNR", String(remote.snr) + "dB");
    }

    // Uptime (for display timer)
    display.set("Uptime", String(millis() / 1000) + "s");

    // LoRa packet count (messages sent/received via LoRa)
    if (bRECEIVER) {
      display.set("LoRaPkts", String(remote.messageCount));
    } else {
      display.set("LoRaPkts", String(local.messageCount));
    }

    // Battery voltage (if enabled)
    #if ENABLE_BATTERY_MONITOR
      float batVoltage = readBatteryVoltage();
      display.set("Battery", String(batVoltage, 2) + "V");
    #endif

    // Current monitoring (if enabled)
    #if ENABLE_CURRENT_MONITOR
      display.set("Current", String(current.current_mA, 0) + "mA");
      display.set("Power", String(current.power_mW, 0) + "mW");
      display.set("Energy", String(current.energyUsed_mAh, 1) + "mAh");
      // Use INA219 voltage if current monitor is enabled (more accurate)
      #if !ENABLE_BATTERY_MONITOR
        display.set("Voltage", String(current.voltage, 2) + "V");
      #endif
    #endif

    // Extended telemetry (if enabled)
    #if ENABLE_EXTENDED_TELEMETRY
      display.set("Uptime", String(millis() / 1000) + "s");
      display.set("Heap", String(ESP.getFreeHeap() / 1024) + "KB");

      // Internal temperature
      extern "C" uint8_t temprature_sens_read();
      uint8_t raw = temprature_sens_read();
      float tempC = (raw - 32) / 1.8;
      display.set("Temp", String((int)tempC) + "C");
    #endif

    // Send all data
    display.send();

    // Check for fire alerts
    #if ENABLE_AUDIO_DETECTION
      if (audio.alarmDetected) {
        display.alert("FIRE: Audio!");
      }
    #endif

    #if ENABLE_LIGHT_DETECTION
      if (light.alarmDetected) {
        display.alert("FIRE: Light!");
      }
    #endif

    // Clear alert if no alarms
    #if ENABLE_AUDIO_DETECTION || ENABLE_LIGHT_DETECTION
      #if ENABLE_AUDIO_DETECTION
        bool audioAlarm = audio.alarmDetected;
      #else
        bool audioAlarm = false;
      #endif

      #if ENABLE_LIGHT_DETECTION
        bool lightAlarm = light.alarmDetected;
      #else
        bool lightAlarm = false;
      #endif

      if (!audioAlarm && !lightAlarm) {
        display.clearAlert();
      }
    #endif
  #endif
}

/**
 * Send immediate alert to display
 * Bypasses update interval
 */
void sendDisplayAlert(String message) {
  #if ENABLE_DISPLAY_OUTPUT
    display.alert(message);
  #endif
}

/**
 * Clear alert from display
 */
void clearDisplayAlert() {
  #if ENABLE_DISPLAY_OUTPUT
    display.clearAlert();
  #endif
}

#endif // DISPLAY_SENDER_H
