/*=====================================================================
  Roboter_Gruppe_9.ino

  Full-featured version with:
  - Auto role detection (GPIO16↔GPIO17 jumper)
  - Working LoRa communication (RYLR896 module)
  - LCD display on receiver (4 versions)
  - Connection watchdog & health monitoring
  - Kill-switch (GPIO12↔GPIO14, hold 3s = restart)
  - Touch sensor and LED

  Hardware Setup:
  - RYLR896 TX -> ESP32 GPIO25 (RX)
  - RYLR896 RX -> ESP32 GPIO26 (TX)

  Role Detection:
  - Receiver: GPIO16 connected to GPIO17 (jumper wire)
  - Sender:   GPIO16 floating (no connection)
  - Note: GPIO16 and GPIO17 are physically next to each other

  Kill-Switch:
  - GPIO14 = GND, GPIO13 = INPUT_PULLUP
  - Connect GPIO13↔GPIO14 and hold 3s = restart device

  Both devices run IDENTICAL code!
  Role is auto-detected based on jumper wire.
=======================================================================*/

#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <esp_system.h>  // For esp_restart()

#include "config.h"
#include "structs.h"
#include "functions.h"
#include "lora_handler.h"
#include "health_monitor.h"
#include "display_sender.h"  // TFT display station support

// Feature modules
#if ENABLE_BATTERY_MONITOR
  #include "battery_monitor.h"
#endif
#if ENABLE_AUDIO_DETECTION
  #include "audio_detector.h"
#endif
#if ENABLE_LIGHT_DETECTION
  #include "light_detector.h"
#endif
#if ENABLE_CURRENT_MONITOR
  #include "current_monitor.h"
#endif

// =============== KILL-SWITCH CONFIG ================================
// GPIO 12 is a strapping pin on ESP32 - use GPIO 13 instead!
#define KILLSWITCH_GND_PIN 14
#define KILLSWITCH_READ_PIN 13  // Changed from 12 to 13 (safer pin)
#define KILLSWITCH_HOLD_TIME 3000
#define KILLSWITCH_DEBUG true    // Enable debug output

// =============== GLOBALS ================================
bool bRECEIVER = 0;  // Auto-detected
uint8_t MY_LORA_ADDRESS = 0;
uint8_t TARGET_LORA_ADDRESS = 0;

DeviceState local;
DeviceState remote;
TimingData timing;
SpinnerData spinner;
HealthMonitor health;  // Connection watchdog & health monitoring

// Kill-switch state
unsigned long killSwitchPressStart = 0;
unsigned long lastKillSwitchDebug = 0;

// Bi-directional communication stats
int ackReceived = 0;         // Number of ACKs received (sender)
unsigned long lastAckTime = 0;  // Last ACK timestamp (sender)

// =============== KILL-SWITCH FUNCTIONS ================================

void initKillSwitch() {
  pinMode(KILLSWITCH_GND_PIN, OUTPUT);
  digitalWrite(KILLSWITCH_GND_PIN, LOW);
  pinMode(KILLSWITCH_READ_PIN, INPUT_PULLUP);
  delay(100);  // Longer delay for pin stabilization

  Serial.println("✓ Kill-switch initialized: GPIO13↔14, hold 3s to restart");

  // Test pins immediately
  Serial.print("  GPIO14 (GND): ");
  int gpio14 = digitalRead(KILLSWITCH_GND_PIN);
  Serial.println(gpio14 == LOW ? "LOW ✓" : "HIGH ❌");

  Serial.print("  GPIO13 (READ): ");
  int gpio13 = digitalRead(KILLSWITCH_READ_PIN);
  Serial.print(gpio13);
  Serial.println(gpio13 == HIGH ? " (HIGH ✓ - not pressed)" : " (LOW - PRESSED!)");
}

