/*=====================================================================
  packet_stats.h - Detailed Packet Statistics

  FEATURE 10: Packet Statistics Logging

  Tracks detailed statistics about packet transmission and reception:
  - Duplicate packets detected
  - Out-of-order packets
  - Retransmission attempts
  - Average/min/max RSSI
  - Average/min/max SNR
  - Packet jitter (timing variance)
  - Sequential packet loss streaks
  - Recovery success rate

  Benefits:
  - Deep debugging of connection issues
  - Performance analysis for different ranges/SF/power
  - Identify intermittent problems
  - Optimize LoRa parameters
  - Scientific data for research

  Use Cases:
  - Field testing new deployments
  - Range tests with detailed metrics
  - Troubleshooting packet loss
  - Comparing different LoRa configurations
  - Academic research

  Configuration:
  - PACKET_STATS_INTERVAL: Report interval (30 seconds default)
  - Stats are continuously updated
  - Reports printed to serial periodically

  Testing:
  1. Set ENABLE_PACKET_STATS true in config.h
  2. Upload code
  3. Wait 30 seconds
  4. Check serial output for detailed statistics
  5. Move devices to test range impact

  Performance Impact:
  - Memory: ~100 bytes RAM
  - CPU: Negligible (<0.1ms per packet)
  - No impact on communication

  Output Format:
  - Serial: Human-readable report every 30s
  - CSV: Optional extended stats in data output
=======================================================================*/

#ifndef PACKET_STATS_H
#define PACKET_STATS_H

#include <Arduino.h>
#include "config.h"

// Detailed packet statistics
struct PacketStatistics {
  // Reception stats
  unsigned long packetsReceived;
  unsigned long packetsLost;
  unsigned long duplicates;
  unsigned long outOfOrder;

  // Transmission stats
  unsigned long packetsSent;
  unsigned long transmissionAttempts;
  unsigned long ackReceived;
  unsigned long ackTimeout;

  // RSSI statistics
  int rssiMin;
  int rssiMax;
  long rssiSum;
  int rssiCount;
  float rssiAvg;

  // SNR statistics
  int snrMin;
  int snrMax;
  long snrSum;
  int snrCount;
  float snrAvg;

  // Timing statistics
  unsigned long lastPacketTime;
  unsigned long minInterval;
  unsigned long maxInterval;
  unsigned long totalInterval;
  int intervalCount;
  float avgInterval;
  float jitter;  // Standard deviation of interval

  // Loss streaks
  int currentLossStreak;
  int maxLossStreak;
  int totalStreaks;

  // Recovery stats
  int recoveryAttempts;
  int successfulRecoveries;

  // Reporting
  unsigned long lastReport;
  int reportCount;
};

PacketStatistics pktStats = {
  0, 0, 0, 0,          // Reception
  0, 0, 0, 0,          // Transmission
  999, -999, 0, 0, 0,  // RSSI (init min=999, max=-999)
  999, -999, 0, 0, 0,  // SNR
  0, 999999, 0, 0, 0, 0, 0,  // Timing
  0, 0, 0,             // Loss streaks
  0, 0,                // Recovery
  0, 0                 // Reporting
};

// Initialize packet statistics
void initPacketStats() {
  #if ENABLE_PACKET_STATS
    pktStats.lastPacketTime = millis();
    pktStats.lastReport = millis();

    Serial.println("📈 Packet statistics enabled");
    Serial.print("  Report interval: ");
    Serial.print(PACKET_STATS_INTERVAL / 1000);
    Serial.println(" seconds");
    Serial.println("  Tracking:");
    Serial.println("    - Duplicates, out-of-order packets");
    Serial.println("    - RSSI/SNR min/max/avg");
    Serial.println("    - Packet timing and jitter");
    Serial.println("    - Loss streaks and recovery");
  #endif
}

// Record received packet
void recordPacketReceived(int rssi, int snr, int sequence) {
  #if ENABLE_PACKET_STATS
    unsigned long now = millis();

    pktStats.packetsReceived++;

    // Update RSSI stats
    if (rssi < pktStats.rssiMin) pktStats.rssiMin = rssi;
    if (rssi > pktStats.rssiMax) pktStats.rssiMax = rssi;
    pktStats.rssiSum += rssi;
    pktStats.rssiCount++;
    pktStats.rssiAvg = (float)pktStats.rssiSum / (float)pktStats.rssiCount;

    // Update SNR stats
    if (snr < pktStats.snrMin) pktStats.snrMin = snr;
    if (snr > pktStats.snrMax) pktStats.snrMax = snr;
    pktStats.snrSum += snr;
    pktStats.snrCount++;
    pktStats.snrAvg = (float)pktStats.snrSum / (float)pktStats.snrCount;

    // Update timing stats
    if (pktStats.lastPacketTime > 0) {
      unsigned long interval = now - pktStats.lastPacketTime;

      if (interval < pktStats.minInterval) pktStats.minInterval = interval;
      if (interval > pktStats.maxInterval) pktStats.maxInterval = interval;

      pktStats.totalInterval += interval;
      pktStats.intervalCount++;
      pktStats.avgInterval = (float)pktStats.totalInterval / (float)pktStats.intervalCount;

      // Simple jitter calculation (difference from average)
      float deviation = abs((float)interval - pktStats.avgInterval);
      pktStats.jitter = (pktStats.jitter * 0.9) + (deviation * 0.1);  // Moving average
    }

    pktStats.lastPacketTime = now;

    // Reset loss streak (packet received successfully)
    if (pktStats.currentLossStreak > 0) {
      if (pktStats.currentLossStreak > pktStats.maxLossStreak) {
        pktStats.maxLossStreak = pktStats.currentLossStreak;
      }
      pktStats.totalStreaks++;
      pktStats.currentLossStreak = 0;
    }
  #endif
}

