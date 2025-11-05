/*=====================================================================
  Roboter_Display_TFT.ino - LoRa Display Station

  ESP32-2432S022 TFT Display (240x320)

  Role: Standalone LoRa receiver that displays real-time status
        from the main ESP32 sender device.

  Hardware:
  - ESP32-WROOM-32
  - 2.4" ST7789 TFT (240x320, 8-bit parallel)
  - CST820 Touch (I2C)
  - RYLR896 LoRa module

  LoRa Connections:
  - TX (LoRa) → GPIO 18 (ESP32 RX1)
  - RX (LoRa) → GPIO 26 (ESP32 TX1)
  - VCC → 3.3V
  - GND → GND

  Address Configuration:
  - This device: Address 3 (DISPLAY_ADDRESS)
  - Main sender: Address 2 (LORA_SENDER_ADDRESS)
  - Receives broadcast (Address 0) and direct messages

  Features:
  - Real-time LoRa data display
  - Touch-free operation (display only)
  - Battery status monitoring
  - RSSI signal strength indicator
  - Alert notifications

  Author: Roboter Gruppe 9
  Version: 1.0
  Date: 2025-01-05
=======================================================================*/

#include <Arduino.h>

// LovyanGFX for ESP32-2432S022 TFT display
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "display_config.h"

// =============== LORA CONFIGURATION ================================
#define LORA_BAUDRATE 115200
#define LORA_RX_PIN 18                  // ESP32 RX1 ← LoRa TX
#define LORA_TX_PIN 26                  // ESP32 TX1 → LoRa RX
#define DISPLAY_ADDRESS 3               // This display's LoRa address
#define LORA_SENDER_ADDRESS 2           // Main sender address
#define LORA_NETWORK_ID 6               // Must match main device

// =============== DISPLAY CONFIGURATION ================================
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define BACKLIGHT_PIN 0
#define BACKLIGHT_BRIGHTNESS 200        // 0-255

// =============== UPDATE INTERVALS ================================
#define DISPLAY_UPDATE_INTERVAL 500     // Update display every 500ms
#define LORA_TIMEOUT 10000              // No data timeout (10s)
#define HEADER_UPDATE_INTERVAL 1000     // Update header every 1s

// =============== COLORS ================================
#define COLOR_BG 0x0000          // Black
#define COLOR_HEADER 0x001F      // Blue
#define COLOR_TEXT 0xFFFF        // White
#define COLOR_LABEL 0xC618       // Gray
#define COLOR_GOOD 0x07E0        // Green
#define COLOR_WARN 0xFD20        // Orange
#define COLOR_BAD 0xF800         // Red
#define COLOR_ALERT 0xFFE0       // Yellow

// =============== GLOBAL OBJECTS ================================
static LGFX tft;
HardwareSerial LoRaSerial(1);  // Use UART1 for LoRa

// =============== DATA STRUCTURES ================================
struct DisplayData {
  // LoRa status
  bool loraConnected;
  unsigned long lastDataTime;
  int rssi;
  int snr;
  String lastMessage;

  // Sensor data from main device
  int sequence;
  bool ledState;
  bool touchState;

  // Battery
  float batteryVoltage;
  int batteryPercent;

  // Telemetry
  unsigned long uptime;
  int freeHeap;
  int temperature;
  int loopFrequency;

  // Alerts
  bool audioAlert;
  bool lightAlert;
  String alertMessage;

  // Statistics
  unsigned long packetsReceived;
  unsigned long packetErrors;
};

DisplayData displayData = {
  false, 0, 0, 0, "",
  0, false, false,
  0.0, 0,
  0, 0, 0, 0,
  false, false, "",
  0, 0
};

// =============== DISPLAY REGIONS ================================
// Screen layout: Header | Status | Telemetry | Alerts | Footer
#define HEADER_Y 0
#define HEADER_H 40
#define STATUS_Y 40
#define STATUS_H 100
#define TELEMETRY_Y 140
#define TELEMETRY_H 100
#define ALERTS_Y 240
#define ALERTS_H 60
#define FOOTER_Y 300
#define FOOTER_H 20

// =============== FUNCTION PROTOTYPES ================================
void initLoRa();
void updateLoRaData();
void parseLoRaMessage(String message, int rssi, int snr);
void updateDisplay();
void drawHeader();
void drawStatus();
void drawTelemetry();
void drawAlerts();
void drawFooter();
String formatUptime(unsigned long seconds);
int getRSSIColor(int rssi);
void testDisplay();

