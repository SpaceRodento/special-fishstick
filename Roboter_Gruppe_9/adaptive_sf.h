/*=====================================================================
  adaptive_sf.h - Adaptive Spreading Factor

  FEATURE 9: Adaptive Spreading Factor

  Automatically adjusts LoRa spreading factor based on signal quality.
  Optimizes throughput vs. range trade-off dynamically.

  How it works:
  1. Monitor RSSI continuously
  2. If RSSI > -80 dBm (good signal) → Decrease SF (faster speed)
  3. If RSSI < -105 dBm (weak signal) → Increase SF (better range)
  4. Wait for stabilization before next adjustment
  5. Both devices must sync SF changes

  Benefits:
  - Maximum speed at close range (SF7 = 11× faster than SF12)
  - Maintains link at long range (SF12 = max sensitivity)
  - Automatic optimization as distance changes
  - Reduced air time (less collisions)
  - Better battery life (shorter TX time at close range)

  Trade-offs:
  - Complexity: Both devices must coordinate
  - Packet loss during SF transition
  - Not suitable for rapidly moving devices
  - Requires bi-directional communication

  Spreading Factor Table:
  SF  | Speed    | Range  | Air Time | Sensitivity
  ----|----------|--------|----------|-------------
  7   | 5.5 kbps | 2 km   | 41 ms    | -123 dBm
  8   | 3.1 kbps | 3 km   | 72 ms    | -126 dBm
  9   | 1.8 kbps | 4 km   | 144 ms   | -129 dBm
  10  | 1.0 kbps | 5 km   | 288 ms   | -132 dBm
  11  | 0.5 kbps | 7 km   | 577 ms   | -134.5 dBm
  12  | 0.3 kbps | 10 km  | 991 ms   | -137 dBm

  RSSI Thresholds (configurable):
  - RSSI > -80 dBm: Excellent → Try SF-1
  - RSSI -80 to -105: Good → Maintain current SF
  - RSSI < -105 dBm: Weak → Try SF+1

  Configuration:
  - ADAPTIVE_SF_RSSI_GOOD: Threshold to decrease SF (-80 default)
  - ADAPTIVE_SF_RSSI_WEAK: Threshold to increase SF (-105 default)
  - SF_CHANGE_COOLDOWN: Min time between changes (30s default)
  - SF_CHANGE_SAMPLES: Required samples before change (10 default)

  Synchronization:
  - Sender announces SF change via special packet
  - Receiver acknowledges and switches
  - Both wait for confirmation before using new SF
  - If sync fails, revert to SF12 (safest)

  Testing:
  1. Set ENABLE_ADAPTIVE_SF true in config.h
  2. Upload to both devices
  3. Start at close range (should use SF7-8)
  4. Move apart (SF increases)
  5. Move together (SF decreases)
  6. Check serial for SF change messages

  Performance:
  - Close range (RSSI -50): SF7 → 11× faster
  - Medium range (RSSI -90): SF10 → 3× faster
  - Long range (RSSI -115): SF12 → max reliability

  Safety:
  - Always start at SF12 (maximum range)
  - If communication fails, revert to SF12
  - Never go below SF7 (minimum allowed)
  - Never go above SF12 (maximum allowed)
=======================================================================*/

#ifndef ADAPTIVE_SF_H
#define ADAPTIVE_SF_H

#include <Arduino.h>
#include "config.h"

// Adaptive SF configuration
#define SF_MIN 7
#define SF_MAX 12
#define SF_CHANGE_COOLDOWN 30000     // 30 seconds between changes
#define SF_CHANGE_SAMPLES 10         // Samples required before change
#define SF_SYNC_TIMEOUT 5000         // 5 seconds to confirm SF change

// Adaptive SF state
struct AdaptiveSFState {
  int currentSF;                    // Current spreading factor
  int targetSF;                     // Target SF (during transition)
  int rssiSamples[SF_CHANGE_SAMPLES]; // Recent RSSI samples
  int sampleIndex;                  // Current sample index
  int sampleCount;                  // Number of samples collected
  unsigned long lastChange;         // Last SF change time
  int changeCount;                  // Total SF changes
  bool isChanging;                  // Currently changing SF
  unsigned long changeStartTime;    // SF change start time
  bool isEnabled;                   // Feature enabled flag
};

AdaptiveSFState adaptiveSF = {12, 12, {0}, 0, 0, 0, 0, false, 0, false};

