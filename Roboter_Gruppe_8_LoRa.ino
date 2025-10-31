/*=====================================================================
  Roboter_Gruppe_8_LoRa_Final.ino
  
  Final working version with:
  - Auto role detection (GPIO15↔GPIO13 jumper)
  - Working LoRa communication
  - LCD display on receiver
  - Touch sensor and LED
  
  Receiver: GPIO15 connected to GPIO13
  Sender:   GPIO15 floating
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

void updateLCD() {
  if (millis() - timing.lastLCD >= 100) {
    timing.lastLCD = millis();
    
    // Top row - Remote
    lcd.setCursor(0, 0);
    lcd.print("REM:");
    lcd.print(remote.ledState);
    lcd.print(" T:");
    lcd.print(remote.touchState);
    lcd.print("    ");
    
    lcd.setCursor(15, 0);
    lcd.print(spinner.symbols[remote.spinnerIndex]);
    
    // Bottom row - Local
    lcd.setCursor(0, 1);
    lcd.print("LOC:");
    lcd.print(local.ledState);
    lcd.print(" T:");
    lcd.print(local.touchState);
    lcd.print("    ");
    
    lcd.setCursor(15, 1);
    lcd.print(spinner.symbols[local.spinnerIndex]);
  }
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
  Serial.println("║  ZignalMeister LoRa Final ║");
  Serial.println("╚════════════════════════════╝");
  
  // Auto-detect role
  pinMode(MODE_GND_PIN, OUTPUT);
  digitalWrite(MODE_GND_PIN, LOW);
  pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
  delay(100);
  
  bRECEIVER = (digitalRead(MODE_SELECT_PIN) == LOW);
  
  if (bRECEIVER) {
    Serial.println("\n>>> RECEIVER MODE");
    MY_LORA_ADDRESS = LORA_RECEIVER_ADDRESS;
    TARGET_LORA_ADDRESS = LORA_SENDER_ADDRESS;
  } else {
    Serial.println("\n>>> SENDER MODE");
    MY_LORA_ADDRESS = LORA_SENDER_ADDRESS;
    TARGET_LORA_ADDRESS = LORA_RECEIVER_ADDRESS;
  }
  
  Serial.print("My Address: ");
  Serial.println(MY_LORA_ADDRESS);
  Serial.print("Target Address: ");
  Serial.println(TARGET_LORA_ADDRESS);
  
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