// =============== SETUP ================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  ROBOTER GRUPPE 9 - DISPLAY STATION   ║");
  Serial.println("║  ESP32-2432S022 + LoRa Display        ║");
  Serial.println("╚════════════════════════════════════════╝\n");

  // Initialize backlight
  pinMode(BACKLIGHT_PIN, OUTPUT);
  analogWrite(BACKLIGHT_PIN, BACKLIGHT_BRIGHTNESS);

  // Initialize TFT display
  Serial.println("📺 Initializing TFT display...");
  tft.init();
  tft.setRotation(0);  // Portrait mode (240x320)
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);

  // Show boot screen
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString("ROBOTER", TFT_WIDTH/2, TFT_HEIGHT/2 - 20);
  tft.drawString("GRUPPE 9", TFT_WIDTH/2, TFT_HEIGHT/2 + 20);
  tft.setTextSize(1);
  tft.drawString("Display Station", TFT_WIDTH/2, TFT_HEIGHT/2 + 50);
  delay(2000);

  // Initialize LoRa
  Serial.println("📡 Initializing LoRa...");
  initLoRa();

  // Clear screen and draw initial layout
  tft.fillScreen(COLOR_BG);
  drawHeader();
  drawStatus();
  drawTelemetry();
  drawAlerts();
  drawFooter();

  Serial.println("✓ Display station ready!\n");
  Serial.println("Waiting for LoRa data...");

  displayData.lastDataTime = millis();
}

// =============== MAIN LOOP ================================
void loop() {
  static unsigned long lastDisplayUpdate = 0;
  static unsigned long lastHeaderUpdate = 0;
  unsigned long now = millis();

  // Check for LoRa data
  updateLoRaData();

  // Update header (time, connection status)
  if (now - lastHeaderUpdate >= HEADER_UPDATE_INTERVAL) {
    drawHeader();
    lastHeaderUpdate = now;
  }

  // Update display
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    drawStatus();
    drawTelemetry();
    drawAlerts();
    drawFooter();
    lastDisplayUpdate = now;
  }

  // Check for timeout
  if (now - displayData.lastDataTime > LORA_TIMEOUT) {
    if (displayData.loraConnected) {
      displayData.loraConnected = false;
      Serial.println("⚠️  LoRa connection timeout!");
      drawHeader();  // Update connection indicator
    }
  }

  delay(10);
}

// =============== LORA FUNCTIONS ================================

