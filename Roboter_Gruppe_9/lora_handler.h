/*=====================================================================
  lora_handler.h - RYLR896 LoRa Handler

  Handles all RYLR896 LoRa module communication.
  Based on proven working implementation.

  Features:
  - Reliable AT command interface
  - Automatic initialization with optimal settings
  - Message send/receive with error handling
  - RSSI and SNR monitoring

  Connection:
  - RYLR896 TX -> ESP32 GPIO25 (RXD2)
  - RYLR896 RX -> ESP32 GPIO26 (TXD2)
  - Baudrate: 115200

  LoRa Parameters:
  - Spreading Factor: 12 (maximum range)
  - Bandwidth: 125kHz
  - Coding Rate: 4/5
  - Preamble: 4
=======================================================================*/

#ifndef LORA_HANDLER_H
#define LORA_HANDLER_H

#include <HardwareSerial.h>
#include "config.h"
#include "structs.h"

// Use Serial1 explicitly for better reliability
HardwareSerial LoRaSerial(1);

// =============== AT COMMAND FUNCTION ================================
inline String sendLoRaCommand(String command, int timeout = 500) {
  // Clear any pending data before sending command
  while (LoRaSerial.available()) {
    LoRaSerial.read();
  }

  LoRaSerial.println(command);

  Serial.print("[LoRa TX] ");
  Serial.println(command);

  unsigned long start = millis();
  String response = "";
  int bytesRead = 0;

  while (millis() - start < timeout) {
    if (LoRaSerial.available()) {
      char c = LoRaSerial.read();
      response += c;
      bytesRead++;

      // Show raw bytes for debugging garbled data
      #if DEBUG_LORA_AT
      if (c < 32 || c > 126) {  // Non-printable character
        Serial.print("<0x");
        Serial.print(c, HEX);
        Serial.print(">");
      }
      #endif

      if (c == '\n' && response.length() > 2) break;
    }
  }

  response.trim();

  if (response.length() > 0) {
    Serial.print("[LoRa RX] ");
    Serial.println(response);
  } else {
    Serial.println("[LoRa RX] <no response>");
  }

  #if DEBUG_LORA_AT
  Serial.print("  → Bytes received: ");
  Serial.print(bytesRead);
  Serial.print(", Elapsed: ");
  Serial.print(millis() - start);
  Serial.println("ms");
  #endif

  return response;
}

// Helper: Clear serial buffer and wait for READY signal
inline void waitForReady(int timeout = 5000) {
  Serial.println("Waiting for +READY signal...");
  unsigned long start = millis();
  String buffer = "";

  while (millis() - start < timeout) {
    if (LoRaSerial.available()) {
      char c = LoRaSerial.read();
      buffer += c;

      // Check if we got READY signal
      if (buffer.indexOf("READY") >= 0) {
        Serial.println("✓ Module ready!");
        delay(100);
        // Clear any remaining data
        while (LoRaSerial.available()) LoRaSerial.read();
        return;
      }

      // Keep buffer size manageable
      if (buffer.length() > 50) {
        buffer = buffer.substring(buffer.length() - 30);
      }
    }
  }
  Serial.println("⚠ READY signal timeout (continuing anyway)");
}

