/*=====================================================================
  detailed_telemetry.h - Unified Detailed Statistics & Telemetry

  FEATURES 8 & 10: Yhdistetty telemetria ja pakettitilastot

  Yhdistää kaksi moduulia yhteen kokonaisuuteen:
  1. Pakettitilastot (packet_stats.h) - SNR, ajoitus, jitter
  2. Järjestelmätelemetria (extended_telemetry.h) - uptime, muisti, lämpötila

  TÄRKEÄ MUUTOS: Ei enää duplikaatteja!
  - RSSI-tilastot → health_monitor.h (yksi lähde)
  - Pakettihäviö → health_monitor.h (yksi lähde)
  - SNR-tilastot → detailed_telemetry.h (uniikki)
  - Ajoitustilastot → detailed_telemetry.h (uniikki)
  - Järjestelmätilastot → detailed_telemetry.h (uniikki)

  ═══════════════════════════════════════════════════════════════════

  PAKETTITILASTOT (packet_stats.h toiminnallisuus):

  SNR-tilastot:
  - Min/max/keskiarvo SNR
  - SNR-näytteiden määrä
  - Automaattinen päivitys vastaanotetuista paketeista

  Ajoitustilastot:
  - Min/max/keskiarvo pakettien väli
  - Jitter (vaihtelun keskihajonta)
  - Intervallilaskenta

  Häviöputket (loss streaks):
  - Nykyinen häviöputki
  - Pisin häviöputki
  - Häviöputkien määrä

  Duplikaatit ja järjestysvirheet:
  - Duplikaattien laskenta
  - Järjestyksen rikkoneiden pakettien laskenta

  Lähetystilastot:
  - Lähetetyt paketit
  - ACK-vastaukset
  - ACK-aikakatkaisut

  ═══════════════════════════════════════════════════════════════════

  JÄRJESTELMÄTELEMETRIA (extended_telemetry.h toiminnallisuus):

  Järjestelmätiedot:
  - Käynnissäoloaika (uptime)
  - Vapaa heap-muisti (KB)
  - Minimi vapaa heap (muistivuotojen havaitseminen)
  - Sisäinen lämpötila (°C)
  - Loop-taajuus (Hz)
  - WiFi RSSI (jos WiFi käytössä)

  Terveystarkistukset:
  - Alhainen muisti -varoitus (<50 KB)
  - Korkea lämpötila -varoitus (>85°C)
  - Hidas loop -varoitus (<10 Hz)

  ═══════════════════════════════════════════════════════════════════

  INTEGRAATIO HEALTH_MONITOR.H:N KANSSA:

  detailed_telemetry.h KÄYTTÄÄ health_monitor.h -dataa:
  - getRSSIAverage() → RSSI keskiarvo
  - health.rssiMin → RSSI minimi
  - health.rssiMax → RSSI maksimi
  - getPacketLoss() → pakettihäviö %
  - health.packetsReceived → vastaanotetut paketit
  - health.packetsLost → hävinneet paketit

  detailed_telemetry.h LISÄÄ omaa dataa:
  - SNR min/max/avg (health_monitor ei seuraa SNR:ää)
  - Ajoitustilastot (interval, jitter)
  - Häviöputket (loss streaks)
  - Järjestelmätelemetria (uptime, heap, temp)

  ═══════════════════════════════════════════════════════════════════

  API:

  void initDetailedTelemetry()
    - Alustaa molemmat moduulit

  void recordPacketReceived(int rssi, int snr, int sequence)
    - Tallentaa vastaanotetun paketin (SNR + ajoitus)

  void recordPacketLost()
    - Tallentaa hävinneen paketin (häviöputket)

  void recordDuplicate(int sequence)
    - Tallentaa duplikaattipaketin

  void updateTelemetry()
    - Päivittää järjestelmätelemetrian

  void printDetailedReport(HealthMonitor& health)
    - Tulostaa kaikki tilastot (packet stats + telemetry)

  String getTelemetryPayload()
    - Palauttaa telemetrian LoRa-payloadiin

  bool isSystemHealthy()
    - Tarkistaa järjestelmän terveyden

=======================================================================*/