void checkKillSwitch() {
  bool pressed = (digitalRead(KILLSWITCH_READ_PIN) == LOW);

  // Debug: Print GPIO state every 2 seconds if debug enabled
  if (KILLSWITCH_DEBUG && millis() - lastKillSwitchDebug >= 2000) {
    lastKillSwitchDebug = millis();
    Serial.print("[KillSwitch Debug] GPIO13: ");
    Serial.print(digitalRead(KILLSWITCH_READ_PIN));
    Serial.println(pressed ? " (PRESSED)" : " (released)");
  }

  if (pressed) {
    if (killSwitchPressStart == 0) {
      killSwitchPressStart = millis();
      Serial.println("\n🔴 Kill-switch PRESSED - hold to restart...");
    }

    unsigned long pressDuration = millis() - killSwitchPressStart;

    // Show countdown every second
    if (pressDuration >= 1000 && pressDuration < 1100) {
      Serial.println("🔴 2 more seconds...");
    }
    else if (pressDuration >= 2000 && pressDuration < 2100) {
      Serial.println("🔴 1 more second...");
    }
    else if (pressDuration >= KILLSWITCH_HOLD_TIME) {
      executeRestart("Physical kill-switch");
    }
  }
  else {
    if (killSwitchPressStart > 0) {
      Serial.println("✓ Kill-switch released (no restart)");
      killSwitchPressStart = 0;
    }
  }
}

void executeRestart(const char* reason) {
  Serial.println("\n╔════════════════════════════════╗");
  Serial.print("║  🔴 RESTART: ");
  Serial.print(reason);
  for (int i = strlen(reason); i < 18; i++) Serial.print(" ");
  Serial.println("║");
  Serial.println("╚════════════════════════════════╝\n");
  Serial.flush();
  delay(100);

  // Use ESP32 specific restart for reliability
  #ifdef ESP32
    esp_restart();
  #else
    ESP.restart();
  #endif
}

void processRemoteKillSwitch(String payload) {
  // Check for remote kill-switch commands
  if (payload.indexOf("CMD:RESTART") >= 0) {
    Serial.println("\n⚠️  REMOTE RESTART COMMAND RECEIVED!");
    executeRestart("Remote command");
  }
  else if (payload.indexOf("CMD:STOP") >= 0) {
    Serial.println("\n⚠️  REMOTE STOP COMMAND RECEIVED!");
    Serial.println("(Feature not yet implemented)");
  }
}

// =============== FUNCTIONS ================================

void updateSpinner() {
  if (millis() - timing.lastSpinner >= 150) {
    timing.lastSpinner = millis();
    local.spinnerIndex++;
    if (local.spinnerIndex >= 4) local.spinnerIndex = 0;
  }
}

void parsePayload(String payload) {
  // Parse: SEQ:x,LED:x,TOUCH:x,SPIN:x,COUNT:x
  int seqIdx = payload.indexOf("SEQ:");
  int ledIdx = payload.indexOf("LED:");
  int touchIdx = payload.indexOf("TOUCH:");
  int spinIdx = payload.indexOf("SPIN:");

  // Parse sequence number
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
    // Bounds check to prevent buffer overflow!
    if (remote.spinnerIndex < 0 || remote.spinnerIndex >= 4) {
      remote.spinnerIndex = 0;
    }
  }
}

// =============== LCD HELPER FUNCTIONS ================================

// Create visual signal strength bar
// barWidth: number of characters for the bar (e.g., 10 for wide bar)
// Returns string like "[████████░░]"
String getSignalBar(int rssi, int barWidth) {
  // RSSI typically ranges from -120 (worst) to -40 (best)
  // Map to number of filled bars
  int filled = map(rssi, -120, -40, 0, barWidth);
  filled = constrain(filled, 0, barWidth);

  String bar = "[";
  for (int i = 0; i < barWidth; i++) {
    if (i < filled) {
      bar += (char)0xFF;  // Solid block █
    } else {
      bar += (char)0xA1;  // Light shade ░
    }
  }
  bar += "]";
  return bar;
}

// Get signal quality icon based on RSSI
char getSignalIcon(int rssi) {
  if (rssi > -50) return '^';       // Excellent
  else if (rssi > -80) return '=';  // Good
  else if (rssi > -100) return '-'; // Fair
  else if (rssi > -110) return 'v'; // Poor
  else return 'X';                  // Critical
}

