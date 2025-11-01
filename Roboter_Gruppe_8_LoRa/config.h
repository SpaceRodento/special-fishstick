/*=====================================================================
  config.h - Configuration

  All pin definitions and configuration constants for the project.

  Role Detection:
  - MODE_SELECT_PIN (GPIO15) is read with internal pull-up
  - MODE_GND_PIN (GPIO17) provides GND reference
  - When GPIO15 is connected to GPIO17: RECEIVER mode
  - When GPIO15 is floating: SENDER mode
=======================================================================*/

#ifndef CONFIG_H
#define CONFIG_H

// =============== PIN DEFINITIONS ================================
#define LED_PIN 2
#define RXD2 25      // RYLR896 TX -> ESP32 GPIO25
#define TXD2 26      // RYLR896 RX -> ESP32 GPIO26
#define TOUCH_PIN T0

// =============== MODE DETECTION ================================
#define MODE_SELECT_PIN 15
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

#endif // CONFIG_H