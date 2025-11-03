/*=====================================================================
  health_monitor.h - Connection Watchdog & Health Monitoring

  Features:
  - Connection state machine (UNKNOWN -> CONNECTED -> WEAK -> LOST)
  - RSSI statistics tracking (min, max, average)
  - Packet loss detection with sequence numbers
  - Automatic recovery attempts
  - Health status reporting

  Usage:
  1. Call initHealthMonitor() in setup()
  2. Call updateConnectionState() in loop() (receiver)
  3. Call trackPacket() when packet received
  4. Call getConnectionStateString() for display

=======================================================================*/

#ifndef HEALTH_MONITOR_H
#define HEALTH_MONITOR_H

#include "config.h"
#include "structs.h"

// =============== GLOBAL WATCHDOG CONFIG ================================
// Default thresholds - can be adjusted
WatchdogConfig watchdogCfg = {
  .weakTimeout = 3000,           // 3 seconds -> WEAK
  .lostTimeout = 8000,           // 8 seconds -> LOST
  .weakRssiThreshold = -100,     // dBm
  .criticalRssiThreshold = -110, // dBm
  .recoveryInterval = 15000,     // Try recovery every 15s
  .maxRecoveryAttempts = 3       // Give up after 3 attempts
};

// =============== INITIALIZE HEALTH MONITOR ================================
inline void initHealthMonitor(HealthMonitor& health) {
  health.state = CONN_UNKNOWN;
  health.stateChangeTime = millis();
  health.connectedSince = 0;

  health.rssiMin = 0;
  health.rssiMax = -120;
  health.rssiSum = 0;
  health.rssiSamples = 0;

  health.expectedSeq = 0;
  health.packetsReceived = 0;
  health.packetsLost = 0;
  health.packetsDuplicate = 0;

  health.recoveryAttempts = 0;
  health.lastRecoveryAttempt = 0;

  health.startTime = millis();

  Serial.println("✓ Health Monitor initialized");
}

// =============== UPDATE RSSI STATISTICS ================================
inline void updateRSSI(HealthMonitor& health, int rssi) {
  // Update min/max
  if (rssi < health.rssiMin || health.rssiSamples == 0) {
    health.rssiMin = rssi;
  }
  if (rssi > health.rssiMax || health.rssiSamples == 0) {
    health.rssiMax = rssi;
  }

  // Update sum for average (use sliding window to prevent overflow)
  health.rssiSum += rssi;
  health.rssiSamples++;

  // Reset statistics every 100 samples to prevent overflow
  if (health.rssiSamples >= 100) {
    health.rssiSum = health.rssiSum / health.rssiSamples;  // Keep average
    health.rssiSamples = 1;
  }
}

// =============== GET RSSI AVERAGE ================================
inline int getRSSIAverage(HealthMonitor& health) {
  if (health.rssiSamples == 0) return 0;
  return health.rssiSum / health.rssiSamples;
}

// =============== TRACK PACKET (Sequence number & loss detection) ================================
inline void trackPacket(HealthMonitor& health, int receivedSeq) {
  // First packet - initialize
  if (health.packetsReceived == 0) {
    health.expectedSeq = receivedSeq + 1;
    health.packetsReceived = 1;
    return;
  }

  // Check for lost packets
  if (receivedSeq > health.expectedSeq) {
    int lost = receivedSeq - health.expectedSeq;
    health.packetsLost += lost;
    Serial.print("⚠ Lost packets detected: ");
    Serial.println(lost);
  }
  // Check for duplicate
  else if (receivedSeq < health.expectedSeq) {
    health.packetsDuplicate++;
    Serial.println("⚠ Duplicate packet");
    return;  // Don't update expected
  }

  health.packetsReceived++;
  health.expectedSeq = receivedSeq + 1;
}

// =============== GET PACKET LOSS PERCENTAGE ================================
inline float getPacketLoss(HealthMonitor& health) {
  int totalExpected = health.packetsReceived + health.packetsLost;
  if (totalExpected == 0) return 0.0;
  return (float)health.packetsLost / totalExpected * 100.0;
}

// =============== GET CONNECTION STATE STRING ================================
inline const char* getConnectionStateString(ConnectionState state) {
  switch (state) {
    case CONN_UNKNOWN:    return "UNKNOWN";
    case CONN_CONNECTING: return "CONNECT";
    case CONN_CONNECTED:  return "OK";
    case CONN_WEAK:       return "WEAK";
    case CONN_LOST:       return "LOST";
    default:              return "ERROR";
  }
}

// =============== GET CONNECTION STATE ICON ================================
inline char getConnectionIcon(ConnectionState state) {
  switch (state) {
    case CONN_UNKNOWN:    return '?';
    case CONN_CONNECTING: return '~';
    case CONN_CONNECTED:  return '*';  // Good
    case CONN_WEAK:       return '!';  // Warning
    case CONN_LOST:       return 'X';  // Error
    default:              return '#';
  }
}