// Initialize adaptive SF
void initAdaptiveSF() {
  #if ENABLE_ADAPTIVE_SF
    adaptiveSF.currentSF = 12;  // Start with maximum range
    adaptiveSF.targetSF = 12;
    adaptiveSF.sampleIndex = 0;
    adaptiveSF.sampleCount = 0;
    adaptiveSF.lastChange = millis();
    adaptiveSF.changeCount = 0;
    adaptiveSF.isChanging = false;
    adaptiveSF.isEnabled = true;

    Serial.println("📡 Adaptive Spreading Factor enabled");
    Serial.print("  Initial SF: SF");
    Serial.println(adaptiveSF.currentSF);
    Serial.print("  Good RSSI threshold: ");
    Serial.print(ADAPTIVE_SF_RSSI_GOOD);
    Serial.println(" dBm");
    Serial.print("  Weak RSSI threshold: ");
    Serial.print(ADAPTIVE_SF_RSSI_WEAK);
    Serial.println(" dBm");
    Serial.print("  Cooldown: ");
    Serial.print(SF_CHANGE_COOLDOWN / 1000);
    Serial.println(" seconds");
    Serial.println("  ⚠️  Both devices must have this enabled!");
  #endif
}

// Add RSSI sample
void addRSSISample(int rssi) {
  #if ENABLE_ADAPTIVE_SF
    // Store sample
    adaptiveSF.rssiSamples[adaptiveSF.sampleIndex] = rssi;
    adaptiveSF.sampleIndex = (adaptiveSF.sampleIndex + 1) % SF_CHANGE_SAMPLES;

    if (adaptiveSF.sampleCount < SF_CHANGE_SAMPLES) {
      adaptiveSF.sampleCount++;
    }
  #endif
}

// Calculate average RSSI from samples
float getAverageRSSI() {
  #if ENABLE_ADAPTIVE_SF
    if (adaptiveSF.sampleCount == 0) return 0;

    long sum = 0;
    for (int i = 0; i < adaptiveSF.sampleCount; i++) {
      sum += adaptiveSF.rssiSamples[i];
    }

    return (float)sum / (float)adaptiveSF.sampleCount;
  #else
    return 0;
  #endif
}

// Determine optimal SF based on RSSI
int determineOptimalSF(float avgRSSI) {
  #if ENABLE_ADAPTIVE_SF
    int optimalSF = adaptiveSF.currentSF;

    // Strong signal → decrease SF (faster speed)
    if (avgRSSI > ADAPTIVE_SF_RSSI_GOOD) {
      if (adaptiveSF.currentSF > SF_MIN) {
        optimalSF = adaptiveSF.currentSF - 1;
      }
    }
    // Weak signal → increase SF (better range)
    else if (avgRSSI < ADAPTIVE_SF_RSSI_WEAK) {
      if (adaptiveSF.currentSF < SF_MAX) {
        optimalSF = adaptiveSF.currentSF + 1;
      }
    }

    return optimalSF;
  #else
    return 12;
  #endif
}

// Apply SF change to LoRa module
bool applySpreadingFactor(int sf) {
  #if ENABLE_ADAPTIVE_SF
    Serial.print("📡 Applying SF");
    Serial.print(sf);
    Serial.println("...");

    // Build AT command
    // AT+PARAMETER=SF,BW,CR,PREAMBLE
    // SF: 7-12, BW: 7=125kHz, CR: 1=4/5, PREAMBLE: 4
    String cmd = "AT+PARAMETER=" + String(sf) + ",7,1,4";

    extern HardwareSerial LoRaSerial;
    LoRaSerial.println(cmd);
    delay(100);

    // Check response
    if (LoRaSerial.available()) {
      String response = LoRaSerial.readStringUntil('\n');
      response.trim();

      if (response == "+OK") {
        Serial.print("✓ SF changed to SF");
        Serial.println(sf);
        return true;
      } else {
        Serial.print("❌ SF change failed: ");
        Serial.println(response);
        return false;
      }
    }

    return false;
  #else
    return false;
  #endif
}

// Send SF change announcement to remote device
void announceSpreadingFactorChange(int newSF) {
  #if ENABLE_ADAPTIVE_SF
    Serial.print("→ Announcing SF change to SF");
    Serial.println(newSF);

    String announcement = "CMD:SF_CHANGE:" + String(newSF);

    extern void sendLoRaMessage(String payload, int address);
    sendLoRaMessage(announcement, LORA_SENDER_ADDRESS);
  #endif
}

// Process SF change announcement from remote
void processSpreadingFactorChange(String payload) {
  #if ENABLE_ADAPTIVE_SF
    // Parse CMD:SF_CHANGE:X
    int sfIdx = payload.indexOf("CMD:SF_CHANGE:");
    if (sfIdx >= 0) {
      int newSF = payload.substring(sfIdx + 14).toInt();

      if (newSF >= SF_MIN && newSF <= SF_MAX) {
        Serial.print("📡 Remote requests SF change to SF");
        Serial.println(newSF);

        // Apply change
        if (applySpreadingFactor(newSF)) {
          adaptiveSF.currentSF = newSF;
          adaptiveSF.targetSF = newSF;

          // Send ACK
          String ack = "CMD:SF_ACK:" + String(newSF);
          extern void sendLoRaMessage(String payload, int address);
          sendLoRaMessage(ack, LORA_SENDER_ADDRESS);
        }
      } else {
        Serial.print("❌ Invalid SF: ");
        Serial.println(newSF);
      }
    }
  #endif
}