// =============== INITIALIZE LoRa ================================
inline bool initLoRa(uint8_t myAddress, uint8_t networkID) {
  Serial.println("\n============================");
  Serial.println("=== RYLR896 Init ===");
  Serial.println("============================");

  #if DEBUG_LORA_AT
  Serial.println("📡 LoRa Debug Mode: ENABLED");
  Serial.print("  Baudrate: ");
  Serial.println(LORA_BAUDRATE);
  Serial.print("  RX Pin (RYLR896 TX): GPIO ");
  Serial.println(RXD2);
  Serial.print("  TX Pin (RYLR896 RX): GPIO ");
  Serial.println(TXD2);
  #endif

  // Start serial connection
  LoRaSerial.begin(LORA_BAUDRATE, SERIAL_8N1, RXD2, TXD2);
  delay(1000);  // Increased from 500ms to 1000ms

  Serial.println("📝 Clearing serial buffer...");
  int cleared = 0;
  while (LoRaSerial.available()) {
    LoRaSerial.read();
    cleared++;
  }
  #if DEBUG_LORA_AT
  Serial.print("  → Cleared ");
  Serial.print(cleared);
  Serial.println(" bytes");
  #endif

  // CRITICAL: Reset module first!
  Serial.println("🔄 Resetting module...");
  sendLoRaCommand("AT+RESET", 2000);  // Increased timeout to 2000ms

  // Wait for READY signal
  waitForReady(5000);  // Increased from 3000ms to 5000ms

  // Test communication with multiple retries
  Serial.println("🔍 Testing connection...");
  String response = "";
  int attempts = 0;
  bool connected = false;

  for (attempts = 1; attempts <= 3; attempts++) {
    Serial.print("  Attempt ");
    Serial.print(attempts);
    Serial.print("/3... ");

    response = sendLoRaCommand("AT", 1500);  // Increased timeout to 1500ms

    // Check for +OK (note the plus sign!)
    if (response.indexOf("+OK") >= 0 || response.indexOf("OK") >= 0) {
      Serial.println("✓ Success!");
      connected = true;
      break;
    } else {
      Serial.println("✗ Failed");
      Serial.print("    Got: '");
      Serial.print(response);
      Serial.println("'");
      if (attempts < 3) {
        Serial.println("    Waiting 1 second before retry...");
        delay(1000);
      }
    }
  }

  if (!connected) {
    Serial.println("❌ Module not responding after 3 attempts!");
    Serial.println("💡 Troubleshooting:");
    Serial.println("   1. Check RYLR896 power (3.3V, NOT 5V!)");
    Serial.println("   2. Verify TX/RX connections:");
    Serial.print("      - RYLR896 TX → ESP32 GPIO ");
    Serial.println(RXD2);
    Serial.print("      - RYLR896 RX → ESP32 GPIO ");
    Serial.println(TXD2);
    Serial.println("   3. Check common GND");
    Serial.println("   4. Try different baudrate (9600/57600/115200)");
    return false;
  }
  Serial.println("✓ Module responding");

  // Get version
  response = sendLoRaCommand("AT+VER?", 1000);

  // Set address
  Serial.print("Setting address to ");
  Serial.print(myAddress);
  Serial.println("...");
  response = sendLoRaCommand("AT+ADDRESS=" + String(myAddress), 1000);
  if (response.indexOf("OK") < 0) {
    Serial.println("❌ Address failed!");
    return false;
  }
  Serial.println("✓ Address set");

  // Set network ID
  Serial.print("Setting network ID to ");
  Serial.print(networkID);
  Serial.println("...");
  response = sendLoRaCommand("AT+NETWORKID=" + String(networkID), 1000);
  if (response.indexOf("OK") < 0) {
    Serial.println("❌ Network ID failed!");
    return false;
  }
  Serial.println("✓ Network ID set");

  // Set parameters (SF12 = max range, works reliably)
  Serial.println("Setting parameters...");
  response = sendLoRaCommand("AT+PARAMETER=12,7,1,4", 1000);
  if (response.indexOf("OK") >= 0) {
    Serial.println("✓ Parameters: SF12, BW125kHz");
  }
  
  Serial.println("============================");
  Serial.println("✓ RYLR896 Ready!");
  Serial.println("============================\n");
  
  return true;
}

// =============== SEND MESSAGE ================================
inline bool sendLoRaMessage(String message, uint8_t targetAddress) {
  String command = "AT+SEND=" + String(targetAddress) + "," + 
                   String(message.length()) + "," + message;
  
  String response = sendLoRaCommand(command, 2000);
  
  if (response.indexOf("OK") >= 0) {
    Serial.print("✓ Sent: ");
    Serial.println(message);
    return true;
  } else {
    Serial.println("❌ Send failed!");
    return false;
  }
}

// =============== RECEIVE MESSAGE ================================
inline bool receiveLoRaMessage(DeviceState& remote, String& payload) {
  if (!LoRaSerial.available()) {
    return false;
  }

  // Read with timeout to avoid blocking (readStringUntil has 1s default timeout)
  // Manual read is faster and more responsive
  String response = "";
  unsigned long start = millis();
  while (millis() - start < 100) {  // 100ms timeout
    if (LoRaSerial.available()) {
      char c = LoRaSerial.read();
      if (c == '\n' || c == '\r') {
        if (response.length() > 0) break;  // Got complete line
      } else {
        response += c;
      }
    }
  }
  response.trim();
  
  // Check if it's a received message
  if (response.startsWith("+RCV=")) {
    Serial.println("\n╔════════════════════════");
    Serial.println("║ LoRa Message Received");
    Serial.println("╠════════════════════════");
    Serial.print("║ Raw: ");
    Serial.println(response);

    // Parse: +RCV=sender,length,data,RSSI,SNR
    // NOTE: Data field may contain commas! Use length to parse correctly.
    int start = 5;  // Skip "+RCV="
    int comma1 = response.indexOf(',', start);
    int comma2 = response.indexOf(',', comma1 + 1);

    if (comma1 > 0 && comma2 > 0) {
      String sender = response.substring(start, comma1);
      String lengthStr = response.substring(comma1 + 1, comma2);
      int dataLength = lengthStr.toInt();

      // Extract exact data length (data may contain commas!)
      int dataStart = comma2 + 1;
      payload = response.substring(dataStart, dataStart + dataLength);

      // RSSI and SNR are after the data
      int rssiStart = dataStart + dataLength + 1;  // +1 for comma
      int comma3 = response.indexOf(',', rssiStart);

      if (comma3 > 0) {
        String rssiStr = response.substring(rssiStart, comma3);
        String snrStr = response.substring(comma3 + 1);

        remote.rssi = rssiStr.toInt();
        remote.snr = snrStr.toInt();
        remote.lastMessageTime = millis();  // Update last message timestamp!

        Serial.print("║ From: ");
        Serial.println(sender);
        Serial.print("║ Data: ");
        Serial.println(payload);
        Serial.print("║ RSSI: ");
        Serial.print(remote.rssi);
        Serial.println(" dBm");
        Serial.print("║ SNR:  ");
        Serial.println(remote.snr);
        Serial.println("╚════════════════════════");

        return true;
      }
    }
  }
  
  return false;
}

#endif // LORA_HANDLER_H