// =============== LCD DISPLAY VERSIONS ================================
// Uncomment ONE version at a time to use it

void updateLCD() {
  if (millis() - timing.lastLCD >= 100) {
    timing.lastLCD = millis();

    // Check for connection timeout (10 seconds)
    bool connectionLost = (bRECEIVER && millis() - remote.lastMessageTime > 10000);

    if (connectionLost) {
      // Connection lost warning
      lcd.setCursor(0, 0);
      lcd.print("*** NO SIGNAL ***");
      lcd.setCursor(0, 1);
      unsigned long seconds = (millis() - remote.lastMessageTime) / 1000;
      lcd.print("Last: ");
      lcd.print(seconds);
      lcd.print("s ago    ");
      return;
    }

    // VERSION 1: WIDE VISUAL BAR + RSSI (RECOMMENDED) ⭐
    // Uncomment this version:
  //  updateLCD_Version1_WideBar();

    // VERSION 2: COMPACT WITH NUMBERS
    // Uncomment this version:
     updateLCD_Version2_Compact();

    // VERSION 3: DETAILED INFO
    // Uncomment this version:
    // updateLCD_Version3_Detailed();

    // VERSION 4: ORIGINAL (Status only, no signal)
    // Uncomment this version:
    // updateLCD_Version4_Original();
  }
}

// =============== VERSION 1: WIDE VISUAL BAR ⭐ ================================
void updateLCD_Version1_WideBar() {
  // Line 1: Connection status + signal bar + count + remote spinner
  lcd.setCursor(0, 0);

  // Connection status icon
  lcd.print(getConnectionIcon(health.state));

  lcd.print(getSignalBar(remote.rssi, 10));  // [██████░░░░] (10 chars to fit status)
  lcd.print(remote.messageCount % 100);  // Max 2 digits
  lcd.print(" ");

  // Remote spinner (received via LoRa, updates slowly)
  lcd.setCursor(15, 0);
  lcd.print(spinner.symbols[remote.spinnerIndex]);

  // Line 2: RSSI value + local status + local spinner
  lcd.setCursor(0, 1);
  lcd.print(remote.rssi);
  lcd.print("dB L:");
  lcd.print(local.ledState);
  lcd.print(" T:");
  lcd.print(local.touchState);
  lcd.print("  ");

  // Local spinner (fast animation showing system is responsive)
  lcd.setCursor(15, 1);
  lcd.print(spinner.symbols[local.spinnerIndex]);
}

// =============== VERSION 2: COMPACT ================================
void updateLCD_Version2_Compact() {
  // Line 1: Connection + signal bar + RSSI + remote spinner
  lcd.setCursor(0, 0);

  // Connection status icon
  lcd.print(getConnectionIcon(health.state));

  lcd.print(getSignalBar(remote.rssi, 7));  // 7 chars to fit status icon
  lcd.print(remote.rssi);
  lcd.print(" ");

  // Remote spinner (received via LoRa, updates slowly)
  lcd.setCursor(15, 0);
  lcd.print(spinner.symbols[remote.spinnerIndex]);

  // Line 2: SNR + local/remote status + counter + local spinner
  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(remote.snr);
  lcd.print(" L:");
  lcd.print(local.ledState);
  lcd.print(" R:");
  lcd.print(remote.ledState);
  lcd.print(" ");
  lcd.print(remote.messageCount % 100);
  lcd.print(" ");

  // Local spinner (fast animation showing system is responsive)
  lcd.setCursor(15, 1);
  lcd.print(spinner.symbols[local.spinnerIndex]);
}

