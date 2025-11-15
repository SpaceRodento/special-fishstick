/*=====================================================================
  DisplayDevice.ino - Universal UART TFT Display Station

  ESP32-2432S022 TFT Display (240x320) - UART-based universal display

  Receives data via UART from any ESP32 and displays it on the
  landscape TFT screen (320x240).

  Hardware:
  - ESP32-WROOM-32
  - 2.4" ST7789 TFT (240x320 → landscape 320x240)
  - 8-bit parallel interface

  POWER SUPPLY:
  - Display ESP32 must be powered from its OWN USB cable or 5V source
  - DO NOT power from another ESP32!

  UART Connection (ONLY 2 WIRES!):
  ┌────────────────┐           ┌────────────────────────────┐
  │  Sender ESP32  │           │  Display ESP32-2432S022    │
  │                │           │  (powered from own USB)    │
  │  TX (GPIO XX) ─┼──────────►│─ RX (GPIO 3)               │
  │  GND          ─┼───────────│─ GND (signal reference)    │
  └────────────────┘           └────────────────────────────┘

  Protocol (CSV):
  - "KEY:VALUE,KEY2:VALUE2,..."
  - Example: "LED:ON,TEMP:42,Counter:123"

  Special Commands:
  - "CLEAR" = Clear all fields
  - "ALERT:message" = Show alert
  - "CLEAR_ALERT" = Remove alert

  Version: 1.0 (Universal UART)
  Author: SpaceRodento
  Date: 2025-11-15
=======================================================================*/

#include <Arduino.h>

// LovyanGFX for ESP32-2432S022 TFT display
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "display_config.h"

// =============== UART CONFIGURATION ================================
#define UART_BAUDRATE 115200
// NOTE: Using Serial (UART0)
// Physical pins: RX = GPIO 3, TX = GPIO 1 (hardwired on board)

// =============== DISPLAY CONFIGURATION ================================
// Landscape mode: 320x240 (rotated from 240x320)
#define TFT_WIDTH 320
#define TFT_HEIGHT 240
#define BACKLIGHT_PIN 0
#define BACKLIGHT_BRIGHTNESS 200        // 0-255

// =============== FONT SIZES ================================
#define FONT_SMALL 1     // Small text, labels (8px)
#define FONT_NORMAL 2    // Main text, readable data (16px)
#define FONT_LARGE 3     // Large numbers, titles (24px)

// =============== UPDATE INTERVALS ================================
#define DISPLAY_UPDATE_INTERVAL 500     // Display update every 500ms
#define DATA_TIMEOUT 5000               // Data connection timeout (5s)

// =============== COLOR PALETTE ================================
// Easy to modify colors - RGB565 values
// RGB565 calculator: https://rgbcolorpicker.com/565

// Base colors
#define COLOR_BG           0x0000    // Black background
#define COLOR_PRIMARY      0x001F    // Blue (primary color, header)
#define COLOR_SECONDARY    0xFD20    // Orange (secondary, active state)
#define COLOR_TEXT_PRIMARY 0xFFFF    // White text
#define COLOR_TEXT_SECONDARY 0x8410  // Gray text (labels)

// Status indicators
#define COLOR_SUCCESS      0x07E0    // Green (good status, connection OK)
#define COLOR_WARNING      0xFD20    // Orange (warning, weak signal)
#define COLOR_ERROR        0xF800    // Red (error, connection lost)

// Aliases for compatibility
#define COLOR_HEADER       COLOR_PRIMARY
#define COLOR_TEXT         COLOR_TEXT_PRIMARY
#define COLOR_LABEL        COLOR_TEXT_SECONDARY
#define COLOR_GOOD         COLOR_SUCCESS
#define COLOR_WARN         COLOR_WARNING
#define COLOR_BAD          COLOR_ERROR
#define COLOR_ALERT_BG     COLOR_ERROR
#define COLOR_ALERT_TEXT   COLOR_TEXT_PRIMARY

// =============== GLOBAL OBJECTS ================================
static LGFX tft;

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

// Uptime tracking
unsigned long uptimeSeconds = 0;

// =============== DISPLAY LAYOUT (Landscape 320x240) ===============
// Display divided into three sections: HEADER, DATA, FOOTER
//
// ┌────────────────────────────────────────┐
// │  HEADER - 30px                         │  Title, connection status
// ├────────────────────────────────────────┤
// │                                        │
// │  DATA AREA - 180px                     │  Display all received fields
// │  (scrollable list of key:value pairs)  │
// │                                        │
// ├────────────────────────────────────────┤
// │  FOOTER - 30px                         │  Status / Alert area
// └────────────────────────────────────────┘

