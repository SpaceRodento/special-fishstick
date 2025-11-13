/*=====================================================================
  structs.h - Data Structures
=======================================================================*/

#ifndef STRUCTS_H
#define STRUCTS_H

// =============== DEVICE STATE STRUCTURE ================================
struct DeviceState {
  // LED status
  bool ledState;
  int ledCount;

  // Touch sensor
  bool touchState;
  unsigned long touchValue;

  // Communication
  int messageCount;
  unsigned long lastMessageTime;
  int sequenceNumber;  // For packet tracking

  // Spinner animation
  int spinnerIndex;

  // LoRa signal quality
  int rssi;  // Received Signal Strength Indicator (dBm)
  int snr;   // Signal-to-Noise Ratio (dB)
};

// =============== TIMING DATA STRUCTURE ================================
struct TimingData {
  unsigned long lastLED;
  unsigned long lastLCD;
  unsigned long lastSensor;
  unsigned long lastCheck;
  unsigned long lastSend;
  unsigned long lastSpinner;
  unsigned long lastHealthReport;
  unsigned long lastDataOutput;  // For CSV/JSON logging
};

// =============== SPINNER DATA STRUCTURE ================================
struct SpinnerData {
  char symbols[4];
  int index;
  unsigned long lastUpdate;
};

// =============== CONNECTION STATE ENUM ================================
enum ConnectionState {
  CONN_UNKNOWN = 0,
  CONN_CONNECTING = 1,
  CONN_CONNECTED = 2,
  CONN_WEAK = 3,
  CONN_LOST = 4
};

// =============== HEALTH MONITORING STRUCTURE ================================
struct HealthMonitor {
  // Connection state
  ConnectionState state;
  unsigned long stateChangeTime;
  unsigned long connectedSince;

  // RSSI statistics (sliding window)
  int rssiMin;
  int rssiMax;
  long rssiSum;      // For average calculation
  int rssiSamples;

  // Packet tracking
  int expectedSeq;        // Next expected sequence number
  int packetsReceived;
  int packetsLost;
  int packetsDuplicate;

  // Recovery attempts
  int recoveryAttempts;
  unsigned long lastRecoveryAttempt;
  bool maxAttemptsReachedNotified;  // Track if we've already notified about max attempts

  // Uptime
  unsigned long startTime;
};

// =============== WATCHDOG CONFIGURATION ================================
struct WatchdogConfig {
  unsigned long weakTimeout;      // Time to consider connection WEAK (ms)
  unsigned long lostTimeout;      // Time to consider connection LOST (ms)
  int weakRssiThreshold;          // RSSI below this = WEAK (-100 dBm)
  int criticalRssiThreshold;      // RSSI below this = CRITICAL (-110 dBm)
  unsigned long recoveryInterval; // Time between recovery attempts (ms)
  int maxRecoveryAttempts;        // Max recovery attempts before giving up
};

// =============== ERROR COUNTERS STRUCTURE ================================
struct ErrorCounters {
  int rxBufferOverflow;
  int parseErrors;
  int rxTimeouts;
  int loraATfails;
};

// =============== LORA CONFIG STRUCTURE ================================
struct LoRaConfig {
  uint16_t deviceAddress;
  uint8_t networkID;
  bool initialized;
  String firmwareVersion;
};

#endif // STRUCTS_H