void initLoRa() {
  LoRaSerial.begin(LORA_BAUDRATE, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
  delay(100);

  // Configure LoRa module
  Serial.println("  Configuring RYLR896...");

  // Set address
  LoRaSerial.print("AT+ADDRESS=");
  LoRaSerial.println(DISPLAY_ADDRESS);
  delay(100);

  // Set network ID
  LoRaSerial.print("AT+NETWORKID=");
  LoRaSerial.println(LORA_NETWORK_ID);
  delay(100);

  // Get version
  LoRaSerial.println("AT+VER?");
  delay(100);

  // Print responses
  while (LoRaSerial.available()) {
    String response = LoRaSerial.readStringUntil('\n');
    Serial.println("  " + response);
  }

  Serial.println("✓ LoRa initialized");
  Serial.print("  Address: ");
  Serial.println(DISPLAY_ADDRESS);
  Serial.print("  Network ID: ");
  Serial.println(LORA_NETWORK_ID);
}

void updateLoRaData() {
  if (LoRaSerial.available()) {
    String line = LoRaSerial.readStringUntil('\n');
    line.trim();

    // Parse LoRa message: +RCV=addr,len,data,rssi,snr
    if (line.startsWith("+RCV=")) {
      displayData.packetsReceived++;
      displayData.lastDataTime = millis();
      displayData.loraConnected = true;

      // Parse message
      line = line.substring(5);  // Remove "+RCV="

      // Extract address
      int comma1 = line.indexOf(',');
      int fromAddr = line.substring(0, comma1).toInt();

      // Extract length
      int comma2 = line.indexOf(',', comma1 + 1);
      int dataLen = line.substring(comma1 + 1, comma2).toInt();

      // Extract data
      int comma3 = line.indexOf(',', comma2 + 1);
      String data = line.substring(comma2 + 1, comma3);

      // Extract RSSI
      int comma4 = line.indexOf(',', comma3 + 1);
      int rssi = line.substring(comma3 + 1, comma4).toInt();

      // Extract SNR
      int snr = line.substring(comma4 + 1).toInt();

      // Store RSSI/SNR
      displayData.rssi = rssi;
      displayData.snr = snr;
      displayData.lastMessage = data;

      // Parse message data
      parseLoRaMessage(data, rssi, snr);

      // Debug
      Serial.print("📡 RX: ");
      Serial.print(data);
      Serial.print(" [RSSI:");
      Serial.print(rssi);
      Serial.print(" SNR:");
      Serial.print(snr);
      Serial.println("]");
    }
  }
}

void parseLoRaMessage(String message, int rssi, int snr) {
  // Parse CSV format: SEQ:123,LED:1,TOUCH:0,BAT:3.85,...

  // Sequence number
  int seqIdx = message.indexOf("SEQ:");
  if (seqIdx >= 0) {
    int commaIdx = message.indexOf(',', seqIdx);
    if (commaIdx < 0) commaIdx = message.length();
    displayData.sequence = message.substring(seqIdx + 4, commaIdx).toInt();
  }

  // LED state
  int ledIdx = message.indexOf("LED:");
  if (ledIdx >= 0) {
    int commaIdx = message.indexOf(',', ledIdx);
    if (commaIdx < 0) commaIdx = message.length();
    displayData.ledState = message.substring(ledIdx + 4, commaIdx).toInt() == 1;
  }

  // Touch state
  int touchIdx = message.indexOf("TOUCH:");
  if (touchIdx >= 0) {
    int commaIdx = message.indexOf(',', touchIdx);
    if (commaIdx < 0) commaIdx = message.length();
    displayData.touchState = message.substring(touchIdx + 6, commaIdx).toInt() == 1;
  }

  // Battery voltage
  int batIdx = message.indexOf("BAT:");
  if (batIdx >= 0) {
    int commaIdx = message.indexOf(',', batIdx);
    if (commaIdx < 0) commaIdx = message.length();
    displayData.batteryVoltage = message.substring(batIdx + 4, commaIdx).toFloat();

    // Calculate percentage (3.0V = 0%, 4.2V = 100%)
    displayData.batteryPercent = (int)((displayData.batteryVoltage - 3.0) / 1.2 * 100);
    if (displayData.batteryPercent < 0) displayData.batteryPercent = 0;
    if (displayData.batteryPercent > 100) displayData.batteryPercent = 100;
  }

  // Uptime
  int upIdx = message.indexOf("UP:");
  if (upIdx >= 0) {
    int commaIdx = message.indexOf(',', upIdx);
    if (commaIdx < 0) commaIdx = message.length();
    displayData.uptime = message.substring(upIdx + 3, commaIdx).toInt();
  }

  // Free heap
  int heapIdx = message.indexOf("HEAP:");
  if (heapIdx >= 0) {
    int commaIdx = message.indexOf(',', heapIdx);
    if (commaIdx < 0) commaIdx = message.length();
    displayData.freeHeap = message.substring(heapIdx + 5, commaIdx).toInt();
  }

  // Temperature
  int tempIdx = message.indexOf("TEMP:");
  if (tempIdx >= 0) {
    int commaIdx = message.indexOf(',', tempIdx);
    if (commaIdx < 0) commaIdx = message.length();
    displayData.temperature = message.substring(tempIdx + 5, commaIdx).toInt();
  }

  // Loop frequency
  int loopIdx = message.indexOf("LOOP:");
  if (loopIdx >= 0) {
    int commaIdx = message.indexOf(',', loopIdx);
    if (commaIdx < 0) commaIdx = message.length();
    displayData.loopFrequency = message.substring(loopIdx + 5, commaIdx).toInt();
  }

  // Alerts
  if (message.indexOf("ALERT:FIRE_AUDIO") >= 0) {
    displayData.audioAlert = true;
    displayData.alertMessage = "FIRE: Audio detected!";
  }
  if (message.indexOf("ALERT:FIRE_LIGHT") >= 0) {
    displayData.lightAlert = true;
    displayData.alertMessage = "FIRE: Light detected!";
  }
}

// =============== DISPLAY FUNCTIONS ================================

void drawHeader() {
  // Draw header background
  tft.fillRect(0, HEADER_Y, TFT_WIDTH, HEADER_H, COLOR_HEADER);

  // Title
  tft.setTextColor(COLOR_TEXT, COLOR_HEADER);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(2);
  tft.drawString("ROBOTER 9", TFT_WIDTH/2, HEADER_Y + 5);

  // Connection status
  tft.setTextSize(1);
  if (displayData.loraConnected) {
    tft.setTextColor(COLOR_GOOD, COLOR_HEADER);
    tft.drawString("CONNECTED", TFT_WIDTH/2, HEADER_Y + 25);
  } else {
    tft.setTextColor(COLOR_BAD, COLOR_HEADER);
    tft.drawString("NO SIGNAL", TFT_WIDTH/2, HEADER_Y + 25);
  }

  // Uptime (top right)
  tft.setTextColor(COLOR_TEXT, COLOR_HEADER);
  tft.setTextDatum(TR_DATUM);
  String uptimeStr = formatUptime((millis() / 1000));
  tft.drawString(uptimeStr, TFT_WIDTH - 5, HEADER_Y + 25);
}

void drawStatus() {
  int y = STATUS_Y + 5;
  int lineH = 15;

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  // Section title
  tft.setTextColor(COLOR_LABEL, COLOR_BG);
  tft.drawString("STATUS:", 10, y);
  y += lineH + 5;

  // Sequence
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  String seqStr = "Seq: " + String(displayData.sequence) + "      ";
  tft.drawString(seqStr, 20, y);
  y += lineH;

  // LED state
  String ledStr = "LED: ";
  if (displayData.ledState) {
    tft.setTextColor(COLOR_GOOD, COLOR_BG);
    ledStr += "ON ";
  } else {
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    ledStr += "OFF";
  }
  tft.drawString(ledStr + "     ", 20, y);
  y += lineH;

  // Touch state
  String touchStr = "Touch: ";
  if (displayData.touchState) {
    tft.setTextColor(COLOR_GOOD, COLOR_BG);
    touchStr += "YES";
  } else {
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    touchStr += "NO ";
  }
  tft.drawString(touchStr + "    ", 20, y);
  y += lineH;

  // RSSI
  int rssiColor = getRSSIColor(displayData.rssi);
  tft.setTextColor(rssiColor, COLOR_BG);
  String rssiStr = "RSSI: " + String(displayData.rssi) + " dBm    ";
  tft.drawString(rssiStr, 20, y);
  y += lineH;

  // Battery
  String batStr = "Battery: " + String(displayData.batteryVoltage, 2) + "V (" +
                  String(displayData.batteryPercent) + "%)    ";
  int batColor = displayData.batteryPercent > 20 ? COLOR_GOOD : COLOR_BAD;
  tft.setTextColor(batColor, COLOR_BG);
  tft.drawString(batStr, 20, y);
}

void drawTelemetry() {
  int y = TELEMETRY_Y + 5;
  int lineH = 15;

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  // Section title
  tft.setTextColor(COLOR_LABEL, COLOR_BG);
  tft.drawString("TELEMETRY:", 10, y);
  y += lineH + 5;

  // Uptime
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  String uptimeStr = "Uptime: " + formatUptime(displayData.uptime) + "     ";
  tft.drawString(uptimeStr, 20, y);
  y += lineH;

  // Free heap
  String heapStr = "Heap: " + String(displayData.freeHeap) + " KB    ";
  tft.drawString(heapStr, 20, y);
  y += lineH;

  // Temperature
  String tempStr = "Temp: " + String(displayData.temperature) + " C    ";
  tft.drawString(tempStr, 20, y);
  y += lineH;

  // Loop frequency
  String loopStr = "Loop: " + String(displayData.loopFrequency) + " Hz    ";
  tft.drawString(loopStr, 20, y);
}

void drawAlerts() {
  int y = ALERTS_Y + 5;

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  // Section title
  tft.setTextColor(COLOR_LABEL, COLOR_BG);
  tft.drawString("ALERTS:", 10, y);
  y += 20;

  // Check for alerts
  if (displayData.audioAlert || displayData.lightAlert) {
    // FIRE ALERT!
    tft.fillRect(10, y, TFT_WIDTH - 20, 30, COLOR_ALERT);
    tft.setTextColor(COLOR_BAD, COLOR_ALERT);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString("FIRE ALARM!", TFT_WIDTH/2, y + 15);
  } else {
    // No alerts
    tft.setTextColor(COLOR_GOOD, COLOR_BG);
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(1);
    tft.drawString("No alerts            ", 20, y);
  }
}

void drawFooter() {
  int y = FOOTER_Y;

  tft.fillRect(0, y, TFT_WIDTH, FOOTER_H, COLOR_BG);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_LABEL, COLOR_BG);

  String footerStr = "Packets: " + String(displayData.packetsReceived);
  tft.drawString(footerStr, TFT_WIDTH/2, y + 5);
}

// =============== UTILITY FUNCTIONS ================================

String formatUptime(unsigned long seconds) {
  if (seconds < 60) {
    return String(seconds) + "s";
  } else if (seconds < 3600) {
    int mins = seconds / 60;
    return String(mins) + "m";
  } else {
    int hours = seconds / 3600;
    int mins = (seconds % 3600) / 60;
    return String(hours) + "h" + String(mins) + "m";
  }
}

int getRSSIColor(int rssi) {
  if (rssi > -80) return COLOR_GOOD;       // Excellent
  if (rssi > -100) return COLOR_WARN;      // Fair
  return COLOR_BAD;                        // Poor
}