// Header section
#define HEADER_Y        0
#define HEADER_H        30

// Data section
#define DATA_Y          30
#define DATA_H          180
#define DATA_X          10
#define DATA_W          (TFT_WIDTH - 20)

// Footer section
#define FOOTER_Y        210
#define FOOTER_H        30

// =============== FUNCTION PROTOTYPES ================================
void parseMessage(String message);
void updateDisplay();
void drawHeader();
void drawData();
void drawFooter();
String getFieldValue(String key);
void setFieldValue(String key, String value);
void clearAllFields();

// =============== SETUP ================================
void setup() {
  // Initialize Serial (used for both USB debug and physical RX)
  Serial.begin(UART_BAUDRATE);
  delay(100);

  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  UNIVERSAL TFT DISPLAY STATION        ║");
  Serial.println("║  ESP32-2432S022 (UART/GPIO3)          ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  Serial.println("⚠️  NOTE: Serial Monitor may show mixed data");
  Serial.println("   (Both USB debug + incoming UART data)\n");

  // Initialize backlight
  pinMode(BACKLIGHT_PIN, OUTPUT);
  analogWrite(BACKLIGHT_PIN, BACKLIGHT_BRIGHTNESS);

  // Initialize TFT display
  Serial.println("📺 Initializing TFT display...");
  tft.init();
  tft.setRotation(1);  // Landscape mode (320x240)
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);

  // Show boot screen
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(FONT_LARGE);
  tft.drawString("TFT Display", TFT_WIDTH/2, TFT_HEIGHT/2 - 20);
  tft.setTextSize(FONT_SMALL);
  tft.drawString("UART Display Station", TFT_WIDTH/2, TFT_HEIGHT/2 + 10);
  tft.drawString("Waiting for data...", TFT_WIDTH/2, TFT_HEIGHT/2 + 30);
  delay(2000);

  Serial.println("📡 UART ready (physical RX on GPIO 3)");
  Serial.print("  Baudrate: ");
  Serial.println(UART_BAUDRATE);
  Serial.println("  Waiting for data from sender ESP32...");

  // Clear display and show initial layout
  tft.fillScreen(COLOR_BG);
  drawHeader();
  drawFooter();

  Serial.println("\n✅ Display station ready!");
  Serial.println("📊 Send data in CSV format: KEY:VALUE,KEY2:VALUE2,...\n");
}

// =============== LOOP ================================
void loop() {
  static unsigned long lastUpdate = 0;
  static unsigned long lastUptimeUpdate = 0;

  // Update uptime counter every second
  if (millis() - lastUptimeUpdate >= 1000) {
    lastUptimeUpdate = millis();
    uptimeSeconds++;
  }

  // Read UART data from Serial (UART0, physical RX GPIO 3)
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    message.trim();

    if (message.length() > 0) {
      packetsReceived++;
      lastDataTime = millis();
      dataConnected = true;

      Serial.print("📥 RX [");
      Serial.print(packetsReceived);
      Serial.print("]: ");
      Serial.println(message);

      parseMessage(message);
    }
  }

  // Check data timeout
  if (dataConnected && (millis() - lastDataTime > DATA_TIMEOUT)) {
    dataConnected = false;
    Serial.println("⚠️  Data timeout - no data received for 5 seconds");
  }

  // Update display periodically
  if (millis() - lastUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastUpdate = millis();
    updateDisplay();
  }

  delay(10);
}

// =============== MESSAGE PARSING ================================
void parseMessage(String message) {
  // Handle special commands
  if (message == "CLEAR") {
    clearAllFields();
    Serial.println("🗑️  Cleared all fields");
    return;
  }

  if (message.startsWith("ALERT:")) {
    alertMessage = message.substring(6);
    alertActive = true;
    Serial.print("🚨 ALERT: ");
    Serial.println(alertMessage);
    return;
  }

  if (message == "CLEAR_ALERT") {
    alertActive = false;
    alertMessage = "";
    Serial.println("✅ Alert cleared");
    return;
  }

  // Parse CSV format: KEY:VALUE,KEY2:VALUE2,...
  int startPos = 0;

  while (startPos < message.length()) {
    // Find next comma
    int commaPos = message.indexOf(',', startPos);
    if (commaPos == -1) commaPos = message.length();

    // Extract key:value pair
    String pair = message.substring(startPos, commaPos);
    int colonPos = pair.indexOf(':');

    if (colonPos > 0) {
      String key = pair.substring(0, colonPos);
      String value = pair.substring(colonPos + 1);
      key.trim();
      value.trim();

      setFieldValue(key, value);
    }

    startPos = commaPos + 1;
  }
}