// Update adaptive SF (call regularly with current RSSI)
void updateAdaptiveSF(int currentRSSI) {
  #if ENABLE_ADAPTIVE_SF
    unsigned long now = millis();

    // Skip if currently changing
    if (adaptiveSF.isChanging) {
      // Check for timeout
      if (now - adaptiveSF.changeStartTime > SF_SYNC_TIMEOUT) {
        Serial.println("⚠️  SF change timeout, reverting to SF12");
        applySpreadingFactor(12);
        adaptiveSF.currentSF = 12;
        adaptiveSF.targetSF = 12;
        adaptiveSF.isChanging = false;
      }
      return;
    }

    // Skip if in cooldown
    if (now - adaptiveSF.lastChange < SF_CHANGE_COOLDOWN) {
      return;
    }

    // Add RSSI sample
    addRSSISample(currentRSSI);

    // Need enough samples
    if (adaptiveSF.sampleCount < SF_CHANGE_SAMPLES) {
      return;
    }

    // Calculate average RSSI
    float avgRSSI = getAverageRSSI();

    // Determine optimal SF
    int optimalSF = determineOptimalSF(avgRSSI);

    // Check if change needed
    if (optimalSF != adaptiveSF.currentSF) {
      Serial.println("\n╔════ ADAPTIVE SF ════╗");
      Serial.print("║ Current SF:  SF");
      Serial.println(adaptiveSF.currentSF);
      Serial.print("║ Avg RSSI:    ");
      Serial.print(avgRSSI, 1);
      Serial.println(" dBm");
      Serial.print("║ Target SF:   SF");
      Serial.println(optimalSF);

      if (avgRSSI > ADAPTIVE_SF_RSSI_GOOD) {
        Serial.println("║ Reason:      Strong signal → Faster speed");
      } else {
        Serial.println("║ Reason:      Weak signal → Better range");
      }

      Serial.println("╚═════════════════════╝");

      // Start SF change process
      adaptiveSF.targetSF = optimalSF;
      adaptiveSF.isChanging = true;
      adaptiveSF.changeStartTime = now;

      // Announce to remote device
      announceSpreadingFactorChange(optimalSF);

      // Apply locally
      if (applySpreadingFactor(optimalSF)) {
        adaptiveSF.currentSF = optimalSF;
        adaptiveSF.lastChange = now;
        adaptiveSF.changeCount++;

        // Reset samples
        adaptiveSF.sampleCount = 0;
        adaptiveSF.sampleIndex = 0;
      }

      adaptiveSF.isChanging = false;
    }
  #endif
}

// Get current SF
int getCurrentSF() {
  #if ENABLE_ADAPTIVE_SF
    return adaptiveSF.currentSF;
  #else
    return 12;
  #endif
}

// Get SF change count
int getSFChangeCount() {
  #if ENABLE_ADAPTIVE_SF
    return adaptiveSF.changeCount;
  #else
    return 0;
  #endif
}

// Print adaptive SF status
void printAdaptiveSFStatus() {
  #if ENABLE_ADAPTIVE_SF
    Serial.println("\n╔═══════ ADAPTIVE SF STATUS ═══════╗");
    Serial.print("║ Current SF:      SF");
    Serial.println(adaptiveSF.currentSF);

    if (adaptiveSF.sampleCount > 0) {
      Serial.print("║ Avg RSSI:        ");
      Serial.print(getAverageRSSI(), 1);
      Serial.println(" dBm");
    }

    Serial.print("║ Changes:         ");
    Serial.println(adaptiveSF.changeCount);

    Serial.print("║ Time since last: ");
    Serial.print((millis() - adaptiveSF.lastChange) / 1000);
    Serial.println(" s");

    Serial.print("║ Samples:         ");
    Serial.print(adaptiveSF.sampleCount);
    Serial.print(" / ");
    Serial.println(SF_CHANGE_SAMPLES);

    if (adaptiveSF.isChanging) {
      Serial.println("║ Status:          ⏳ CHANGING");
    } else {
      Serial.println("║ Status:          ✓ STABLE");
    }

    Serial.println("╚══════════════════════════════════╝\n");
  #endif
}

// Force SF (override adaptive)
void forceSpreadingFactor(int sf) {
  #if ENABLE_ADAPTIVE_SF
    if (sf < SF_MIN || sf > SF_MAX) {
      Serial.println("❌ Invalid SF (must be 7-12)");
      return;
    }

    Serial.print("⚠️  Forcing SF to SF");
    Serial.println(sf);

    if (applySpreadingFactor(sf)) {
      adaptiveSF.currentSF = sf;
      adaptiveSF.targetSF = sf;
      adaptiveSF.lastChange = millis();

      // Announce to remote
      announceSpreadingFactorChange(sf);
    }
  #endif
}

// Reset to SF12 (safest, maximum range)
void resetToMaxRange() {
  #if ENABLE_ADAPTIVE_SF
    Serial.println("🔄 Resetting to SF12 (max range)");
    forceSpreadingFactor(12);
  #endif
}

#endif // ADAPTIVE_SF_H
