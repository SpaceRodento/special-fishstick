/*=====================================================================
  Universal_Display_TFT.ino - Universal Serial Display Station

  ESP32-2432S022 TFT Display (240x320) - Yleiskäyttöinen näyttölaite

  UART-pohjainen näyttö joka vastaanottaa dataa mistä tahansa ESP32:sta.
  Ei LoRa-riippuvuuksia - vain Serial-yhteys!

  Hardware:
  - ESP32-WROOM-32
  - 2.4" ST7789 TFT (240x320, 8-bit parallel)
  - CST820 Touch (I2C) - valinnainen

  Kytkentä (VAIN 2 JOHTOA!):
  ┌────────────────┐           ┌────────────────┐
  │   Pää-ESP32    │           │  Display-ESP32 │
  │                │           │                │
  │  TX (GPIO 17) ─┼──────────►│─ RX (GPIO 18) │
  │  GND          ─┼───────────│─ GND          │
  └────────────────┘           └────────────────┘

  Protokolla:
  - CSV-muotoinen data
  - Esim: "KEY:VALUE,KEY2:VALUE2,..."
  - Esim: "LED:ON,TEMP:42,RSSI:-78"

  Erikoiskomennot:
  - "CLEAR" = Tyhjennä näyttö
  - "ALERT:message" = Näytä hälytys
  - "STATUS:message" = Päivitä tilaviesti

  Käyttö:
  1. Lataa tämä koodi ESP32-2432S022:een
  2. Kytke TX → RX ja GND → GND
  3. Lähetä dataa pää-ESP32:lta Serial1:llä
  4. Näyttö päivittyy automaattisesti!

  Version: 2.0 (UART-based, LoRa-free)
  Author: Roboter Gruppe 9
  Date: 2025-01-05
=======================================================================*/

#include <Arduino.h>

// LovyanGFX for ESP32-2432S022 TFT display
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "display_config.h"

// =============== UART CONFIGURATION ================================
#define UART_BAUDRATE 115200
#define UART_RX_PIN 18                  // Display RX ← Main TX
#define UART_TX_PIN 19                  // Display TX → Main RX (optional)
#define MAX_MESSAGE_LENGTH 256

// =============== DISPLAY CONFIGURATION ================================
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define BACKLIGHT_PIN 0
#define BACKLIGHT_BRIGHTNESS 200        // 0-255

// =============== UPDATE INTERVALS ================================
#define DISPLAY_UPDATE_INTERVAL 100     // Update display every 100ms
#define DATA_TIMEOUT 5000               // No data timeout (5s)

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
HardwareSerial DataSerial(1);  // Use UART1 for data

// =============== DATA STORAGE ================================
#define MAX_FIELDS 20
struct DataField {
  String key;
  String value;
  unsigned long lastUpdate;
};

DataField fields[MAX_FIELDS];
int fieldCount = 0;

bool dataConnected = false;
unsigned long lastDataTime = 0;
unsigned long packetsReceived = 0;
String alertMessage = "";
bool alertActive = false;

// =============== DISPLAY REGIONS ================================
#define HEADER_Y 0
#define HEADER_H 40
#define DATA_Y 40
#define DATA_H 220
#define ALERT_Y 260
#define ALERT_H 40
#define FOOTER_Y 300
#define FOOTER_H 20

// =============== FUNCTION PROTOTYPES ================================
void updateData();
void parseMessage(String message);
void updateDisplay();
void drawHeader();
void drawData();
void drawAlert();
void drawFooter();
String getFieldValue(String key);
void setFieldValue(String key, String value);
void clearAllFields();

// =============== SETUP ================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  UNIVERSAL SERIAL DISPLAY STATION     ║");
  Serial.println("║  ESP32-2432S022 TFT Display           ║");
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
  tft.drawString("DISPLAY", TFT_WIDTH/2, TFT_HEIGHT/2 - 30);
  tft.drawString("STATION", TFT_WIDTH/2, TFT_HEIGHT/2);
  tft.setTextSize(1);
  tft.drawString("Universal Serial Display", TFT_WIDTH/2, TFT_HEIGHT/2 + 40);
  tft.drawString("Waiting for data...", TFT_WIDTH/2, TFT_HEIGHT/2 + 60);
  delay(2000);

  // Initialize UART
  Serial.println("📡 Initializing UART...");
  DataSerial.begin(UART_BAUDRATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  Serial.print("  RX: GPIO ");
  Serial.println(UART_RX_PIN);
  Serial.print("  Baudrate: ");
  Serial.println(UART_BAUDRATE);

  // Clear screen and draw initial layout
  tft.fillScreen(COLOR_BG);
  drawHeader();
  drawData();
  drawAlert();
  drawFooter();

  Serial.println("✓ Display station ready!\n");
  Serial.println("Waiting for serial data...");
  Serial.println("Format: KEY:VALUE,KEY2:VALUE2,...\n");

  lastDataTime = millis();
}

// =============== MAIN LOOP ================================
void loop() {
  static unsigned long lastDisplayUpdate = 0;
  unsigned long now = millis();

  // Check for incoming data
  updateData();

  // Update display periodically
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    updateDisplay();
    lastDisplayUpdate = now;
  }

  // Check for timeout
  if (now - lastDataTime > DATA_TIMEOUT) {
    if (dataConnected) {
      dataConnected = false;
      Serial.println("⚠️  Data connection timeout!");
      drawHeader();  // Update connection indicator
    }
  }

  delay(10);
}

// =============== DATA FUNCTIONS ================================

void updateData() {
  if (DataSerial.available()) {
    String message = DataSerial.readStringUntil('\n');
    message.trim();

    if (message.length() > 0) {
      packetsReceived++;
      lastDataTime = millis();
      dataConnected = true;

      // Debug
      Serial.print("📨 RX: ");
      Serial.println(message);

      // Parse and update fields
      parseMessage(message);
    }
  }
}

