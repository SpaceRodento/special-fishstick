/*=====================================================================
  display_sender.h - Display Station Data Sender

  Sends real-time status updates to TFT display station
  (ESP32-2432S022 @ LoRa address 3)

  Features:
  - Sends comprehensive status data
  - Includes battery, RSSI, telemetry
  - Supports alert notifications
  - Configurable update interval

  Usage:
  1. Set ENABLE_DISPLAY_OUTPUT true in config.h
  2. Call sendDisplayUpdate() in loop()
  3. Display station will receive and parse data automatically

  Message Format:
  CSV format: "SEQ:123,LED:1,TOUCH:0,BAT:3.85,UP:3600,..."
=======================================================================*/

#ifndef DISPLAY_SENDER_H
#define DISPLAY_SENDER_H

#include <Arduino.h>
#include "config.h"

// External declarations
extern bool sendLoRaMessage(String message, uint8_t targetAddress);
extern DeviceState local;

#if ENABLE_BATTERY_MONITOR
  extern float readBatteryVoltage();
#endif

#if ENABLE_EXTENDED_TELEMETRY
  extern String getTelemetryString();
#endif

#if ENABLE_AUDIO_DETECTION
  extern bool isFireAlarmActive();
  extern int audio_currentRMS;
  extern int audio_peakCount;
#endif

#if ENABLE_LIGHT_DETECTION
  extern bool isFireLightActive();
  extern uint16_t light_red;
  extern int light_flashCount;
#endif

// Display update tracking
unsigned long lastDisplayUpdate = 0;

/**
 * Send status update to display station
 * Call this regularly from loop() (every DISPLAY_UPDATE_INTERVAL ms)
 */
void sendDisplayUpdate() {
  #if ENABLE_DISPLAY_OUTPUT
    unsigned long now = millis();

    // Check if it's time to update
    if (now - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL) {
      return;
    }

    lastDisplayUpdate = now;

    // Build payload with basic status
    String payload = "SEQ:" + String(local.sequenceNumber) +
                     ",LED:" + String(local.ledState ? 1 : 0) +
                     ",TOUCH:" + String(local.touchState ? 1 : 0);

    // Add battery voltage if enabled
    #if ENABLE_BATTERY_MONITOR
      float batVoltage = readBatteryVoltage();
      payload += ",BAT:" + String(batVoltage, 2);
    #endif

    // Add extended telemetry if enabled
    #if ENABLE_EXTENDED_TELEMETRY
      payload += getTelemetryString();
    #endif

    // Add fire alerts if detected
    #if ENABLE_AUDIO_DETECTION
      if (isFireAlarmActive()) {
        payload += ",ALERT:FIRE_AUDIO";
        payload += ",RMS:" + String(audio_currentRMS);
        payload += ",PEAKS:" + String(audio_peakCount);
      }
    #endif

    #if ENABLE_LIGHT_DETECTION
      if (isFireLightActive()) {
        payload += ",ALERT:FIRE_LIGHT";
        payload += ",RED:" + String(light_red);
        payload += ",FLASHES:" + String(light_flashCount);
      }
    #endif

    // Send to display station (address 3)
    if (sendLoRaMessage(payload, LORA_DISPLAY_ADDRESS)) {
      Serial.print("📺 Display update sent (");
      Serial.print(payload.length());
      Serial.println(" bytes)");
    } else {
      Serial.println("❌ Display update failed");
    }
  #endif
}

/**
 * Send alert to display station
 * Use for immediate notifications (bypasses update interval)
 */
void sendDisplayAlert(String alertType, String alertData) {
  #if ENABLE_DISPLAY_OUTPUT
    String payload = "ALERT:" + alertType;
    if (alertData.length() > 0) {
      payload += "," + alertData;
    }

    if (sendLoRaMessage(payload, LORA_DISPLAY_ADDRESS)) {
      Serial.print("🚨 Alert sent to display: ");
      Serial.println(alertType);
    }
  #endif
}

/**
 * Print display configuration at startup
 */
void printDisplayConfig() {
  #if ENABLE_DISPLAY_OUTPUT
    Serial.println("\n📺 Display Station Enabled:");
    Serial.print("  Target address: ");
    Serial.println(LORA_DISPLAY_ADDRESS);
    Serial.print("  Update interval: ");
    Serial.print(DISPLAY_UPDATE_INTERVAL);
    Serial.println(" ms");

    Serial.print("  Data includes: SEQ, LED, TOUCH");

    #if ENABLE_BATTERY_MONITOR
      Serial.print(", BAT");
    #endif

    #if ENABLE_EXTENDED_TELEMETRY
      Serial.print(", UP, HEAP, TEMP, LOOP");
    #endif

    #if ENABLE_AUDIO_DETECTION
      Serial.print(", FIRE_AUDIO");
    #endif

    #if ENABLE_LIGHT_DETECTION
      Serial.print(", FIRE_LIGHT");
    #endif

    Serial.println();
  #else
    Serial.println("\n📺 Display Station: Disabled");
    Serial.println("  (Set ENABLE_DISPLAY_OUTPUT true to enable)");
  #endif
}

#endif // DISPLAY_SENDER_H
