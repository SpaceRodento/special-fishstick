/*=====================================================================
  Roboter_Gruppe_8_LoRa_Final.ino

  Final working version with:
  - Auto role detection (GPIO15↔GPIO17 jumper)
  - Working LoRa communication (RYLR896 module)
  - LCD display on receiver
  - Touch sensor and LED

  Hardware Setup:
  - RYLR896 TX -> ESP32 GPIO25 (RX)
  - RYLR896 RX -> ESP32 GPIO26 (TX)

  Role Detection:
  - Receiver: GPIO16 connected to GPIO17 (jumper wire)
  - Sender:   GPIO16 floating (no connection)
  - Note: GPIO16 and GPIO17 are physically next to each other

  Both devices run IDENTICAL code!
  Role is auto-detected based on jumper wire.
=======================================================================*/

#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

#include "config.h"
#include "structs.h"
#include "functions.h"
#include "lora_handler.h"

// =============== GLOBALS ================================
bool bRECEIVER = 0;  // Auto-detected
uint8_t MY_LORA_ADDRESS = 0;
uint8_t TARGET_LORA_ADDRESS = 0;

DeviceState local;
DeviceState remote;
TimingData timing;
SpinnerData spinner;

// =============== FUNCTIONS ================================

void updateSpinner() {
  if (millis() - timing.lastSpinner >= 150) {
    timing.lastSpinner = millis();
    local.spinnerIndex++;
    if (local.spinnerIndex >= 4) local.spinnerIndex = 0;
  }
}

void parsePayload(String payload) {
  // Parse: LED:x,TOUCH:x,SPIN:x,COUNT:x
  int ledIdx = payload.indexOf("LED:");
  int touchIdx = payload.indexOf("TOUCH:");
  int spinIdx = payload.indexOf("SPIN:");
  
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
  // Line 1: Wide signal bar (11 chars) + count + remote spinner
  lcd.setCursor(0, 0);
  lcd.print(getSignalBar(remote.rssi, 11));  // [███████░░░░]
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
  // Line 1: Signal bar (8 chars) + RSSI + remote spinner
  lcd.setCursor(0, 0);
  lcd.print(getSignalBar(remote.rssi, 8));
  //lcd.print(" ");
  lcd.print(remote.rssi);
  lcd.print("dB");
  lcd.print(" ");

  // Remote spinner (received via LoRa, updates slowly)
  lcd.setCursor(15, 0);
  lcd.print(spinner.symbols[remote.spinnerIndex]);

  // Line 2: SNR + status + counter + local spinner
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
  local = {LOW, 0, false, 0, 0, 0, 0, 0};
  remote = {0, 0, 0, 0, 0, 0, 0, 0};
  timing = {0, 0, 0, 0, 0, 0};
  spinner = {{'<', '^', '>', 'v'}, 0, 0};
  
  // Initialize LoRa
  if (!initLoRa(MY_LORA_ADDRESS, LORA_NETWORK_ID)) {
    Serial.println("\n❌ LoRa init failed!");
    while(1) delay(1000);
  }
  
  // LCD for receiver
  if (bRECEIVER) {
    initLCD();
    // Initialize lastMessageTime to current time to prevent false "NO SIGNAL" at startup
    remote.lastMessageTime = millis();
  }

  Serial.println("\n✓ Setup complete!\n");
}

// =============== LOOP ================================
void loop() {
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
    }
    
    updateLCD();
    
    if (millis() - timing.lastCheck >= 5000) {
      timing.lastCheck = millis();
      printStatus();
    }
    
  } else {
    // SENDER: Send every 2 seconds
    if (millis() - timing.lastSend >= 2000) {
      timing.lastSend = millis();
      
      String payload = "LED:" + String(local.ledState) + 
                       ",TOUCH:" + String(local.touchState) + 
                       ",SPIN:" + String(local.spinnerIndex) +
                       ",COUNT:" + String(local.messageCount);
      
      if (sendLoRaMessage(payload, TARGET_LORA_ADDRESS)) {
        local.messageCount++;
      }
    }
    
    if (millis() - timing.lastCheck >= 5000) {
      timing.lastCheck = millis();
      printStatus();
    }
  }
  
  delay(10);
}