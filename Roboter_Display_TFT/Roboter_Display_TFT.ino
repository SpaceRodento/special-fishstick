/*=====================================================================
  Roboter_Display_TFT.ino - Roboter Gruppe 9 Näyttölaite

  ESP32-2432S022 TFT Display (240x320) - UART-pohjainen näyttö

  Vastaanottaa dataa UART:n kautta päälaitteelta ja näyttää sen
  vaakatason näytöllä (landscape, 320x240).

  Hardware:
  - ESP32-WROOM-32
  - 2.4" ST7789 TFT (240x320 → landscape 320x240)
  - 8-bit parallel interface

  VIRRANSYÖTTÖ:
  - Display-ESP32 syötetään OMASTA USB-kaapelista tai 5V lähteestä
  - EI syötetä virta toiselta ESP32:lta!

  UART-kytkentä (VAIN 2 JOHTOA!):
  ┌────────────────┐           ┌────────────────────────────┐
  │  Robot ESP32   │           │  Display ESP32-2432S022    │
  │                │           │  (syötetty omasta USB:sta) │
  │  TX (GPIO 23) ─┼──────────►│─ RX (GPIO 18)             │
  │  GND          ─┼───────────│─ GND (signaalin referenssi)│
  └────────────────┘           └────────────────────────────┘

  Protokolla (CSV):
  - "KEY:VALUE,KEY2:VALUE2,..."
  - Esim: "LED:ON,TEMP:42,RSSI:-78"

  Erikoiskomennot:
  - "CLEAR" = Tyhjennä kaikki kentät
  - "ALERT:viesti" = Näytä hälytys
  - "CLEARALERT" = Poista hälytys

  Version: 2.0 (UART, Landscape)
  Author: Roboter Gruppe 9
  Date: 2025-11-06
=======================================================================*/

#include <Arduino.h>

// LovyanGFX for ESP32-2432S022 TFT display
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "display_config.h"

// =============== UART CONFIGURATION ================================
// ⚠️  CRITICAL: ESP32-2432S022 physical RX connector uses UART0 (GPIO 3)!
// The 4-pin physical connector is hardwired to UART0, not configurable
#define UART_BAUDRATE 115200
// NOTE: Using Serial (UART0), not HardwareSerial(1)
// Physical pins: RX = GPIO 3, TX = GPIO 1 (hardwired on board)

// =============== DISPLAY CONFIGURATION ================================
// Landscape mode: 320x240 (rotated from 240x320)
#define TFT_WIDTH 320
#define TFT_HEIGHT 240
#define BACKLIGHT_PIN 0
#define BACKLIGHT_BRIGHTNESS 200        // 0-255

// =============== UPDATE INTERVALS ================================
#define DISPLAY_UPDATE_INTERVAL 500     // Update display every 500ms (reduced from 100ms to avoid flicker)
#define DATA_TIMEOUT 5000               // No data timeout (5s)

// =============== COLORS ================================
#define COLOR_BG 0x0000          // Black
#define COLOR_HEADER 0x001F      // Blue
#define COLOR_TEXT 0xFFFF        // White
#define COLOR_LABEL 0x8410       // Gray
#define COLOR_GOOD 0x07E0        // Green
#define COLOR_WARN 0xFD20        // Orange
#define COLOR_BAD 0xF800         // Red
#define COLOR_ALERT_BG 0xF800    // Red background
#define COLOR_ALERT_TEXT 0xFFFF  // White text

// =============== GLOBAL OBJECTS ================================
static LGFX tft;
// ⚠️  Use Serial (UART0) for physical RX connector, not HardwareSerial(1)!

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

// LoRa connection status (extracted from incoming data)
String loraConnectionState = "UNKNOWN";  // OK, WEAK, LOST, UNKNOWN

// =============== DISPLAY REGIONS (Landscape 320x240) ===============
#define HEADER_Y 0
#define HEADER_H 30
#define DATA_Y 30
#define DATA_H 180
#define ALERT_Y 210
#define ALERT_H 30

// =============== FUNCTION PROTOTYPES ================================
void parseMessage(String message);
void updateDisplay();
void drawHeader();
void drawData();
void drawAlert();
String getFieldValue(String key);
void setFieldValue(String key, String value);
void clearAllFields();

// =============== SETUP ================================
void setup() {
  // ⚠️  Initialize Serial FIRST (used for both USB debug and physical RX)
  // Physical RX connector on ESP32-2432S022 is hardwired to UART0 (GPIO 3)
  Serial.begin(UART_BAUDRATE);
  delay(100);

  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  ROBOTER GRUPPE 9 - DISPLAY STATION   ║");
  Serial.println("║  ESP32-2432S022 TFT (UART0/GPIO3)     ║");
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
  tft.setTextSize(2);
  tft.drawString("ROBOTER 9", TFT_WIDTH/2, TFT_HEIGHT/2 - 20);
  tft.setTextSize(1);
  tft.drawString("Display Station", TFT_WIDTH/2, TFT_HEIGHT/2 + 10);
  tft.drawString("Waiting for data...", TFT_WIDTH/2, TFT_HEIGHT/2 + 30);
  delay(2000);

  // UART is already initialized (Serial.begin() at top of setup)
  // Physical RX connector uses UART0 (GPIO 3), no need for separate init
  Serial.println("📡 UART ready (physical RX on GPIO 3)");
  Serial.print("  Baudrate: ");
  Serial.println(UART_BAUDRATE);
  Serial.println("  Waiting for data from Robot ESP32...");

  // Clear display and show initial layout
  tft.fillScreen(COLOR_BG);
  drawHeader();

  Serial.println("\n✅ Display station ready!");
  Serial.println("📊 Send data in CSV format: KEY:VALUE,KEY2:VALUE2,...\n");
}