// =============== DATA FIELD MANAGEMENT ================================
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

  // Add new field if space available
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
}

// =============== DISPLAY DRAWING ================================
void updateDisplay() {
  drawHeader();
  drawData();
  drawFooter();
}

void drawHeader() {
  // Header background
  tft.fillRect(0, HEADER_Y, TFT_WIDTH, HEADER_H, COLOR_HEADER);

  // Title
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(FONT_NORMAL);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("TFT Display", 5, HEADER_Y + 7);

  // Connection status (top right)
  tft.setTextDatum(TR_DATUM);
  tft.setTextSize(FONT_SMALL);
  if (dataConnected) {
    tft.setTextColor(COLOR_GOOD);
    tft.drawString("CONNECTED", TFT_WIDTH - 5, HEADER_Y + 3);
  } else {
    tft.setTextColor(COLOR_BAD);
    tft.drawString("NO SIGNAL", TFT_WIDTH - 5, HEADER_Y + 3);
  }

  // Packet counter (below status)
  tft.setTextColor(COLOR_LABEL);
  String pktStr = "PKT:" + String(packetsReceived);
  tft.drawString(pktStr, TFT_WIDTH - 5, HEADER_Y + 15);
}

void drawData() {
  // Clear data area
  tft.fillRect(0, DATA_Y, TFT_WIDTH, DATA_H, COLOR_BG);

  if (fieldCount == 0 && !dataConnected) {
    // No data yet
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(FONT_NORMAL);
    tft.setTextColor(COLOR_LABEL);
    tft.drawString("Waiting for data...", TFT_WIDTH/2, DATA_Y + DATA_H/2);
    return;
  }

  // Display all fields as list
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(FONT_NORMAL);

  int x = DATA_X;
  int y = DATA_Y + 5;
  int lineHeight = 18;
  int maxLines = (DATA_H - 10) / lineHeight;

  // Show most recent fields (up to screen capacity)
  int startIndex = max(0, fieldCount - maxLines);

  for (int i = startIndex; i < fieldCount && i < startIndex + maxLines; i++) {
    // Draw key (label color)
    tft.setTextColor(COLOR_LABEL);
    String keyStr = fields[i].key + ":";
    tft.drawString(keyStr, x, y);

    // Draw value (white text)
    tft.setTextColor(COLOR_TEXT);
    int valueX = x + 100;  // Fixed column for values

    // Truncate long values
    String valueStr = fields[i].value;
    if (valueStr.length() > 20) {
      valueStr = valueStr.substring(0, 17) + "...";
    }
    tft.drawString(valueStr, valueX, y);

    y += lineHeight;
  }

  // If more fields than can fit, show scroll indicator
  if (fieldCount > maxLines) {
    tft.setTextDatum(BR_DATUM);
    tft.setTextSize(FONT_SMALL);
    tft.setTextColor(COLOR_LABEL);
    tft.drawString("...", TFT_WIDTH - 5, DATA_Y + DATA_H - 5);
  }
}

void drawFooter() {
  // Footer background color depends on alert status
  uint16_t bgColor = alertActive ? COLOR_ALERT_BG : COLOR_SECONDARY;
  tft.fillRect(0, FOOTER_Y, TFT_WIDTH, FOOTER_H, bgColor);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(FONT_SMALL);
  tft.setTextColor(COLOR_TEXT_PRIMARY);

  if (alertActive) {
    // Show alert message
    String alertStr = "⚠ " + alertMessage;
    if (alertStr.length() > 40) {
      alertStr = alertStr.substring(0, 37) + "...";
    }
    tft.drawString(alertStr, 5, FOOTER_Y + 5);
  } else {
    // Show uptime
    int hours = uptimeSeconds / 3600;
    int minutes = (uptimeSeconds % 3600) / 60;
    int seconds = uptimeSeconds % 60;

    String uptimeStr = "Uptime: ";
    if (hours > 0) {
      uptimeStr += String(hours) + "h ";
    }
    uptimeStr += String(minutes) + "m " + String(seconds) + "s";

    tft.drawString(uptimeStr, 5, FOOTER_Y + 5);

    // Show data rate on right side
    if (dataConnected) {
      tft.setTextDatum(TR_DATUM);
      unsigned long timeSinceData = (millis() - lastDataTime) / 1000;
      String dataAge = "Last: " + String(timeSinceData) + "s";
      tft.drawString(dataAge, TFT_WIDTH - 5, FOOTER_Y + 5);
    }
  }
}
