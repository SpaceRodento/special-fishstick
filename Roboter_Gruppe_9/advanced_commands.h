/*=====================================================================
  advanced_commands.h - Extended Remote Commands

  FEATURE 4: Advanced Remote Commands

  Extended command set for remote device control and diagnostics.
  Commands can be sent via LoRa from remote device.

  Available Commands:
  - CMD:STATUS          - Request full status report
  - CMD:RESET_STATS     - Reset packet counters
  - CMD:RESTART         - Reboot device (existing)
  - CMD:SET_POWER:X     - Set TX power to X dBm
  - CMD:SET_SF:X        - Set spreading factor to X
  - CMD:SET_INTERVAL:X  - Set send interval to X ms
  - CMD:LED_ON          - Turn LED on
  - CMD:LED_OFF         - Turn LED off
  - CMD:LED_BLINK:X     - Blink LED X times
  - CMD:PING            - Simple ping test (responds with PONG)
  - CMD:GET_RSSI        - Request RSSI from remote
  - CMD:GET_BATTERY     - Request battery voltage

  Response Format:
  Commands that request data respond with ACK:<data>

  Testing:
  1. Set ENABLE_ADVANCED_COMMANDS true in config.h
  2. Upload to both devices
  3. Send commands from one device
  4. Check serial output on both devices

  Example Usage:
  Sender Serial: CONFIG:SEND:CMD:STATUS
  Receiver responds with full status over LoRa
=======================================================================*/

#ifndef ADVANCED_COMMANDS_H
#define ADVANCED_COMMANDS_H

#include <Arduino.h>
#include "config.h"

// Command statistics
struct CommandStats {
  int commandsReceived;
  int commandsExecuted;
  int commandsRejected;
  unsigned long lastCommandTime;
  String lastCommand;
};

CommandStats cmdStats = {0, 0, 0, 0, ""};

// Initialize advanced commands
void initAdvancedCommands() {
  #if ENABLE_ADVANCED_COMMANDS
    Serial.println("✓ Advanced commands enabled");
    Serial.println("  Available commands:");
    Serial.println("    CMD:STATUS, CMD:RESET_STATS, CMD:PING");
    Serial.println("    CMD:SET_POWER:X, CMD:SET_SF:X");
    Serial.println("    CMD:LED_ON, CMD:LED_OFF, CMD:LED_BLINK:X");
    Serial.println("    CMD:GET_RSSI, CMD:GET_BATTERY");
  #endif
}

// Build status report string
String buildStatusReport() {
  #if ENABLE_ADVANCED_COMMANDS
    extern DeviceState local;
    extern DeviceState remote;
    extern HealthMonitor health;

    String status = "STATUS,";
    status += "UPTIME:" + String(millis() / 1000) + "s,";
    status += "HEAP:" + String(ESP.getFreeHeap() / 1024) + "KB,";
    status += "RSSI:" + String(remote.rssi) + ",";
    status += "SNR:" + String(remote.snr) + ",";
    status += "LOSS:" + String(getPacketLoss(health), 2) + "%,";
    status += "STATE:" + getConnectionStateString(health.state) + ",";
    status += "TX:" + String(local.messageCount) + ",";
    status += "RX:" + String(remote.messageCount);

    #if ENABLE_BATTERY_MONITOR
      extern BatteryStatus battery;
      status += ",BATT:" + String(battery.voltage, 2) + "V";
    #endif

    return status;
  #else
    return "";
  #endif
}

// LED blink function
void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