// =============== UPDATE CONNECTION STATE ================================
// Call this regularly in receiver loop
inline void updateConnectionState(HealthMonitor& health, DeviceState& remote) {
  unsigned long now = millis();
  unsigned long timeSinceLastMsg = now - remote.lastMessageTime;
  ConnectionState oldState = health.state;
  ConnectionState newState = health.state;

  // Determine new state based on time and RSSI
  if (timeSinceLastMsg > watchdogCfg.lostTimeout) {
    // No messages for > 8 seconds
    newState = CONN_LOST;
  }
  else if (timeSinceLastMsg > watchdogCfg.weakTimeout ||
           remote.rssi < watchdogCfg.weakRssiThreshold) {
    // No messages for 3-8 seconds OR weak signal
    newState = CONN_WEAK;
  }
  else if (timeSinceLastMsg < watchdogCfg.weakTimeout &&
           remote.rssi >= watchdogCfg.weakRssiThreshold) {
    // Messages recent and signal good
    newState = CONN_CONNECTED;

    // Track connected time
    if (oldState != CONN_CONNECTED) {
      health.connectedSince = now;
    }
  }

  // State changed?
  if (newState != oldState) {
    health.state = newState;
    health.stateChangeTime = now;

    // Log state change
    Serial.print("\n╔════════ CONNECTION STATE CHANGE ════════╗\n");
    Serial.print("║ ");
    Serial.print(getConnectionStateString(oldState));
    Serial.print(" -> ");
    Serial.print(getConnectionStateString(newState));
    Serial.println();
    Serial.print("║ Time since last message: ");
    Serial.print(timeSinceLastMsg / 1000.0, 1);
    Serial.println(" s");
    Serial.print("║ RSSI: ");
    Serial.print(remote.rssi);
    Serial.println(" dBm");
    Serial.println("╚═══════════════════════════════════════╝\n");
  }
}

// =============== ATTEMPT RECOVERY ================================
// Try to recover lost connection by re-initializing LoRa
inline bool attemptRecovery(HealthMonitor& health, uint8_t myAddress, uint8_t networkID) {
  unsigned long now = millis();

  // Check if we should attempt recovery
  if (health.state != CONN_LOST) {
    return false;  // Only recover from LOST state
  }

  // Check recovery cooldown
  if (now - health.lastRecoveryAttempt < watchdogCfg.recoveryInterval) {
    return false;  // Too soon
  }

  // Check max attempts
  if (health.recoveryAttempts >= watchdogCfg.maxRecoveryAttempts) {
    Serial.println("❌ Max recovery attempts reached. Manual intervention needed.");
    return false;
  }

  // Attempt recovery
  health.recoveryAttempts++;
  health.lastRecoveryAttempt = now;

  Serial.println("\n╔════════════════════════════════════╗");
  Serial.print("║ RECOVERY ATTEMPT #");
  Serial.println(health.recoveryAttempts);
  Serial.println("║ Re-initializing LoRa module...");
  Serial.println("╚════════════════════════════════════╝");

  // Re-initialize LoRa (from lora_handler.h)
  bool success = initLoRa(myAddress, networkID);

  if (success) {
    Serial.println("✓ Recovery successful!");
    health.state = CONN_CONNECTING;
    health.stateChangeTime = now;
    health.recoveryAttempts = 0;  // Reset counter on success
    return true;
  } else {
    Serial.println("❌ Recovery failed");
    return false;
  }
}

// =============== PRINT HEALTH REPORT ================================
inline void printHealthReport(HealthMonitor& health, DeviceState& remote) {
  Serial.println("\n╔═══════════════════════════════════════╗");
  Serial.println("║        HEALTH MONITOR REPORT         ║");
  Serial.println("╠═══════════════════════════════════════╣");

  // Connection status
  Serial.print("║ Status:     ");
  Serial.print(getConnectionStateString(health.state));
  Serial.print(" ");
  Serial.println(getConnectionIcon(health.state));

  unsigned long uptime = (millis() - health.startTime) / 1000;
  Serial.print("║ Uptime:     ");
  Serial.print(uptime);
  Serial.println(" s");

  if (health.state == CONN_CONNECTED) {
    unsigned long connTime = (millis() - health.connectedSince) / 1000;
    Serial.print("║ Connected:  ");
    Serial.print(connTime);
    Serial.println(" s");
  }

  // RSSI statistics
  Serial.println("╠═══════════════════════════════════════╣");
  Serial.print("║ RSSI Avg:   ");
  Serial.print(getRSSIAverage(health));
  Serial.println(" dBm");
  Serial.print("║ RSSI Min:   ");
  Serial.print(health.rssiMin);
  Serial.println(" dBm");
  Serial.print("║ RSSI Max:   ");
  Serial.print(health.rssiMax);
  Serial.println(" dBm");
  Serial.print("║ Samples:    ");
  Serial.println(health.rssiSamples);

  // Packet statistics
  Serial.println("╠═══════════════════════════════════════╣");
  Serial.print("║ Packets RX: ");
  Serial.println(health.packetsReceived);
  Serial.print("║ Lost:       ");
  Serial.print(health.packetsLost);
  Serial.print(" (");
  Serial.print(getPacketLoss(health), 1);
  Serial.println("%)");
  Serial.print("║ Duplicate:  ");
  Serial.println(health.packetsDuplicate);

  Serial.println("╚═══════════════════════════════════════╝\n");
}

// =============== GET UPTIME STRING ================================
inline String getUptimeString(HealthMonitor& health) {
  unsigned long seconds = (millis() - health.startTime) / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;

  if (hours > 0) {
    return String(hours) + "h " + String(minutes % 60) + "m";
  } else if (minutes > 0) {
    return String(minutes) + "m " + String(seconds % 60) + "s";
  } else {
    return String(seconds) + "s";
  }
}

#endif // HEALTH_MONITOR_H
