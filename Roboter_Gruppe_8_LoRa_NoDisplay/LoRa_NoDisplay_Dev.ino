/*=====================================================================
  LoRa_NoDisplay_Dev.ino

  Development version without LCD display for single-device testing.

  Features:
  - Auto role detection (GPIO 16↔17 jumper)
  - Kill-switch (GPIO 12↔14, long press = restart)
  - Connection watchdog & health monitoring
  - Structured serial output for PC data logging
  - Works as both sender and receiver

  Hardware Setup:
  - RYLR896 TX -> ESP32 GPIO25 (RX)
  - RYLR896 RX -> ESP32 GPIO26 (TX)

  Role Detection:
  - Receiver: GPIO16 connected to GPIO17 (jumper wire)
  - Sender:   GPIO16 floating (no connection)

  Kill-Switch:
  - GPIO14 = GND, GPIO12 = INPUT_PULLUP
  - Connect GPIO12↔GPIO14 and hold 3s = restart device

  Serial Output:
  - Structured CSV/JSON format for PC reading
  - Human-readable status updates
  - Health monitoring reports

=======================================================================*/

#include <WiFi.h>

#include "config.h"
#include "structs.h"
#include "lora_handler.h"
#include "health_monitor.h"

// =============== KILL-SWITCH CONFIG ================================
#define KILLSWITCH_GND_PIN 14    // Provides GND
#define KILLSWITCH_READ_PIN 12   // Read with pull-up
#define KILLSWITCH_HOLD_TIME 3000  // 3 seconds to restart

// =============== GLOBALS ================================
bool bRECEIVER = 0;
uint8_t MY_LORA_ADDRESS = 0;
uint8_t TARGET_LORA_ADDRESS = 0;

DeviceState local;
DeviceState remote;
TimingData timing;
SpinnerData spinner;
HealthMonitor health;

// Kill-switch state
unsigned long killSwitchPressStart = 0;
bool killSwitchActive = false;

// =============== KILL-SWITCH FUNCTIONS ================================

void initKillSwitch() {
  pinMode(KILLSWITCH_GND_PIN, OUTPUT);
  digitalWrite(KILLSWITCH_GND_PIN, LOW);  // Provide GND
  pinMode(KILLSWITCH_READ_PIN, INPUT_PULLUP);

  Serial.println("✓ Kill-switch initialized (GPIO12↔14, hold 3s to restart)");
}

void checkKillSwitch() {
  bool pressed = (digitalRead(KILLSWITCH_READ_PIN) == LOW);

  if (pressed) {
    if (killSwitchPressStart == 0) {
      killSwitchPressStart = millis();
      Serial.println("🔴 Kill-switch pressed...");
    }

    unsigned long pressDuration = millis() - killSwitchPressStart;

    // Show countdown
    if (pressDuration >= 1000 && pressDuration < 2000) {
      static bool printed1s = false;
      if (!printed1s) {
        Serial.println("🔴 Hold for 2 more seconds to restart...");
        printed1s = true;
      }
    } else if (pressDuration >= 2000 && pressDuration < 3000) {
      static bool printed2s = false;
      if (!printed2s) {
        Serial.println("🔴 Hold for 1 more second to restart...");
        printed2s = true;
      }
    } else if (pressDuration >= KILLSWITCH_HOLD_TIME) {
      Serial.println("\n╔════════════════════════════════╗");
      Serial.println("║  🔴 KILL SWITCH ACTIVATED 🔴  ║");
      Serial.println("║     RESTARTING DEVICE...      ║");
      Serial.println("╚════════════════════════════════╝\n");
      delay(500);
      ESP.restart();
    }
  } else {
    // Released before restart
    if (killSwitchPressStart > 0) {
      Serial.println("✓ Kill-switch released");
    }
    killSwitchPressStart = 0;
  }
}

// =============== DATA OUTPUT FUNCTIONS ================================

// Print structured CSV data for PC logging
void printDataCSV() {
  // Format: TIMESTAMP,ROLE,RSSI,SNR,SEQ,MSG_COUNT,CONN_STATE,PACKET_LOSS,LED,TOUCH
  Serial.print("DATA_CSV,");
  Serial.print(millis());
  Serial.print(",");
  Serial.print(bRECEIVER ? "RX" : "TX");
  Serial.print(",");
  Serial.print(remote.rssi);
  Serial.print(",");
  Serial.print(remote.snr);
  Serial.print(",");
  Serial.print(remote.sequenceNumber);
  Serial.print(",");
  Serial.print(bRECEIVER ? remote.messageCount : local.messageCount);
  Serial.print(",");
  Serial.print(getConnectionStateString(health.state));
  Serial.print(",");
  Serial.print(getPacketLoss(health), 2);
  Serial.print(",");
  Serial.print(local.ledState);
  Serial.print(",");
  Serial.println(local.touchState);
}