// Process advanced remote command
void processAdvancedCommand(String payload) {
  #if ENABLE_ADVANCED_COMMANDS
    // Check if this is a command
    if (payload.indexOf("CMD:") < 0) {
      return;  // Not a command
    }

    cmdStats.commandsReceived++;
    cmdStats.lastCommandTime = millis();

    // Extract command (everything after CMD:)
    int cmdStart = payload.indexOf("CMD:");
    String command = payload.substring(cmdStart + 4);

    // Remove trailing data
    int commaPos = command.indexOf(',');
    if (commaPos >= 0) {
      command = command.substring(0, commaPos);
    }
    command.trim();

    cmdStats.lastCommand = command;

    Serial.print("📡 Remote command received: ");
    Serial.println(command);

    // STATUS - Send full status report
    if (command == "STATUS") {
      String status = buildStatusReport();
      Serial.println("→ Sending status report");
      // Send via LoRa (function must be implemented in main code)
      extern void sendLoRaMessage(String payload, int address);
      sendLoRaMessage(status, LORA_SENDER_ADDRESS);
      cmdStats.commandsExecuted++;
    }

    // RESET_STATS - Reset counters
    else if (command == "RESET_STATS") {
      extern HealthMonitor health;
      health.packetsReceived = 0;
      health.packetsLost = 0;
      health.rssiSum = 0;
      health.rssiCount = 0;
      Serial.println("✓ Statistics reset");
      cmdStats.commandsExecuted++;
    }

    // PING - Simple connectivity test
    else if (command == "PING") {
      Serial.println("→ Responding to PING with PONG");
      extern void sendLoRaMessage(String payload, int address);
      sendLoRaMessage("PONG", LORA_SENDER_ADDRESS);
      cmdStats.commandsExecuted++;
    }

    // SET_POWER:X - Set TX power
    else if (command.startsWith("SET_POWER:")) {
      int power = command.substring(10).toInt();
      if (power >= 0 && power <= 20) {
        extern HardwareSerial LoRaSerial;
        String cmd = "AT+CRFOP=" + String(power);
        LoRaSerial.println(cmd);
        delay(50);
        Serial.print("✓ TX power set to ");
        Serial.print(power);
        Serial.println(" dBm");
        cmdStats.commandsExecuted++;
      } else {
        Serial.println("❌ Invalid power (0-20 dBm)");
        cmdStats.commandsRejected++;
      }
    }

    // SET_SF:X - Set spreading factor
    else if (command.startsWith("SET_SF:")) {
      int sf = command.substring(7).toInt();
      if (sf >= 7 && sf <= 12) {
        extern HardwareSerial LoRaSerial;
        String cmd = "AT+PARAMETER=" + String(sf) + ",7,1,4";
        LoRaSerial.println(cmd);
        delay(50);
        Serial.print("✓ Spreading factor set to SF");
        Serial.println(sf);
        cmdStats.commandsExecuted++;
      } else {
        Serial.println("❌ Invalid SF (7-12)");
        cmdStats.commandsRejected++;
      }
    }

    // SET_INTERVAL:X - Set send interval
    else if (command.startsWith("SET_INTERVAL:")) {
      int interval = command.substring(13).toInt();
      if (interval >= 100 && interval <= 60000) {
        #if ENABLE_RUNTIME_CONFIG
          extern RuntimeConfig runtimeCfg;
          runtimeCfg.sendInterval = interval;
        #endif
        Serial.print("✓ Send interval set to ");
        Serial.print(interval);
        Serial.println(" ms");
        cmdStats.commandsExecuted++;
      } else {
        Serial.println("❌ Invalid interval (100-60000 ms)");
        cmdStats.commandsRejected++;
      }
    }

    // LED_ON - Turn LED on
    else if (command == "LED_ON") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("✓ LED turned ON");
      cmdStats.commandsExecuted++;
    }

    // LED_OFF - Turn LED off
    else if (command == "LED_OFF") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("✓ LED turned OFF");
      cmdStats.commandsExecuted++;
    }

    // LED_BLINK:X - Blink LED X times
    else if (command.startsWith("LED_BLINK:")) {
      int times = command.substring(10).toInt();
      if (times >= 1 && times <= 20) {
        Serial.print("✓ Blinking LED ");
        Serial.print(times);
        Serial.println(" times");
        blinkLED(times);
        cmdStats.commandsExecuted++;
      } else {
        Serial.println("❌ Invalid blink count (1-20)");
        cmdStats.commandsRejected++;
      }
    }

    // GET_RSSI - Request RSSI report
    else if (command == "GET_RSSI") {
      extern DeviceState remote;
      String response = "RSSI:" + String(remote.rssi) + ",SNR:" + String(remote.snr);
      Serial.println("→ Sending RSSI report");
      extern void sendLoRaMessage(String payload, int address);
      sendLoRaMessage(response, LORA_SENDER_ADDRESS);
      cmdStats.commandsExecuted++;
    }

    // GET_BATTERY - Request battery voltage
    else if (command == "GET_BATTERY") {
      #if ENABLE_BATTERY_MONITOR
        extern BatteryStatus battery;
        String response = "BATTERY:" + String(battery.voltage, 2) + "V";
        Serial.println("→ Sending battery report");
        extern void sendLoRaMessage(String payload, int address);
        sendLoRaMessage(response, LORA_SENDER_ADDRESS);
        cmdStats.commandsExecuted++;
      #else
        Serial.println("⚠ Battery monitoring disabled");
        cmdStats.commandsRejected++;
      #endif
    }

    // RESTART - Reboot device (already implemented in main code)
    else if (command == "RESTART") {
      Serial.println("🔴 RESTART command - handled by main code");
      // Don't increment counter here, main code handles it
    }

    // Unknown command
    else {
      Serial.print("❌ Unknown command: ");
      Serial.println(command);
      cmdStats.commandsRejected++;
    }

  #endif
}

// Print command statistics
void printCommandStats() {
  #if ENABLE_ADVANCED_COMMANDS
    Serial.println("\n╔═══════ COMMAND STATISTICS ═══════╗");
    Serial.print("║ Total received:  ");
    Serial.println(cmdStats.commandsReceived);
    Serial.print("║ Executed:        ");
    Serial.println(cmdStats.commandsExecuted);
    Serial.print("║ Rejected:        ");
    Serial.println(cmdStats.commandsRejected);
    Serial.print("║ Last command:    ");
    Serial.println(cmdStats.lastCommand);
    Serial.print("║ Last time:       ");
    Serial.print((millis() - cmdStats.lastCommandTime) / 1000);
    Serial.println(" s ago");
    Serial.println("╚══════════════════════════════════╝\n");
  #endif
}

#endif // ADVANCED_COMMANDS_H
