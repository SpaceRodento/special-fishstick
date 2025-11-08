/*=====================================================================
  runtime_config.h - Runtime Configuration via Serial

  FEATURE 2: Runtime Configuration

  Allows changing configuration on-the-fly without re-uploading code.
  Useful for field testing and parameter optimization.

  Commands (send via Serial Monitor):
  - CONFIG:INTERVAL:1000   - Set send interval to 1000ms
  - CONFIG:SF:10           - Set spreading factor to 10
  - CONFIG:POWER:15        - Set TX power to 15 dBm
  - CONFIG:ACK:10          - Set ACK interval to 10 messages
  - CONFIG:SHOW            - Show current configuration
  - CONFIG:RESET           - Reset to defaults

  Testing:
  1. Set ENABLE_RUNTIME_CONFIG true in config.h
  2. Upload code
  3. Open Serial Monitor (115200 baud)
  4. Type: CONFIG:SHOW
  5. Try changing settings: CONFIG:INTERVAL:1000

  Notes:
  - Settings are not saved to EEPROM (reset on reboot)
  - Some changes require restart to take effect
  - Invalid values are rejected with error message
=======================================================================*/

#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include <Arduino.h>
#include "config.h"

// Runtime configuration values (can be changed on-the-fly)
struct RuntimeConfig {
  unsigned long sendInterval;
  int spreadingFactor;
  int txPower;
  int ackInterval;
  int listenTimeout;
  unsigned long dataOutputInterval;
  bool csvOutput;
  bool bidirectional;
};

// Initialize with defaults from config.h
RuntimeConfig runtimeCfg = {
  2000,  // sendInterval (default 2s)
  12,    // spreadingFactor (SF12)
  15,    // txPower (15 dBm)
  5,     // ackInterval
  500,   // listenTimeout
  2000,  // dataOutputInterval
  true,  // csvOutput
  true   // bidirectional
};

// Initialize runtime configuration system
void initRuntimeConfig() {
  #if ENABLE_RUNTIME_CONFIG
    Serial.println("✓ Runtime configuration enabled");
    Serial.println("  Commands:");
    Serial.println("    CONFIG:SHOW           - Show current settings");
    Serial.println("    CONFIG:INTERVAL:ms    - Set send interval");
    Serial.println("    CONFIG:SF:7-12        - Set spreading factor");
    Serial.println("    CONFIG:POWER:dBm      - Set TX power");
    Serial.println("    CONFIG:ACK:n          - Set ACK interval");
    Serial.println("    CONFIG:RESET          - Reset to defaults");
  #endif
}

// Show current configuration
void showConfiguration() {
  #if ENABLE_RUNTIME_CONFIG
    Serial.println("\n╔═══════════ CURRENT CONFIGURATION ═══════════╗");
    Serial.print("║ Send Interval:     ");
    Serial.print(runtimeCfg.sendInterval);
    Serial.println(" ms");
    Serial.print("║ Spreading Factor:  SF");
    Serial.println(runtimeCfg.spreadingFactor);
    Serial.print("║ TX Power:          ");
    Serial.print(runtimeCfg.txPower);
    Serial.println(" dBm");
    Serial.print("║ ACK Interval:      ");
    Serial.println(runtimeCfg.ackInterval);
    Serial.print("║ Listen Timeout:    ");
    Serial.print(runtimeCfg.listenTimeout);
    Serial.println(" ms");
    Serial.print("║ Data Output:       ");
    Serial.print(runtimeCfg.dataOutputInterval);
    Serial.println(" ms");
    Serial.print("║ CSV Output:        ");
    Serial.println(runtimeCfg.csvOutput ? "ON" : "OFF");
    Serial.print("║ Bi-directional:    ");
    Serial.println(runtimeCfg.bidirectional ? "ON" : "OFF");
    Serial.println("╚═════════════════════════════════════════════╝\n");
  #endif
}

// Apply LoRa parameter changes
bool applyLoRaParameters() {
  #if ENABLE_RUNTIME_CONFIG
    // Set spreading factor, bandwidth, coding rate, preamble
    String cmd = "AT+PARAMETER=" + String(runtimeCfg.spreadingFactor) + ",7,1,4";

    Serial.print("→ Applying: ");
    Serial.println(cmd);

    // Send command (assumes LoRaSerial is available)
    extern HardwareSerial LoRaSerial;
    LoRaSerial.println(cmd);
    delay(100);

    // Check response
    if (LoRaSerial.available()) {
      String response = LoRaSerial.readStringUntil('\n');
      response.trim();
      if (response == "+OK") {
        Serial.println("✓ LoRa parameters updated");
        return true;
      } else {
        Serial.print("❌ LoRa error: ");
        Serial.println(response);
        return false;
      }
    }
    return false;
  #else
    return false;
  #endif
}