// Record lost packet
void recordPacketLost() {
  #if ENABLE_PACKET_STATS
    pktStats.packetsLost++;
    pktStats.currentLossStreak++;
  #endif
}

// Record duplicate packet
void recordDuplicate(int sequence) {
  #if ENABLE_PACKET_STATS
    pktStats.duplicates++;
    Serial.print("📋 Duplicate packet: SEQ:");
    Serial.println(sequence);
  #endif
}

// Record out-of-order packet
void recordOutOfOrder(int expected, int received) {
  #if ENABLE_PACKET_STATS
    pktStats.outOfOrder++;
    Serial.print("🔀 Out-of-order packet: Expected SEQ:");
    Serial.print(expected);
    Serial.print(", Got:");
    Serial.println(received);
  #endif
}

// Record transmitted packet
void recordPacketSent() {
  #if ENABLE_PACKET_STATS
    pktStats.packetsSent++;
    pktStats.transmissionAttempts++;
  #endif
}

// Record ACK received
void recordAckReceived() {
  #if ENABLE_PACKET_STATS
    pktStats.ackReceived++;
  #endif
}

// Record ACK timeout
void recordAckTimeout() {
  #if ENABLE_PACKET_STATS
    pktStats.ackTimeout++;
  #endif
}

// Record recovery attempt
void recordRecoveryAttempt(bool successful) {
  #if ENABLE_PACKET_STATS
    pktStats.recoveryAttempts++;
    if (successful) {
      pktStats.successfulRecoveries++;
    }
  #endif
}

// Calculate packet loss percentage
float calculatePacketLoss() {
  #if ENABLE_PACKET_STATS
    unsigned long total = pktStats.packetsReceived + pktStats.packetsLost;
    if (total == 0) return 0.0;
    return ((float)pktStats.packetsLost / (float)total) * 100.0;
  #else
    return 0.0;
  #endif
}

// Calculate ACK success rate
float calculateAckRate() {
  #if ENABLE_PACKET_STATS
    unsigned long total = pktStats.ackReceived + pktStats.ackTimeout;
    if (total == 0) return 0.0;
    return ((float)pktStats.ackReceived / (float)total) * 100.0;
  #else
    return 0.0;
  #endif
}

// Calculate recovery success rate
float calculateRecoveryRate() {
  #if ENABLE_PACKET_STATS
    if (pktStats.recoveryAttempts == 0) return 0.0;
    return ((float)pktStats.successfulRecoveries / (float)pktStats.recoveryAttempts) * 100.0;
  #else
    return 0.0;
  #endif
}