// =============== LOOP ================================
void loop() {
  static unsigned long lastUpdate = 0;

  // Read UART data from Serial (UART0, physical RX GPIO 3)
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    message.trim();

    if (message.length() > 0) {
      packetsReceived++;
      lastDataTime = millis();
      dataConnected = true;

      // Note: This will also print to USB Serial Monitor
      // (mixed with debug messages)
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

  if (message == "CLEARALERT") {
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
  drawAlert();
}

void drawHeader() {
  // Header background
  tft.fillRect(0, HEADER_Y, TFT_WIDTH, HEADER_H, COLOR_HEADER);

  // Title
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("ROBOTER 9", 5, HEADER_Y + 7);

  // TFT (UART) Connection indicator (top right)
  tft.setTextDatum(TR_DATUM);
  tft.setTextSize(1);
  if (dataConnected) {
    tft.setTextColor(COLOR_GOOD);
    tft.drawString("TFT (UART) ON", TFT_WIDTH - 5, HEADER_Y + 3);
  } else {
    tft.setTextColor(COLOR_BAD);
    tft.drawString("TFT (UART) OFF", TFT_WIDTH - 5, HEADER_Y + 3);
  }

  // Data status (below UART indicator)
  if (dataConnected) {
    tft.setTextColor(COLOR_GOOD);
    tft.drawString("DATA ONLINE", TFT_WIDTH - 5, HEADER_Y + 13);
  } else {
    tft.setTextColor(COLOR_LABEL);
    tft.drawString("WAITING", TFT_WIDTH - 5, HEADER_Y + 13);
  }

  // Packet counter (bottom right)
  tft.setTextColor(COLOR_LABEL);
  String pktStr = "PKT:" + String(packetsReceived);
  tft.drawString(pktStr, TFT_WIDTH - 5, HEADER_Y + 23);
}

void drawData() {
  // Clear data area
  tft.fillRect(0, DATA_Y, TFT_WIDTH, DATA_H, COLOR_BG);

  if (fieldCount == 0) {
    // No data yet
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    tft.setTextColor(COLOR_LABEL);
    tft.drawString("No data received", TFT_WIDTH/2, DATA_Y + DATA_H/2);
    return;
  }

  // Display fields in grid layout (landscape optimized)
  // 2 columns, compact layout for 320x240
  int x = 5;
  int y = DATA_Y + 5;
  int colWidth = (TFT_WIDTH - 15) / 2;  // 2 columns
  int lineHeight = 20;
  int col = 0;

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  for (int i = 0; i < fieldCount && i < 16; i++) {  // Max 16 fields (8 per column)
    int xPos = x + (col * colWidth);
    int yPos = y + ((i / 2) * lineHeight);

    // Draw key (label)
    tft.setTextColor(COLOR_LABEL);
    String keyStr = fields[i].key + ":";
    tft.drawString(keyStr, xPos, yPos);

    // Draw value
    tft.setTextColor(COLOR_TEXT);
    int valueX = xPos + tft.textWidth(keyStr) + 3;
    tft.drawString(fields[i].value, valueX, yPos);

    // Alternate columns
    col = (col + 1) % 2;
  }
}

void drawAlert() {
  // Show LoRa connection status (replaces old alert system)
  // Extract ConnState from fields (set by parseMessage)
  String connState = getFieldValue("ConnState");
  if (connState.length() == 0) {
    connState = loraConnectionState;  // Use global if not in fields
  }

  // Update global state
  loraConnectionState = connState;

  // Determine color and text based on connection state
  uint16_t bgColor = COLOR_BG;
  String statusText = "ROBOTER 9";

  if (connState == "OK" || connState == "CONNECTED") {
    bgColor = COLOR_GOOD;  // Green background
    statusText = "ROBOTER 9 - LoRa ONLINE";
  } else if (connState == "WEAK") {
    bgColor = COLOR_WARN;  // Orange background
    statusText = "ROBOTER 9 - LoRa WEAK";
  } else if (connState == "LOST") {
    bgColor = COLOR_BAD;   // Red background
    statusText = "ROBOTER 9 - LoRa OFFLINE";
  } else {
    // UNKNOWN or CONNECTING
    bgColor = COLOR_LABEL;  // Gray background
    statusText = "ROBOTER 9 - LoRa " + connState;
  }

  // Draw background
  tft.fillRect(0, ALERT_Y, TFT_WIDTH, ALERT_H, bgColor);

  // Draw status text (centered, no blinking)
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString(statusText, TFT_WIDTH/2, ALERT_Y + ALERT_H/2);
}