// Apply TX power changes
bool applyTxPower() {
  #if ENABLE_RUNTIME_CONFIG
    String cmd = "AT+CRFOP=" + String(runtimeCfg.txPower);

    Serial.print("→ Applying: ");
    Serial.println(cmd);

    extern HardwareSerial LoRaSerial;
    LoRaSerial.println(cmd);
    delay(100);

    if (LoRaSerial.available()) {
      String response = LoRaSerial.readStringUntil('\n');
      response.trim();
      if (response == "+OK") {
        Serial.println("✓ TX power updated");
        return true;
      } else {
        Serial.print("❌ Error: ");
        Serial.println(response);
        return false;
      }
    }
    return false;
  #else
    return false;
  #endif
}

// Process configuration command
void processConfigCommand(String command) {
  #if ENABLE_RUNTIME_CONFIG
    // Remove whitespace
    command.trim();

    // Check for CONFIG: prefix
    if (!command.startsWith(CONFIG_COMMAND_PREFIX)) {
      return;  // Not a config command
    }

    // Remove prefix
    command = command.substring(strlen(CONFIG_COMMAND_PREFIX));

    Serial.print("📝 Config command: ");
    Serial.println(command);

    // SHOW current configuration
    if (command == "SHOW") {
      showConfiguration();
      return;
    }

    // RESET to defaults
    if (command == "RESET") {
      runtimeCfg.sendInterval = 2000;
      runtimeCfg.spreadingFactor = 12;
      runtimeCfg.txPower = 15;
      runtimeCfg.ackInterval = 5;
      runtimeCfg.listenTimeout = 500;
      Serial.println("✓ Configuration reset to defaults");
      showConfiguration();
      return;
    }

    // Parse key:value commands
    int colonPos = command.indexOf(':');
    if (colonPos < 0) {
      Serial.println("❌ Invalid format. Use: CONFIG:KEY:VALUE");
      return;
    }

    String key = command.substring(0, colonPos);
    String value = command.substring(colonPos + 1);
    key.trim();
    value.trim();

    // INTERVAL
    if (key == "INTERVAL") {
      int interval = value.toInt();
      if (interval >= 100 && interval <= 60000) {
        runtimeCfg.sendInterval = interval;
        Serial.print("✓ Send interval set to ");
        Serial.print(interval);
        Serial.println(" ms");
      } else {
        Serial.println("❌ Invalid interval (100-60000 ms)");
      }
    }
    // SPREADING FACTOR
    else if (key == "SF") {
      int sf = value.toInt();
      if (sf >= 7 && sf <= 12) {
        runtimeCfg.spreadingFactor = sf;
        Serial.print("✓ Spreading factor set to SF");
        Serial.println(sf);
        applyLoRaParameters();
      } else {
        Serial.println("❌ Invalid SF (7-12)");
      }
    }
    // TX POWER
    else if (key == "POWER") {
      int power = value.toInt();
      if (power >= 0 && power <= 20) {
        runtimeCfg.txPower = power;
        Serial.print("✓ TX power set to ");
        Serial.print(power);
        Serial.println(" dBm");
        applyTxPower();
      } else {
        Serial.println("❌ Invalid power (0-20 dBm)");
      }
    }
    // ACK INTERVAL
    else if (key == "ACK") {
      int ack = value.toInt();
      if (ack >= 1 && ack <= 100) {
        runtimeCfg.ackInterval = ack;
        Serial.print("✓ ACK interval set to ");
        Serial.println(ack);
      } else {
        Serial.println("❌ Invalid ACK interval (1-100)");
      }
    }
    // CSV OUTPUT
    else if (key == "CSV") {
      if (value == "ON" || value == "1") {
        runtimeCfg.csvOutput = true;
        Serial.println("✓ CSV output enabled");
      } else if (value == "OFF" || value == "0") {
        runtimeCfg.csvOutput = false;
        Serial.println("✓ CSV output disabled");
      } else {
        Serial.println("❌ Use ON/OFF or 1/0");
      }
    }
    // Unknown command
    else {
      Serial.print("❌ Unknown config key: ");
      Serial.println(key);
      Serial.println("   Valid: INTERVAL, SF, POWER, ACK, CSV");
    }
  #endif
}

// Check for incoming configuration commands
void checkConfigCommands() {
  #if ENABLE_RUNTIME_CONFIG
    if (Serial.available()) {
      String command = Serial.readStringUntil('\n');
      command.trim();

      if (command.startsWith(CONFIG_COMMAND_PREFIX)) {
        processConfigCommand(command);
      }
    }
  #endif
}

#endif // RUNTIME_CONFIG_H
