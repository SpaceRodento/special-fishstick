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
};

// =============== SPINNER DATA STRUCTURE ================================
struct SpinnerData {
  char symbols[4];
  int index;
  unsigned long lastUpdate;
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