// =============== VERSION 3: DETAILED INFO ================================
void updateLCD_Version3_Detailed() {
  // Line 1: RSSI + SNR + signal icon + count
  lcd.setCursor(0, 0);
  lcd.print("RX:");
  lcd.print(remote.messageCount);
  lcd.print(" ");
  lcd.print(remote.rssi);
  lcd.print("dB ");
  lcd.print(getSignalIcon(remote.rssi));
  lcd.print("   ");

  // Line 2: SNR + Local + Remote status + spinner
  lcd.setCursor(0, 1);
  lcd.print("SNR:");
  lcd.print(remote.snr);
  lcd.print(" L:");
  lcd.print(local.ledState);
  lcd.print(" R:");
  lcd.print(remote.ledState);
  lcd.print(" ");

  lcd.setCursor(15, 1);
  lcd.print(spinner.symbols[local.spinnerIndex]);
}

// =============== VERSION 4: ORIGINAL (No signal info) ================================
void updateLCD_Version4_Original() {
  // Line 1: Remote status
  lcd.setCursor(0, 0);
  lcd.print("REM:");
  lcd.print(remote.ledState);
  lcd.print(" T:");
  lcd.print(remote.touchState);
  lcd.print("    ");

  lcd.setCursor(15, 0);
  lcd.print(spinner.symbols[remote.spinnerIndex]);

  // Line 2: Local status
  lcd.setCursor(0, 1);
  lcd.print("LOC:");
  lcd.print(local.ledState);
  lcd.print(" T:");
  lcd.print(local.touchState);
  lcd.print("    ");

  lcd.setCursor(15, 1);
  lcd.print(spinner.symbols[local.spinnerIndex]);
}

// =============== PC DATA LOGGING ================================
// CSV and JSON output for Python data logging

