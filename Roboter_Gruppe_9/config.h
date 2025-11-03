/*=====================================================================
  config.h - Configuration

  All pin definitions and configuration constants for the project.

  Role Detection:
  - MODE_SELECT_PIN (GPIO16) is read with internal pull-up
  - MODE_GND_PIN (GPIO17) provides GND reference
  - When GPIO16 is connected to GPIO17: RECEIVER mode
  - When GPIO16 is floating (no connection): SENDER mode
  - Note: GPIO16 and GPIO17 are physically next to each other
=======================================================================*/

#ifndef CONFIG_H
#define CONFIG_H

// =============== PIN DEFINITIONS ================================
#define LED_PIN 2
#define RXD2 25      // RYLR896 TX -> ESP32 GPIO25
#define TXD2 26      // RYLR896 RX -> ESP32 GPIO26
#define TOUCH_PIN T0

// =============== MODE DETECTION ================================
#define MODE_SELECT_PIN 16
#define MODE_GND_PIN 17

// =============== LoRa CONFIGURATION ================================
#define LORA_RECEIVER_ADDRESS 1   // Receiver ID
#define LORA_SENDER_ADDRESS 2     // Sender ID
#define LORA_NETWORK_ID 6         // Network ID (sama molemmilla!)
#define LORA_BAUDRATE 115200      // RYLR896 baudrate

// =============== COMMUNICATION ================================
#define SERIAL2_BAUDRATE 115200   // Sama kuin LORA_BAUDRATE (yhteensopivuus)
#define MAX_RX_BUFFER 256

// =============== TIMEOUTS ================================
#define RX_TIMEOUT_WARNING 5000

// =============== LORA VALUES (yhteensopivuus vanhan koodin kanssa) ================================
#define LORA_NETWORK_ID_VALUE 6
#define LORA_BROADCAST_ADDR 0
#define LORA_BAND 868

// =============== DEBUG ================================
#define DEBUG_LORA_AT true

// =============== PC DATA LOGGING ================================
#define ENABLE_CSV_OUTPUT true       // Enable CSV data output for Python logging
#define ENABLE_JSON_OUTPUT false     // Enable JSON data output (alternative format)
#define DATA_OUTPUT_INTERVAL 2000    // Output interval in ms (2 seconds)

// =============== BI-DIRECTIONAL COMMUNICATION ================================
#define ENABLE_BIDIRECTIONAL true    // Enable two-way communication
#define ACK_INTERVAL 5               // Send ACK every N messages (receiver)
#define LISTEN_TIMEOUT 500           // Time sender listens for response (ms)

#endif // CONFIG_H