// Print detailed statistics report
void printPacketStatsReport() {
  #if ENABLE_PACKET_STATS
    unsigned long now = millis();

    // Check if it's time to report
    if (now - pktStats.lastReport < PACKET_STATS_INTERVAL) {
      return;
    }

    pktStats.lastReport = now;
    pktStats.reportCount++;

    Serial.println("\n╔═══════════════ PACKET STATISTICS ═══════════════╗");
    Serial.print("║ Report #");
    Serial.println(pktStats.reportCount);

    // Reception stats
    Serial.println("║");
    Serial.println("║ RECEPTION:");
    Serial.print("║   Packets received:    ");
    Serial.println(pktStats.packetsReceived);
    Serial.print("║   Packets lost:        ");
    Serial.print(pktStats.packetsLost);
    Serial.print(" (");
    Serial.print(calculatePacketLoss(), 2);
    Serial.println("%)");
    Serial.print("║   Duplicates:          ");
    Serial.println(pktStats.duplicates);
    Serial.print("║   Out-of-order:        ");
    Serial.println(pktStats.outOfOrder);

    // Transmission stats (if available)
    if (pktStats.packetsSent > 0) {
      Serial.println("║");
      Serial.println("║ TRANSMISSION:");
      Serial.print("║   Packets sent:        ");
      Serial.println(pktStats.packetsSent);
      Serial.print("║   ACK received:        ");
      Serial.print(pktStats.ackReceived);
      Serial.print(" (");
      Serial.print(calculateAckRate(), 1);
      Serial.println("%)");
      Serial.print("║   ACK timeout:         ");
      Serial.println(pktStats.ackTimeout);
    }

    // RSSI stats
    if (pktStats.rssiCount > 0) {
      Serial.println("║");
      Serial.println("║ RSSI (dBm):");
      Serial.print("║   Average:             ");
      Serial.println(pktStats.rssiAvg, 1);
      Serial.print("║   Min:                 ");
      Serial.println(pktStats.rssiMin);
      Serial.print("║   Max:                 ");
      Serial.println(pktStats.rssiMax);
      Serial.print("║   Range:               ");
      Serial.println(pktStats.rssiMax - pktStats.rssiMin);
    }

    // SNR stats
    if (pktStats.snrCount > 0) {
      Serial.println("║");
      Serial.println("║ SNR (dB):");
      Serial.print("║   Average:             ");
      Serial.println(pktStats.snrAvg, 1);
      Serial.print("║   Min:                 ");
      Serial.println(pktStats.snrMin);
      Serial.print("║   Max:                 ");
      Serial.println(pktStats.snrMax);
    }

    // Timing stats
    if (pktStats.intervalCount > 0) {
      Serial.println("║");
      Serial.println("║ TIMING:");
      Serial.print("║   Avg interval:        ");
      Serial.print(pktStats.avgInterval, 0);
      Serial.println(" ms");
      Serial.print("║   Min interval:        ");
      Serial.print(pktStats.minInterval);
      Serial.println(" ms");
      Serial.print("║   Max interval:        ");
      Serial.print(pktStats.maxInterval);
      Serial.println(" ms");
      Serial.print("║   Jitter:              ");
      Serial.print(pktStats.jitter, 1);
      Serial.println(" ms");
    }

    // Loss streaks
    Serial.println("║");
    Serial.println("║ LOSS STREAKS:");
    Serial.print("║   Current streak:      ");
    Serial.println(pktStats.currentLossStreak);
    Serial.print("║   Max streak:          ");
    Serial.println(pktStats.maxLossStreak);
    Serial.print("║   Total streaks:       ");
    Serial.println(pktStats.totalStreaks);

    // Recovery stats
    if (pktStats.recoveryAttempts > 0) {
      Serial.println("║");
      Serial.println("║ RECOVERY:");
      Serial.print("║   Attempts:            ");
      Serial.println(pktStats.recoveryAttempts);
      Serial.print("║   Successful:          ");
      Serial.print(pktStats.successfulRecoveries);
      Serial.print(" (");
      Serial.print(calculateRecoveryRate(), 1);
      Serial.println("%)");
    }

    Serial.println("╚════════════════════════════════════════════════╝\n");
  #endif
}

// Get stats summary string (for CSV output)
String getPacketStatsString() {
  #if ENABLE_PACKET_STATS
    String stats = "";
    stats += String(pktStats.packetsReceived) + ",";
    stats += String(pktStats.packetsLost) + ",";
    stats += String(calculatePacketLoss(), 2) + ",";
    stats += String(pktStats.rssiAvg, 1) + ",";
    stats += String(pktStats.snrAvg, 1) + ",";
    stats += String(pktStats.avgInterval, 0) + ",";
    stats += String(pktStats.jitter, 1);
    return stats;
  #else
    return "0,0,0,0,0,0,0";
  #endif
}

// Reset statistics (for testing)
void resetPacketStats() {
  #if ENABLE_PACKET_STATS
    Serial.println("🔄 Resetting packet statistics...");

    pktStats.packetsReceived = 0;
    pktStats.packetsLost = 0;
    pktStats.duplicates = 0;
    pktStats.outOfOrder = 0;
    pktStats.packetsSent = 0;
    pktStats.transmissionAttempts = 0;
    pktStats.ackReceived = 0;
    pktStats.ackTimeout = 0;
    pktStats.rssiMin = 999;
    pktStats.rssiMax = -999;
    pktStats.rssiSum = 0;
    pktStats.rssiCount = 0;
    pktStats.snrMin = 999;
    pktStats.snrMax = -999;
    pktStats.snrSum = 0;
    pktStats.snrCount = 0;
    pktStats.lastPacketTime = millis();
    pktStats.minInterval = 999999;
    pktStats.maxInterval = 0;
    pktStats.totalInterval = 0;
    pktStats.intervalCount = 0;
    pktStats.currentLossStreak = 0;
    pktStats.maxLossStreak = 0;
    pktStats.totalStreaks = 0;
    pktStats.recoveryAttempts = 0;
    pktStats.successfulRecoveries = 0;

    Serial.println("✓ Statistics reset");
  #endif
}

#endif // PACKET_STATS_H
