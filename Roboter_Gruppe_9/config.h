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

// =============== FEATURE FLAGS ================================
// 🚀 EXPERIMENTAL FEATURES - Easily enable/disable for testing
// Each feature can be tested independently
// See FEATURE_TESTING_GUIDE.md for testing instructions

// FEATURE 1: Battery Monitoring
// Monitors battery voltage and reports low battery warnings
// Hardware: Connect battery to GPIO 35 via voltage divider (2:1 ratio)
// Testing: Enable and check serial output for voltage readings
#define ENABLE_BATTERY_MONITOR false
#define BATTERY_PIN 35                   // ADC1_CH7 (ADC2 conflicts with WiFi!)
#define BATTERY_VOLTAGE_DIVIDER 2.0      // Voltage divider ratio (R1=R2)
#define BATTERY_CHECK_INTERVAL 60000     // Check every 60 seconds
#define BATTERY_LOW_THRESHOLD 3.3        // Warning below 3.3V
#define BATTERY_CRITICAL_THRESHOLD 3.0   // Critical below 3.0V

// FEATURE 2: Runtime Configuration via Serial
// Change settings on-the-fly via serial commands without re-uploading code
// Commands: CONFIG:INTERVAL:2000, CONFIG:SF:10, CONFIG:POWER:15
// Testing: Enable and send commands via Serial Monitor
#define ENABLE_RUNTIME_CONFIG false
#define CONFIG_COMMAND_PREFIX "CONFIG:"  // Command prefix to avoid conflicts

// FEATURE 3: WiFi Access Point + Web Interface
// Creates WiFi AP for configuration and monitoring via web browser
// Access at: http://192.168.4.1 after connecting to "LoRa_Roboter_9" WiFi
// Testing: Enable, connect to WiFi, open browser
#define ENABLE_WIFI_AP false
#define WIFI_AP_SSID "LoRa_Roboter_9"    // WiFi network name
#define WIFI_AP_PASSWORD "roboter123"    // WiFi password (min 8 chars)
#define WIFI_AP_CHANNEL 6                // WiFi channel (1-13)
#define WEB_SERVER_PORT 80               // HTTP port

// FEATURE 4: Advanced Remote Commands
// Extended command set for remote control and diagnostics
// Commands: CMD:STATUS, CMD:RESET_STATS, CMD:SET_POWER:10, etc.
// Testing: Enable and send commands from remote device
#define ENABLE_ADVANCED_COMMANDS false

// FEATURE 5: Performance Monitoring
// Track CPU usage, memory, loop frequency, and system health
// Prints performance report every 60 seconds
// Testing: Enable and check serial for performance metrics
#define ENABLE_PERFORMANCE_MONITOR false
#define PERF_REPORT_INTERVAL 60000       // Report every 60 seconds

// FEATURE 6: Watchdog Timer
// Automatically reboot if system hangs
// Timeout: 10 seconds (system must call esp_task_wdt_reset() regularly)
// Testing: Enable and verify system stays stable
#define ENABLE_WATCHDOG false
#define WATCHDOG_TIMEOUT_S 10            // Timeout in seconds

// FEATURE 7: Data Encryption (Simple XOR)
// Encrypt LoRa payload with XOR cipher (basic obfuscation)
// Note: NOT cryptographically secure, use for basic privacy only
// Testing: Enable on both devices with same key
#define ENABLE_ENCRYPTION false
#define ENCRYPTION_KEY 0xA5              // XOR encryption key (0x00-0xFF)

// FEATURE 8: Extended Telemetry
// Additional data in payload: uptime, free heap, temperature
// Increases packet size, may reduce reliability at long range
// Testing: Enable and check if data appears in CSV output
#define ENABLE_EXTENDED_TELEMETRY false

// FEATURE 9: Adaptive Spreading Factor
// Automatically adjust SF based on signal quality
// Improves throughput at close range, maintains link at long range
// Testing: Enable and monitor SF changes in serial output
#define ENABLE_ADAPTIVE_SF false
#define ADAPTIVE_SF_RSSI_GOOD -80        // Decrease SF above this RSSI
#define ADAPTIVE_SF_RSSI_WEAK -105       // Increase SF below this RSSI

// FEATURE 10: Packet Statistics Logging
// Detailed statistics: retries, duplicates, out-of-order packets
// Useful for debugging and performance analysis
// Testing: Enable and review detailed stats in serial output
#define ENABLE_PACKET_STATS false
#define PACKET_STATS_INTERVAL 30000      // Report every 30 seconds

#endif // CONFIG_H