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
  LoRaSerial.println(command);

  Serial.print("[LoRa TX] ");
  Serial.println(command);

  unsigned long start = millis();
  String response = "";

  while (millis() - start < timeout) {
    if (LoRaSerial.available()) {
      char c = LoRaSerial.read();
      response += c;
      if (c == '\n' && response.length() > 2) break;
    }
  }

  response.trim();

  if (response.length() > 0) {
    Serial.print("[LoRa RX] ");
    Serial.println(response);
  }

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

  // Start serial connection
  LoRaSerial.begin(LORA_BAUDRATE, SERIAL_8N1, RXD2, TXD2);
  delay(500);

  // Clear any old data
  while (LoRaSerial.available()) LoRaSerial.read();

  // CRITICAL: Reset module first!
  Serial.println("Resetting module...");
  sendLoRaCommand("AT+RESET", 1000);

  // Wait for READY signal
  waitForReady(3000);

  // Test communication
  Serial.println("Testing connection...");
  String response = sendLoRaCommand("AT", 1000);

  // Check for +OK (note the plus sign!)
  if (response.indexOf("+OK") < 0 && response.indexOf("OK") < 0) {
    Serial.println("❌ No response from module!");
    Serial.print("Got: '");
    Serial.print(response);
    Serial.println("'");

    // Try one more time
    Serial.println("Retrying...");
    delay(500);
    response = sendLoRaCommand("AT", 1000);
    if (response.indexOf("OK") < 0) {
      return false;
    }
  }
  Serial.println("✓ Connected");

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