void parseMessage(String message) {
  // Special commands
  if (message == "CLEAR") {
    clearAllFields();
    Serial.println("✓ Display cleared");
    return;
  }

  if (message.startsWith("ALERT:")) {
    alertMessage = message.substring(6);
    alertActive = true;
    Serial.print("🚨 Alert: ");
    Serial.println(alertMessage);
    return;
  }

  if (message == "CLEAR_ALERT") {
    alertActive = false;
    alertMessage = "";
    return;
  }

  // Parse CSV format: KEY:VALUE,KEY2:VALUE2,...
  int startPos = 0;
  while (startPos < message.length()) {
    int colonPos = message.indexOf(':', startPos);
    if (colonPos < 0) break;

    int commaPos = message.indexOf(',', colonPos);
    if (commaPos < 0) commaPos = message.length();

    String key = message.substring(startPos, colonPos);
    String value = message.substring(colonPos + 1, commaPos);

    key.trim();
    value.trim();

    if (key.length() > 0) {
      setFieldValue(key, value);
    }

    startPos = commaPos + 1;
  }
}

String getFieldValue(String key) {
  for (int i = 0; i < fieldCount; i++) {
    if (fields[i].key == key) {
      return fields[i].value;
    }
  }
  return "";
}

void setFieldValue(String key, String value) {
  // Update existing field
  for (int i = 0; i < fieldCount; i++) {
    if (fields[i].key == key) {
      fields[i].value = value;
      fields[i].lastUpdate = millis();
      return;
    }
  }

  // Add new field
  if (fieldCount < MAX_FIELDS) {
    fields[fieldCount].key = key;
    fields[fieldCount].value = value;
    fields[fieldCount].lastUpdate = millis();
    fieldCount++;
  }
}

void clearAllFields() {
  fieldCount = 0;
  alertActive = false;
  alertMessage = "";
  tft.fillRect(0, DATA_Y, TFT_WIDTH, DATA_H, COLOR_BG);
  tft.fillRect(0, ALERT_Y, TFT_WIDTH, ALERT_H, COLOR_BG);
}

// =============== DISPLAY FUNCTIONS ================================

void updateDisplay() {
  drawHeader();
  drawData();
  drawAlert();
  drawFooter();
}

void drawHeader() {
  // Draw header background
  tft.fillRect(0, HEADER_Y, TFT_WIDTH, HEADER_H, COLOR_HEADER);

  // Title
  tft.setTextColor(COLOR_TEXT, COLOR_HEADER);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(2);
  tft.drawString("DISPLAY", TFT_WIDTH/2, HEADER_Y + 5);

  // Connection status
  tft.setTextSize(1);
  if (dataConnected) {
    tft.setTextColor(COLOR_GOOD, COLOR_HEADER);
    tft.drawString("CONNECTED", TFT_WIDTH/2, HEADER_Y + 25);
  } else {
    tft.setTextColor(COLOR_BAD, COLOR_HEADER);
    tft.drawString("NO SIGNAL", TFT_WIDTH/2, HEADER_Y + 25);
  }

  // Uptime (top right)
  tft.setTextColor(COLOR_TEXT, COLOR_HEADER);
  tft.setTextDatum(TR_DATUM);
  unsigned long seconds = millis() / 1000;
  String uptimeStr;
  if (seconds < 60) {
    uptimeStr = String(seconds) + "s";
  } else if (seconds < 3600) {
    uptimeStr = String(seconds / 60) + "m";
  } else {
    uptimeStr = String(seconds / 3600) + "h" + String((seconds % 3600) / 60) + "m";
  }
  tft.drawString(uptimeStr, TFT_WIDTH - 5, HEADER_Y + 25);
}

void drawData() {
  int y = DATA_Y + 5;
  int lineH = 18;
  int maxLines = (DATA_H - 10) / lineH;

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  // Draw all fields
  for (int i = 0; i < fieldCount && i < maxLines; i++) {
    // Key (label)
    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    String label = fields[i].key + ":";
    tft.drawString(label, 10, y);

    // Value
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    String value = fields[i].value + "        ";  // Padding to clear old text
    tft.drawString(value, 10 + label.length() * 6 + 5, y);

    y += lineH;
  }

  // Clear remaining area
  if (fieldCount < maxLines) {
    int clearY = y;
    int clearH = DATA_Y + DATA_H - clearY;
    if (clearH > 0) {
      tft.fillRect(0, clearY, TFT_WIDTH, clearH, COLOR_BG);
    }
  }
}

void drawAlert() {
  tft.fillRect(0, ALERT_Y, TFT_WIDTH, ALERT_H, COLOR_BG);

  if (alertActive && alertMessage.length() > 0) {
    // Draw alert box
    tft.fillRect(5, ALERT_Y + 5, TFT_WIDTH - 10, ALERT_H - 10, COLOR_ALERT);

    // Draw alert text
    tft.setTextColor(COLOR_BAD, COLOR_ALERT);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);

    // Truncate if too long
    String displayText = alertMessage;
    if (displayText.length() > 30) {
      displayText = displayText.substring(0, 27) + "...";
    }

    tft.drawString(displayText, TFT_WIDTH/2, ALERT_Y + ALERT_H/2);
  }
}

void drawFooter() {
  tft.fillRect(0, FOOTER_Y, TFT_WIDTH, FOOTER_H, COLOR_BG);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_LABEL, COLOR_BG);

  String footerStr = "Pkts: " + String(packetsReceived);
  tft.drawString(footerStr, TFT_WIDTH/2, FOOTER_Y + 5);
}