// Print JSON data for PC logging
void printDataJSON() {
  Serial.print("{\"type\":\"data\",\"ts\":");
  Serial.print(millis());
  Serial.print(",\"role\":\"");
  Serial.print(bRECEIVER ? "RX" : "TX");
  Serial.print("\",\"rssi\":");
  Serial.print(remote.rssi);
  Serial.print(",\"snr\":");
  Serial.print(remote.snr);
  Serial.print(",\"seq\":");
  Serial.print(remote.sequenceNumber);
  Serial.print(",\"msgCount\":");
  Serial.print(bRECEIVER ? remote.messageCount : local.messageCount);
  Serial.print(",\"state\":\"");
  Serial.print(getConnectionStateString(health.state));
  Serial.print("\",\"loss\":");
  Serial.print(getPacketLoss(health), 2);
  Serial.print(",\"led\":");
  Serial.print(local.ledState);
  Serial.print(",\"touch\":");
  Serial.print(local.touchState);
  Serial.println("}");
}

// Print compact status update
void printStatus() {
  Serial.println("\n════════════════════════════════");
  Serial.print("Role: ");
  Serial.print(bRECEIVER ? "RECEIVER" : "SENDER");
  Serial.print(" | State: ");
  Serial.print(getConnectionStateString(health.state));
  Serial.print(" ");
  Serial.println(getConnectionIcon(health.state));

  if (bRECEIVER) {
    Serial.print("Messages RX: ");
    Serial.print(remote.messageCount);
    Serial.print(" | RSSI: ");
    Serial.print(remote.rssi);
    Serial.print(" dBm | SNR: ");
    Serial.println(remote.snr);
    Serial.print("Packet Loss: ");
    Serial.print(getPacketLoss(health), 1);
    Serial.println("%");
  } else {
    Serial.print("Messages TX: ");
    Serial.println(local.messageCount);
  }

  Serial.print("Local LED: ");
  Serial.print(local.ledState);
  Serial.print(" | Touch: ");
  Serial.println(local.touchState);
  Serial.println("════════════════════════════════\n");
}

// =============== PARSE PAYLOAD ================================

void parsePayload(String payload) {
  // Parse: SEQ:x,LED:x,TOUCH:x,SPIN:x,COUNT:x
  int seqIdx = payload.indexOf("SEQ:");
  int ledIdx = payload.indexOf("LED:");
  int touchIdx = payload.indexOf("TOUCH:");
  int spinIdx = payload.indexOf("SPIN:");

  if (seqIdx >= 0) {
    int comma = payload.indexOf(',', seqIdx);
    if (comma < 0) comma = payload.length();
    remote.sequenceNumber = payload.substring(seqIdx + 4, comma).toInt();
  }

  if (ledIdx >= 0) {
    int comma = payload.indexOf(',', ledIdx);
    if (comma < 0) comma = payload.length();
    remote.ledState = payload.substring(ledIdx + 4, comma).toInt();
  }

  if (touchIdx >= 0) {
    int comma = payload.indexOf(',', touchIdx);
    if (comma < 0) comma = payload.length();
    remote.touchState = payload.substring(touchIdx + 6, comma).toInt();
  }

  if (spinIdx >= 0) {
    int comma = payload.indexOf(',', spinIdx);
    if (comma < 0) comma = payload.length();
    remote.spinnerIndex = payload.substring(spinIdx + 5, comma).toInt();
  }
}