#ifndef DETAILED_TELEMETRY_H
#define DETAILED_TELEMETRY_H

#include <Arduino.h>
#include "config.h"
#include "health_monitor.h"  // RSSI ja packet loss tulevat täältä!

#ifdef __cplusplus
extern "C" {
#endif

// ESP32 internal temperature sensor (if available)
uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

// ═══════════════════════════════════════════════════════════════════
// PACKET STATISTICS (unique data not in health_monitor)
// ═══════════════════════════════════════════════════════════════════

struct PacketStatistics {
  // SNR statistics (NOT in health_monitor)
  int snrMin;
  int snrMax;
  long snrSum;
  int snrCount;
  float snrAvg;

  // Timing statistics (NOT in health_monitor)
  unsigned long lastPacketTime;
  unsigned long minInterval;
  unsigned long maxInterval;
  unsigned long totalInterval;
  int intervalCount;
  float avgInterval;
  float jitter;  // Standard deviation of interval

  // Loss streaks (NOT in health_monitor)
  int currentLossStreak;
  int maxLossStreak;
  int totalStreaks;

  // Duplicates and out-of-order (NOT in health_monitor)
  unsigned long duplicates;
  unsigned long outOfOrder;

  // Transmission stats (NOT in health_monitor)
  unsigned long packetsSent;
  unsigned long transmissionAttempts;
  unsigned long ackReceived;
  unsigned long ackTimeout;

  // Reporting
  unsigned long lastReport;
  int reportCount;
};

// ═══════════════════════════════════════════════════════════════════
// SYSTEM TELEMETRY (from extended_telemetry.h)
// ═══════════════════════════════════════════════════════════════════

struct SystemTelemetry {
  unsigned long uptime;          // System uptime (seconds)
  int freeHeapKB;                // Free heap memory (KB)
  int minFreeHeapKB;             // Minimum free heap (KB)
  float temperature;             // Internal temperature (°C)
  int loopFrequency;             // Loop frequency (Hz)
  int wifiRSSI;                  // WiFi RSSI (if connected)
  unsigned long lastUpdate;      // Last telemetry update
  int updateCount;               // Number of updates
};

// Global instances
PacketStatistics pktStats = {
  999, -999, 0, 0, 0,            // SNR (init min=999, max=-999)
  0, 999999, 0, 0, 0, 0, 0,      // Timing
  0, 0, 0,                       // Loss streaks
  0, 0,                          // Duplicates, out-of-order
  0, 0, 0, 0,                    // Transmission
  0, 0                           // Reporting
};

SystemTelemetry sysTelem = {0, 0, 0, 0.0, 0, 0, 0, 0};

// ═══════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════

void initDetailedTelemetry() {
  #if ENABLE_PACKET_STATS || ENABLE_EXTENDED_TELEMETRY
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║  DETAILED TELEMETRY INIT               ║");
    Serial.println("╚════════════════════════════════════════╝");
  #endif

  #if ENABLE_PACKET_STATS
    pktStats.lastPacketTime = millis();
    pktStats.lastReport = millis();

    Serial.println("  📈 Packet statistics enabled");
    Serial.print("    Report interval: ");
    Serial.print(PACKET_STATS_INTERVAL / 1000);
    Serial.println(" seconds");
    Serial.println("    Tracking:");
    Serial.println("      - SNR min/max/avg");
    Serial.println("      - Packet timing and jitter");
    Serial.println("      - Loss streaks");
    Serial.println("      - Duplicates and out-of-order");
    Serial.println("    RSSI/Packet loss → health_monitor.h");
  #endif

  #if ENABLE_EXTENDED_TELEMETRY
    sysTelem.lastUpdate = millis();
    sysTelem.freeHeapKB = ESP.getFreeHeap() / 1024;
    sysTelem.minFreeHeapKB = ESP.getMinFreeHeap() / 1024;

    Serial.println("  📊 System telemetry enabled");
    Serial.println("    Monitoring:");
    Serial.println("      - System uptime");
    Serial.println("      - Free heap memory");
    Serial.println("      - Internal temperature");
    Serial.println("      - Loop frequency");
    Serial.println("    ⚠️  Payload size +35 bytes");
  #endif

  #if ENABLE_PACKET_STATS || ENABLE_EXTENDED_TELEMETRY
    Serial.println();
    Serial.println("Detailed telemetry ready.");
    Serial.println();
  #endif
}

// ═══════════════════════════════════════════════════════════════════
// PACKET STATISTICS FUNCTIONS
// ═══════════════════════════════════════════════════════════════════

// Record received packet (SNR + timing only, RSSI handled by health_monitor)
void recordPacketReceived(int rssi, int snr, int sequence) {
  #if ENABLE_PACKET_STATS
    unsigned long now = millis();

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

// Record lost packet (loss streak only, count handled by health_monitor)
void recordPacketLost() {
  #if ENABLE_PACKET_STATS
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

// ═══════════════════════════════════════════════════════════════════
// SYSTEM TELEMETRY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════

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
    sysTelem.uptime = now / 1000;

    // Memory stats
    sysTelem.freeHeapKB = ESP.getFreeHeap() / 1024;
    sysTelem.minFreeHeapKB = ESP.getMinFreeHeap() / 1024;

    // Temperature
    sysTelem.temperature = readInternalTemperature();

    // Loop frequency (if performance monitor enabled)
    #if ENABLE_PERFORMANCE_MONITOR
      extern PerformanceMetrics perf;
      sysTelem.loopFrequency = perf.loopFrequency;
    #else
      sysTelem.loopFrequency = 0;
    #endif

    // WiFi RSSI (if WiFi enabled)
    #if ENABLE_WIFI_AP
      if (WiFi.status() == WL_CONNECTED) {
        sysTelem.wifiRSSI = WiFi.RSSI();
      } else {
        sysTelem.wifiRSSI = 0;
      }
    #else
      sysTelem.wifiRSSI = 0;
    #endif

    sysTelem.lastUpdate = now;
    sysTelem.updateCount++;
  #endif
}

// Build extended telemetry string for payload
String getTelemetryPayload() {
  #if ENABLE_EXTENDED_TELEMETRY
    updateTelemetry();

    String telem = "";

    // Uptime
    telem += ",UP:" + String(sysTelem.uptime);

    // Free heap
    telem += ",HEAP:" + String(sysTelem.freeHeapKB);

    // Min heap (memory leak indicator)
    telem += ",MHEAP:" + String(sysTelem.minFreeHeapKB);

    // Temperature
    telem += ",TEMP:" + String(sysTelem.temperature, 1);

    // Loop frequency (if available)
    if (sysTelem.loopFrequency > 0) {
      telem += ",LOOP:" + String(sysTelem.loopFrequency);
    }

    // WiFi RSSI (if available)
    if (sysTelem.wifiRSSI != 0) {
      telem += ",WIFI:" + String(sysTelem.wifiRSSI);
    }

    return telem;
  #else
    return "";
  #endif
}

// Check if system health is good
bool isSystemHealthy() {
  #if ENABLE_EXTENDED_TELEMETRY
    updateTelemetry();

    // Check memory
    if (sysTelem.freeHeapKB < 50) {
      return false;  // Low memory
    }

    // Check temperature
    if (sysTelem.temperature > 85.0) {
      return false;  // Too hot
    }

    // Check loop frequency
    if (sysTelem.loopFrequency > 0 && sysTelem.loopFrequency < 10) {
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
      if (sysTelem.freeHeapKB < 50) issues += " LOW_MEM";
      if (sysTelem.temperature > 85.0) issues += " HIGH_TEMP";
      if (sysTelem.loopFrequency < 10 && sysTelem.loopFrequency > 0) issues += " SLOW_LOOP";
      return issues;
    }
  #else
    return "DISABLED";
  #endif
}

// ═══════════════════════════════════════════════════════════════════
// UNIFIED REPORTING
// ═══════════════════════════════════════════════════════════════════

// Print detailed statistics report (combines everything)
void printDetailedReport(HealthMonitor& health) {
  #if ENABLE_PACKET_STATS || ENABLE_EXTENDED_TELEMETRY
    unsigned long now = millis();

    // Check if it's time to report
    #if ENABLE_PACKET_STATS
    if (now - pktStats.lastReport < PACKET_STATS_INTERVAL) {
      return;
    }
    pktStats.lastReport = now;
    pktStats.reportCount++;
    #endif

    Serial.println("\n╔═══════════════ DETAILED TELEMETRY REPORT ═══════════════╗");

    #if ENABLE_PACKET_STATS
    Serial.print("║ Report #");
    Serial.println(pktStats.reportCount);
    #endif

    // ═══ PACKET RECEPTION (uses health_monitor data) ═══
    #if ENABLE_PACKET_STATS
    Serial.println("║");
    Serial.println("║ PACKET RECEPTION (from health_monitor):");
    Serial.print("║   Packets received:    ");
    Serial.println(health.packetsReceived);
    Serial.print("║   Packets lost:        ");
    Serial.print(health.packetsLost);
    Serial.print(" (");
    Serial.print(getPacketLoss(health), 2);
    Serial.println("%)");
    Serial.print("║   Duplicates:          ");
    Serial.println(pktStats.duplicates);
    Serial.print("║   Out-of-order:        ");
    Serial.println(pktStats.outOfOrder);
    #endif

    // ═══ TRANSMISSION STATS ═══
    #if ENABLE_PACKET_STATS
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
    #endif

    // ═══ RSSI STATS (uses health_monitor data) ═══
    #if ENABLE_PACKET_STATS
    if (health.rssiSamples > 0) {
      Serial.println("║");
      Serial.println("║ RSSI (dBm) (from health_monitor):");
      Serial.print("║   Average:             ");
      Serial.println(getRSSIAverage(health), 1);
      Serial.print("║   Min:                 ");
      Serial.println(health.rssiMin);
      Serial.print("║   Max:                 ");
      Serial.println(health.rssiMax);
      Serial.print("║   Range:               ");
      Serial.println(health.rssiMax - health.rssiMin);
    }
    #endif

    // ═══ SNR STATS (unique to detailed_telemetry) ═══
    #if ENABLE_PACKET_STATS
    if (pktStats.snrCount > 0) {
      Serial.println("║");
      Serial.println("║ SNR (dB) (unique to detailed_telemetry):");
      Serial.print("║   Average:             ");
      Serial.println(pktStats.snrAvg, 1);
      Serial.print("║   Min:                 ");
      Serial.println(pktStats.snrMin);
      Serial.print("║   Max:                 ");
      Serial.println(pktStats.snrMax);
    }
    #endif

    // ═══ TIMING STATS (unique to detailed_telemetry) ═══
    #if ENABLE_PACKET_STATS
    if (pktStats.intervalCount > 0) {
      Serial.println("║");
      Serial.println("║ TIMING (unique to detailed_telemetry):");
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
    #endif

    // ═══ LOSS STREAKS (unique to detailed_telemetry) ═══
    #if ENABLE_PACKET_STATS
    Serial.println("║");
    Serial.println("║ LOSS STREAKS (unique to detailed_telemetry):");
    Serial.print("║   Current streak:      ");
    Serial.println(pktStats.currentLossStreak);
    Serial.print("║   Max streak:          ");
    Serial.println(pktStats.maxLossStreak);
    Serial.print("║   Total streaks:       ");
    Serial.println(pktStats.totalStreaks);
    #endif

    // ═══ SYSTEM TELEMETRY ═══
    #if ENABLE_EXTENDED_TELEMETRY
    updateTelemetry();

    Serial.println("║");
    Serial.println("║ SYSTEM TELEMETRY:");

    Serial.print("║   Uptime:              ");
    if (sysTelem.uptime < 60) {
      Serial.print(sysTelem.uptime);
      Serial.println(" s");
    } else if (sysTelem.uptime < 3600) {
      Serial.print(sysTelem.uptime / 60);
      Serial.print(" min ");
      Serial.print(sysTelem.uptime % 60);
      Serial.println(" s");
    } else {
      Serial.print(sysTelem.uptime / 3600);
      Serial.print(" h ");
      Serial.print((sysTelem.uptime % 3600) / 60);
      Serial.println(" min");
    }

    Serial.print("║   Free heap:           ");
    Serial.print(sysTelem.freeHeapKB);
    Serial.print(" KB");
    if (sysTelem.freeHeapKB < 50) {
      Serial.print(" ⚠️  LOW!");
    }
    Serial.println();

    Serial.print("║   Min heap:            ");
    Serial.print(sysTelem.minFreeHeapKB);
    Serial.println(" KB");

    Serial.print("║   Temperature:         ");
    Serial.print(sysTelem.temperature, 1);
    Serial.print(" °C");
    if (sysTelem.temperature > 80.0) {
      Serial.print(" ⚠️  HIGH!");
    }
    Serial.println();

    if (sysTelem.loopFrequency > 0) {
      Serial.print("║   Loop frequency:      ");
      Serial.print(sysTelem.loopFrequency);
      Serial.print(" Hz");
      if (sysTelem.loopFrequency < 10) {
        Serial.print(" ⚠️  SLOW!");
      }
      Serial.println();
    }

    if (sysTelem.wifiRSSI != 0) {
      Serial.print("║   WiFi RSSI:           ");
      Serial.print(sysTelem.wifiRSSI);
      Serial.println(" dBm");
    }

    Serial.print("║   Health status:       ");
    Serial.println(getHealthStatus());
    #endif

    Serial.println("╚═════════════════════════════════════════════════════════╝\n");
  #endif
}

// Reset statistics (for testing)
void resetDetailedStats() {
  #if ENABLE_PACKET_STATS
    Serial.println("🔄 Resetting detailed statistics...");

    // Reset packet stats (keep SNR, timing, streaks only - RSSI/loss in health_monitor)
    pktStats.duplicates = 0;
    pktStats.outOfOrder = 0;
    pktStats.packetsSent = 0;
    pktStats.transmissionAttempts = 0;
    pktStats.ackReceived = 0;
    pktStats.ackTimeout = 0;
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

    Serial.println("✓ Packet statistics reset");
  #endif

  #if ENABLE_EXTENDED_TELEMETRY
    // Don't reset telemetry counters, they track cumulative data
    Serial.println("  (System telemetry continues running)");
  #endif
}

// Get CSV stats summary (for data logging)
String getDetailedStatsCSV(HealthMonitor& health) {
  #if ENABLE_PACKET_STATS || ENABLE_EXTENDED_TELEMETRY
    String csv = "";

    #if ENABLE_PACKET_STATS
    // Packet stats (use health_monitor for RSSI/loss)
    csv += String(health.packetsReceived) + ",";
    csv += String(health.packetsLost) + ",";
    csv += String(getPacketLoss(health), 2) + ",";
    csv += String(getRSSIAverage(health), 1) + ",";
    csv += String(pktStats.snrAvg, 1) + ",";
    csv += String(pktStats.avgInterval, 0) + ",";
    csv += String(pktStats.jitter, 1) + ",";
    #else
    csv += "0,0,0,0,0,0,0,";
    #endif

    #if ENABLE_EXTENDED_TELEMETRY
    updateTelemetry();
    // System telemetry
    csv += String(sysTelem.uptime) + ",";
    csv += String(sysTelem.freeHeapKB) + ",";
    csv += String(sysTelem.temperature, 1);
    #else
    csv += "0,0,0";
    #endif

    return csv;
  #else
    return "0,0,0,0,0,0,0,0,0,0";
  #endif
}

#endif // DETAILED_TELEMETRY_H
