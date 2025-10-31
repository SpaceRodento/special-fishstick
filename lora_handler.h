/*=====================================================================
  lora_handler.h - RYLR896 LoRa Handler (Working Version)
  
  Based on proven working code
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

// =============== INITIALIZE LoRa ================================
inline bool initLoRa(uint8_t myAddress, uint8_t networkID) {
  Serial.println("\n============================");
  Serial.println("=== RYLR896 Init ===");
  Serial.println("============================");
  
  // Start serial connection
  LoRaSerial.begin(LORA_BAUDRATE, SERIAL_8N1, RXD2, TXD2);
  delay(1000);
  
  // CRITICAL: Reset module first!
  Serial.println("Resetting module...");
  sendLoRaCommand("AT+RESET");
  delay(2000);  // Wait for reset
  
  // Test communication
  Serial.println("Testing connection...");
  String response = sendLoRaCommand("AT");
  if (response.indexOf("OK") < 0) {
    Serial.println("❌ No response!");
    return false;
  }
  Serial.println("✓ Connected");
  
  // Get version
  response = sendLoRaCommand("AT+VER?");
  
  // Set address
  Serial.print("Setting address to ");
  Serial.print(myAddress);
  Serial.println("...");
  response = sendLoRaCommand("AT+ADDRESS=" + String(myAddress));
  if (response.indexOf("OK") < 0) {
    Serial.println("❌ Address failed!");
    return false;
  }
  Serial.println("✓ Address set");
  
  // Set network ID
  Serial.print("Setting network ID to ");
  Serial.print(networkID);
  Serial.println("...");
  response = sendLoRaCommand("AT+NETWORKID=" + String(networkID));
  if (response.indexOf("OK") < 0) {
    Serial.println("❌ Network ID failed!");
    return false;
  }
  Serial.println("✓ Network ID set");
  
  // Set parameters (SF12 = max range, works reliably)
  Serial.println("Setting parameters...");
  response = sendLoRaCommand("AT+PARAMETER=12,7,1,4");
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
  
  String response = LoRaSerial.readStringUntil('\n');
  response.trim();
  
  // Check if it's a received message
  if (response.startsWith("+RCV=")) {
    Serial.println("\n╔════════════════════════");
    Serial.println("║ LoRa Message Received");
    Serial.println("╠════════════════════════");
    Serial.print("║ Raw: ");
    Serial.println(response);
    
    // Parse: +RCV=sender,length,data,RSSI,SNR
    int start = 5;  // Skip "+RCV="
    int comma1 = response.indexOf(',', start);
    int comma2 = response.indexOf(',', comma1 + 1);
    int comma3 = response.indexOf(',', comma2 + 1);
    int comma4 = response.indexOf(',', comma3 + 1);
    
    if (comma1 > 0 && comma2 > 0 && comma3 > 0) {
      String sender = response.substring(start, comma1);
      String length = response.substring(comma1 + 1, comma2);
      payload = response.substring(comma2 + 1, comma3);
      String rssiStr = response.substring(comma3 + 1, comma4);
      String snrStr = response.substring(comma4 + 1);
      
      remote.rssi = rssiStr.toInt();
      remote.snr = snrStr.toInt();
      
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
  
  return false;
}

#endif // LORA_HANDLER_H