// =============== SETUP ================================

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n\n");
  Serial.println("╔════════════════════════════════╗");
  Serial.println("║  LoRa NoDisplay Dev Version   ║");
  Serial.println("║  With Kill-Switch & PC Link   ║");
  Serial.println("╚════════════════════════════════╝");

  // Initialize kill-switch
  initKillSwitch();

  // Auto-detect role
  pinMode(MODE_GND_PIN, OUTPUT);
  digitalWrite(MODE_GND_PIN, LOW);
  pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
  delay(100);

  bRECEIVER = (digitalRead(MODE_SELECT_PIN) == LOW);

  Serial.println("\n╔════════════════════════════╗");
  Serial.println("║   MODE DETECTION          ║");
  Serial.println("╠════════════════════════════╣");
  Serial.print("║ GPIO ");
  Serial.print(MODE_SELECT_PIN);
  Serial.print(": ");
  Serial.println(digitalRead(MODE_SELECT_PIN) == LOW ? "LOW" : "HIGH");
  Serial.println("╚════════════════════════════╝");

  if (bRECEIVER) {
    Serial.println("\n>>> RECEIVER MODE");
    MY_LORA_ADDRESS = LORA_RECEIVER_ADDRESS;
    TARGET_LORA_ADDRESS = LORA_SENDER_ADDRESS;
  } else {
    Serial.println("\n>>> SENDER MODE");
    MY_LORA_ADDRESS = LORA_SENDER_ADDRESS;
    TARGET_LORA_ADDRESS = LORA_RECEIVER_ADDRESS;
  }

  Serial.println("\n╔════════════════════════════╗");
  Serial.println("║   LoRa CONFIGURATION      ║");
  Serial.println("╠════════════════════════════╣");
  Serial.print("║ My Address:     ");
  Serial.println(MY_LORA_ADDRESS);
  Serial.print("║ Target Address: ");
  Serial.println(TARGET_LORA_ADDRESS);
  Serial.print("║ Network ID:     ");
  Serial.println(LORA_NETWORK_ID);
  Serial.println("╚════════════════════════════╝");

  // Initialize pins
  pinMode(LED_PIN, OUTPUT);

  // Initialize structs
  local = {LOW, 0, false, 0, 0, 0, 0, 0, 0};
  remote = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  timing = {0, 0, 0, 0, 0, 0, 0};
  spinner = {{'<', '^', '>', 'v'}, 0, 0};

  // Initialize LoRa
  if (!initLoRa(MY_LORA_ADDRESS, LORA_NETWORK_ID)) {
    Serial.println("\n❌ LoRa init failed!");
    while(1) delay(1000);
  }

  // Initialize health monitor for receiver
  if (bRECEIVER) {
    remote.lastMessageTime = millis();
    initHealthMonitor(health);
  }

  Serial.println("\n✓ Setup complete!");
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║  Serial Output Formats Available:    ║");
  Serial.println("║  - DATA_CSV: CSV format for logging  ║");
  Serial.println("║  - JSON: JSON format for parsing     ║");
  Serial.println("║  - Status updates every 5s           ║");
  Serial.println("║  - Health reports every 30s          ║");
  Serial.println("╚════════════════════════════════════════╝\n");
}

// =============== LOOP ================================

void loop() {
  // Check kill-switch
  checkKillSwitch();

  // LED blink
  if (millis() - timing.lastLED >= 500) {
    timing.lastLED = millis();
    local.ledState = !local.ledState;
    digitalWrite(LED_PIN, local.ledState);
    local.ledCount++;
    if (local.ledCount >= 80) local.ledCount = 0;
  }

  // Touch sensor
  if (millis() - timing.lastSensor >= 200) {
    timing.lastSensor = millis();
    local.touchValue = touchRead(TOUCH_PIN);
    local.touchState = (local.touchValue <= 500);
  }

  // Role-specific
  if (bRECEIVER) {
    // RECEIVER: Listen
    String payload;
    if (receiveLoRaMessage(remote, payload)) {
      parsePayload(payload);
      remote.messageCount++;

      // Update health monitoring
      updateRSSI(health, remote.rssi);
      trackPacket(health, remote.sequenceNumber);

      // Print data in CSV format (easy for Python to parse)
      printDataCSV();

      // Also print JSON (optional, comment out if not needed)
      // printDataJSON();
    }

    // Update connection state (watchdog)
    updateConnectionState(health, remote);

    // Attempt recovery if connection lost
    if (health.state == CONN_LOST) {
      attemptRecovery(health, MY_LORA_ADDRESS, LORA_NETWORK_ID);
    }

    // Regular status update
    if (millis() - timing.lastCheck >= 5000) {
      timing.lastCheck = millis();
      printStatus();
    }

    // Health report every 30 seconds
    if (millis() - timing.lastHealthReport >= 30000) {
      timing.lastHealthReport = millis();
      printHealthReport(health, remote);
    }

  } else {
    // SENDER: Send every 2 seconds
    if (millis() - timing.lastSend >= 2000) {
      timing.lastSend = millis();

      String payload = "SEQ:" + String(local.sequenceNumber) +
                       ",LED:" + String(local.ledState) +
                       ",TOUCH:" + String(local.touchState) +
                       ",SPIN:" + String(local.spinnerIndex) +
                       ",COUNT:" + String(local.messageCount);

      if (sendLoRaMessage(payload, TARGET_LORA_ADDRESS)) {
        local.messageCount++;
        local.sequenceNumber++;

        // Print data
        printDataCSV();
      }
    }

    // Regular status update
    if (millis() - timing.lastCheck >= 5000) {
      timing.lastCheck = millis();
      printStatus();
    }
  }

  delay(10);
}