void printDataCSV() {
  // Format: DATA_CSV,TIMESTAMP,ROLE,RSSI,SNR,SEQ,MSG_COUNT,CONN_STATE,PACKET_LOSS,LED,TOUCH
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

void printDataJSON() {
  // JSON format for easier parsing
  Serial.print("{\"ts\":");
  Serial.print(millis());
  Serial.print(",\"role\":\"");
  Serial.print(bRECEIVER ? "RX" : "TX");
  Serial.print("\",\"rssi\":");
  Serial.print(remote.rssi);
  Serial.print(",\"snr\":");
  Serial.print(remote.snr);
  Serial.print(",\"seq\":");
  Serial.print(remote.sequenceNumber);
  Serial.print(",\"count\":");
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

void printStatus() {
  Serial.println("\n============================");
  
  if (bRECEIVER) {
    Serial.println("--- RECEIVER ---");
    Serial.print("Messages RX: ");
    Serial.println(remote.messageCount);
    Serial.print("Remote LED: ");
    Serial.print(remote.ledState);
    Serial.print(", Touch: ");
    Serial.println(remote.touchState);
    Serial.print("RSSI: ");
    Serial.print(remote.rssi);
    Serial.print(" dBm, SNR: ");
    Serial.println(remote.snr);
  } else {
    Serial.println("--- SENDER ---");
    Serial.print("Messages TX: ");
    Serial.println(local.messageCount);
    #if ENABLE_BIDIRECTIONAL
    Serial.print("ACKs RX: ");
    Serial.print(ackReceived);
    if (lastAckTime > 0) {
      Serial.print(" (last: ");
      Serial.print((millis() - lastAckTime) / 1000);
      Serial.print("s ago)");
    }
    Serial.println();
    #endif
  }

  Serial.print("Local LED: ");
  Serial.print(local.ledState);
  Serial.print(", Touch: ");
  Serial.println(local.touchState);
  Serial.println("============================");
}

// =============== SETUP ================================
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n\n");
  Serial.println("╔════════════════════════════╗");
  Serial.println("║  ZignalMeister 2000        ║");
  Serial.println("╚════════════════════════════╝");

  // Initialize kill-switch first
  initKillSwitch();

  // Auto-detect role
  pinMode(MODE_GND_PIN, OUTPUT);
  digitalWrite(MODE_GND_PIN, LOW);
  pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
  delay(100);
  
  bRECEIVER = (digitalRead(MODE_SELECT_PIN) == LOW);

  // ============================================
  // DEBUG: Show pin state and role detection
  // ============================================
  Serial.println("\n╔════════════════════════════╗");
  Serial.println("║   MODE DETECTION DEBUG    ║");
  Serial.println("╠════════════════════════════╣");
  Serial.print("║ MODE_GND_PIN (");
  Serial.print(MODE_GND_PIN);
  Serial.println(") -> LOW     ║");
  Serial.print("║ MODE_SELECT_PIN (");
  Serial.print(MODE_SELECT_PIN);
  Serial.print("): ");
  int pinReading = digitalRead(MODE_SELECT_PIN);
  if (pinReading == LOW) {
    Serial.println("LOW  ║");
  } else {
    Serial.println("HIGH ║");
  }
  Serial.println("╚════════════════════════════╝");

  if (bRECEIVER) {
    Serial.println("\n>>> RECEIVER MODE");
    Serial.println(">>> Expected: GPIO 16 connected to GPIO 17 (jumper wire)");
    MY_LORA_ADDRESS = LORA_RECEIVER_ADDRESS;
    TARGET_LORA_ADDRESS = LORA_SENDER_ADDRESS;
  } else {
    Serial.println("\n>>> SENDER MODE");
    Serial.println(">>> Expected: GPIO 16 floating (no connection)");
    MY_LORA_ADDRESS = LORA_SENDER_ADDRESS;
    TARGET_LORA_ADDRESS = LORA_RECEIVER_ADDRESS;
  }

  Serial.println("\n╔════════════════════════════╗");
  Serial.println("║   LoRa CONFIGURATION      ║");
  Serial.println("╠════════════════════════════╣");
  Serial.print("║ My Address:     ");
  Serial.print(MY_LORA_ADDRESS);
  Serial.println("          ║");
  Serial.print("║ Target Address: ");
  Serial.print(TARGET_LORA_ADDRESS);
  Serial.println("          ║");
  Serial.print("║ Network ID:     ");
  Serial.print(LORA_NETWORK_ID);
  Serial.println("          ║");
  Serial.println("╚════════════════════════════╝");
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize structs
  // DeviceState: ledState, ledCount, touchState, touchValue, messageCount, lastMessageTime, sequenceNumber, spinnerIndex, rssi, snr
  local = {LOW, 0, false, 0, 0, 0, 0, 0, 0, 0};   // 10 fields - added snr
  remote = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};         // 10 fields - added snr
  timing = {0, 0, 0, 0, 0, 0, 0, 0};               // 8 fields
  spinner = {{'<', '^', '>', 'v'}, 0, 0};
  
  // Initialize health monitor (BOTH roles - needed for PC data logging!)
  initHealthMonitor(health);

  // Initialize LoRa
  if (!initLoRa(MY_LORA_ADDRESS, LORA_NETWORK_ID)) {
    Serial.println("\n❌ LoRa init failed!");
    Serial.println("⚠️  Continuing anyway - kill-switch still works!");
    Serial.println("💡 Connect GPIO 13↔14 and hold 3s to restart\n");
    // Don't freeze - let kill-switch work even if LoRa fails!
  }

  // LCD for receiver
  if (bRECEIVER) {
    initLCD();
    // Initialize lastMessageTime to current time to prevent false "NO SIGNAL" at startup
    remote.lastMessageTime = millis();
  }

  // Display station (UART-based, works for both sender and receiver)
  initDisplaySender();

  // Feature modules initialization
  #if ENABLE_BATTERY_MONITOR
    initBatteryMonitor();
  #endif

  #if ENABLE_AUDIO_DETECTION
    initAudioDetector();
  #endif

  #if ENABLE_LIGHT_DETECTION
    initLightDetector();
  #endif

  #if ENABLE_CURRENT_MONITOR
    initCurrentMonitor();
  #endif

  Serial.println("\n✓ Setup complete!\n");
}

// =============== LOOP ================================
void loop() {
  // Check kill-switch every loop (highest priority!)
  checkKillSwitch();

  // Debug: Confirm loop is running (only first 5 times)
  static int loopCounter = 0;
  if (loopCounter < 5) {
    loopCounter++;
    Serial.print("✓ Loop running (");
    Serial.print(loopCounter);
    Serial.println("/5)");
  }

  // Spinner
  updateSpinner();
  
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

      #if ENABLE_BIDIRECTIONAL
      // Send ACK every ACK_INTERVAL messages
      if (remote.messageCount % ACK_INTERVAL == 0) {
        // ACK includes receiver's current state
        String ackPayload = "ACK," +
                            String("SEQ:") + String(local.sequenceNumber) +
                            ",LED:" + String(local.ledState) +
                            ",TOUCH:" + String(local.touchState) +
                            ",SPIN:" + String(local.spinnerIndex);

        Serial.print("📤 Sending ACK (#");
        Serial.print(remote.messageCount);
        Serial.println(")...");

        delay(50);  // Small delay before sending (LoRa turnaround time)
        if (sendLoRaMessage(ackPayload, TARGET_LORA_ADDRESS)) {
          local.messageCount++;
          local.sequenceNumber++;
          Serial.println("✓ ACK sent");
        } else {
          Serial.println("❌ ACK send failed");
        }
      }
      #endif
    }

    // Update connection state (watchdog)
    updateConnectionState(health, remote);

    // Attempt recovery if connection lost
    if (health.state == CONN_LOST) {
      attemptRecovery(health, MY_LORA_ADDRESS, LORA_NETWORK_ID);
    }

    updateLCD();

    if (millis() - timing.lastCheck >= 5000) {
      timing.lastCheck = millis();
      printStatus();
    }

    // Print health report every 30 seconds
    if (millis() - timing.lastHealthReport >= 30000) {
      timing.lastHealthReport = millis();
      printHealthReport(health, remote);
    }

  } else {
    // SENDER: Send every 2 seconds
    if (millis() - timing.lastSend >= 2000) {
      timing.lastSend = millis();

      // Include sequence number in payload
      String payload = "SEQ:" + String(local.sequenceNumber) +
                       ",LED:" + String(local.ledState) +
                       ",TOUCH:" + String(local.touchState) +
                       ",SPIN:" + String(local.spinnerIndex) +
                       ",COUNT:" + String(local.messageCount);

      if (sendLoRaMessage(payload, TARGET_LORA_ADDRESS)) {
        local.messageCount++;
        local.sequenceNumber++;  // Increment sequence number

        #if ENABLE_BIDIRECTIONAL
        // Listen for ACK/response for a short time
        unsigned long listenStart = millis();
        while (millis() - listenStart < LISTEN_TIMEOUT) {
          String response;
          if (receiveLoRaMessage(remote, response)) {
            // Process received ACK or data
            if (response.indexOf("ACK") >= 0) {
              ackReceived++;
              lastAckTime = millis();
              Serial.print("✓ ACK #");
              Serial.print(ackReceived);
              Serial.print(" received (RSSI: ");
              Serial.print(remote.rssi);
              Serial.println(" dBm)");
              parsePayload(response);  // Parse any data in ACK
              // Update health monitoring for sender too
              updateRSSI(health, remote.rssi);
              trackPacket(health, remote.sequenceNumber);
            }
            break;  // Got response, stop listening
          }
          delay(10);
        }
        #endif
      }
    }

    // Send update to display station (if enabled)
    sendDisplayUpdate();

    if (millis() - timing.lastCheck >= 5000) {
      timing.lastCheck = millis();
      printStatus();
    }
  }

  // PC Data Logging (both roles)
  if (millis() - timing.lastDataOutput >= DATA_OUTPUT_INTERVAL) {
    timing.lastDataOutput = millis();

    #if ENABLE_CSV_OUTPUT
      printDataCSV();
    #endif

    #if ENABLE_JSON_OUTPUT
      printDataJSON();
    #endif
  }

  // Feature modules monitoring
  #if ENABLE_BATTERY_MONITOR
    checkBattery();
  #endif

  #if ENABLE_AUDIO_DETECTION
    checkAudioDetector();
  #endif

  #if ENABLE_LIGHT_DETECTION
    checkLightDetector();
  #endif

  #if ENABLE_CURRENT_MONITOR
    checkCurrentMonitor();
  #endif

  delay(10);
}