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

// =============== DISPLAY MODE ================================
// Signal Testing Mode: Optimized for LoRa signal testing and analysis
// - Shows signal quality bar on right side
// - Emphasizes RSSI/SNR data
// - Compact layout for field testing
// Set to false for normal operation mode (TBD)
#define SIGNAL_TESTING_MODE true

// =============== FONT SIZES ================================
#define FONT_SMALL 1     // Detailed info, labels
#define FONT_NORMAL 2    // Main content, readable data
#define FONT_LARGE 4     // Important numbers, titles

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
#define ALERT_H 40       // Increased from 30 to fit more info

// Signal quality bar (right side)
#define SIGNAL_BAR_X 280
#define SIGNAL_BAR_Y (DATA_Y + 10)
#define SIGNAL_BAR_W 30
#define SIGNAL_BAR_H (DATA_H - 20)

// =============== SIGNAL QUALITY HELPERS ================================
// Based on field testing data:
// RSSI: -40dBm (excellent) to -100dBm (poor)
// SNR: +20dB (excellent) to -10dB (poor)

int calculateSignalQuality(int rssi, int snr) {
  // RSSI contribution: -40 to -100 dBm → 100% to 0%
  int rssiPercent = map(constrain(rssi, -100, -40), -100, -40, 0, 100);

  // SNR contribution: -10 to +20 dB → 0% to 30% bonus
  int snrBonus = map(constrain(snr, -10, 20), -10, 20, 0, 30);

  // Total quality (max 100%)
  int quality = constrain(rssiPercent + snrBonus, 0, 100);
  return quality;
}

uint16_t getSignalQualityColor(int quality) {
  if (quality >= 70) return COLOR_GOOD;   // Green (70-100%)
  if (quality >= 40) return COLOR_WARN;   // Orange (40-69%)
  return COLOR_BAD;                        // Red (0-39%)
}

// =============== FUNCTION PROTOTYPES ================================
void parseMessage(String message);
void updateDisplay();
void drawHeader();
void drawData();
void drawSignalQualityBar();
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
  #if SIGNAL_TESTING_MODE
  drawSignalQualityBar();
  #endif
  drawAlert();
}

void drawHeader() {
  // Header background
  tft.fillRect(0, HEADER_Y, TFT_WIDTH, HEADER_H, COLOR_HEADER);

  // Title with LED indicator
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString("ROBOTER 9", 5, HEADER_Y + 7);

  // LED indicator (blinking circle) - synced with LoRa transmission
  String ledValue = getFieldValue("LED");
  bool ledOn = (ledValue == "ON" || ledValue == "1");
  int ledX = 120;  // Position after title
  int ledY = HEADER_Y + 15;
  int ledRadius = 6;

  // Draw LED circle
  if (ledOn) {
    tft.fillCircle(ledX, ledY, ledRadius, COLOR_BAD);    // Red when ON
    tft.drawCircle(ledX, ledY, ledRadius, COLOR_TEXT);   // White border
  } else {
    tft.fillCircle(ledX, ledY, ledRadius, COLOR_BG);     // Black when OFF
    tft.drawCircle(ledX, ledY, ledRadius, COLOR_LABEL);  // Gray border
  }

  // TFT (UART) Connection indicator (top right)
  tft.setTextDatum(TR_DATUM);
  tft.setTextSize(1);
  if (dataConnected) {
    tft.setTextColor(COLOR_GOOD);
    tft.drawString("UART ON", TFT_WIDTH - 5, HEADER_Y + 3);
  } else {
    tft.setTextColor(COLOR_BAD);
    tft.drawString("UART OFF", TFT_WIDTH - 5, HEADER_Y + 3);
  }

  // Data status (below UART indicator)
  if (dataConnected) {
    tft.setTextColor(COLOR_GOOD);
    tft.drawString("DATA ONLINE", TFT_WIDTH - 5, HEADER_Y + 13);
  } else {
    tft.setTextColor(COLOR_LABEL);
    tft.drawString("WAITING", TFT_WIDTH - 5, HEADER_Y + 13);
  }

  // Packet counter (bottom right) - UART packets received by display
  tft.setTextColor(COLOR_LABEL);
  String pktStr = "PKT:" + String(packetsReceived);
  tft.drawString(pktStr, TFT_WIDTH - 5, HEADER_Y + 23);
}

void drawData() {
  // Clear data area (leave space for signal bar on right if enabled)
  #if SIGNAL_TESTING_MODE
  int dataWidth = SIGNAL_BAR_X - 5;  // Leave space for signal bar
  #else
  int dataWidth = TFT_WIDTH;
  #endif
  tft.fillRect(0, DATA_Y, dataWidth, DATA_H, COLOR_BG);

  if (fieldCount == 0) {
    // No data yet
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(FONT_SMALL);
    tft.setTextColor(COLOR_LABEL);
    tft.drawString("No data received", dataWidth/2, DATA_Y + DATA_H/2);
    return;
  }

  // Get important fields
  String loraPkts = getFieldValue("LoRaPkts");
  String uptime = getFieldValue("Uptime");
  String mode = getFieldValue("Mode");
  String count = getFieldValue("Count");
  String touch = getFieldValue("TOUCH");

  // === MAIN INFO AREA: Normal-sized, readable ===
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(FONT_NORMAL);

  int x = 10;
  int y = DATA_Y + 15;
  int lineHeight = 28;  // Spacing for normal font

  // Mode (RECEIVER/SENDER)
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("Mode:", x, y);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString(mode.length() > 0 ? mode : "?", x + 70, y);
  y += lineHeight;

  // LoRa Packets
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("Packets:", x, y);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString(loraPkts.length() > 0 ? loraPkts : "0", x + 100, y);
  y += lineHeight;

  // Count (total messages)
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("Count:", x, y);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString(count.length() > 0 ? count : "0", x + 90, y);
  y += lineHeight;

  // Touch status
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("Touch:", x, y);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString(touch.length() > 0 ? touch : "?", x + 85, y);
  y += lineHeight;

  // Uptime
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("Uptime:", x, y);
  tft.setTextColor(COLOR_TEXT);
  tft.drawString(uptime.length() > 0 ? uptime : "0s", x + 100, y);
  y += lineHeight;

  // Additional fields (if any)
  tft.setTextSize(FONT_SMALL);
  int smallY = y + 5;
  int fieldNum = 0;

  for (int i = 0; i < fieldCount && fieldNum < 3; i++) {
    // Skip already displayed fields
    if (fields[i].key == "Mode" || fields[i].key == "LoRaPkts" ||
        fields[i].key == "Count" || fields[i].key == "TOUCH" ||
        fields[i].key == "Uptime" || fields[i].key == "ConnState" ||
        fields[i].key == "RSSI" || fields[i].key == "SNR" ||
        fields[i].key == "LED" || fields[i].key == "SEQ") {
      continue;
    }

    tft.setTextColor(COLOR_LABEL);
    String keyStr = fields[i].key + ":";
    tft.drawString(keyStr, x, smallY);
    tft.setTextColor(COLOR_TEXT);
    tft.drawString(fields[i].value, x + 60, smallY);
    smallY += 15;
    fieldNum++;
  }
}

void drawSignalQualityBar() {
  // Signal quality bar on right side (only in signal testing mode)

  // Get signal values
  String rssiStr = getFieldValue("RSSI");
  String snrStr = getFieldValue("SNR");
  String connState = getFieldValue("ConnState");

  // Parse values
  int rssi = (rssiStr.length() > 0) ? rssiStr.substring(0, rssiStr.indexOf("d")).toInt() : -100;
  int snr = (snrStr.length() > 0) ? snrStr.substring(0, snrStr.indexOf("d")).toInt() : -10;

  // Calculate quality (0-100%)
  int quality = calculateSignalQuality(rssi, snr);
  uint16_t barColor = getSignalQualityColor(quality);

  // Draw bar background (black)
  tft.fillRect(SIGNAL_BAR_X, SIGNAL_BAR_Y, SIGNAL_BAR_W, SIGNAL_BAR_H, COLOR_BG);

  // Draw bar border (white)
  tft.drawRect(SIGNAL_BAR_X, SIGNAL_BAR_Y, SIGNAL_BAR_W, SIGNAL_BAR_H, COLOR_TEXT);

  // Draw filled bar (bottom to top based on quality)
  if (quality > 0 && connState.length() > 0 && connState != "UNKNOWN") {
    int fillHeight = (SIGNAL_BAR_H - 4) * quality / 100;
    int fillY = SIGNAL_BAR_Y + SIGNAL_BAR_H - 2 - fillHeight;

    // Animated: slight pulse effect (optional)
    static int pulsePhase = 0;
    pulsePhase = (pulsePhase + 1) % 10;
    if (pulsePhase < 5) fillHeight += 1;  // Subtle animation

    tft.fillRect(SIGNAL_BAR_X + 2, fillY, SIGNAL_BAR_W - 4, fillHeight, barColor);
  }

  // Draw percentage label below bar
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(FONT_SMALL);
  tft.setTextColor(COLOR_LABEL);
  String qualityText = String(quality) + "%";
  tft.drawString(qualityText, SIGNAL_BAR_X + SIGNAL_BAR_W/2, SIGNAL_BAR_Y + SIGNAL_BAR_H + 10);
}

void drawAlert() {
  // LoRa Status Bar - signal quality information
  // Color scheme: Gray (no connection) / Orange (connection active)

  // Extract LoRa data from fields
  String connState = getFieldValue("ConnState");
  String rssi = getFieldValue("RSSI");
  String snr = getFieldValue("SNR");

  if (connState.length() == 0) {
    connState = loraConnectionState;  // Use global if not in fields
  }
  loraConnectionState = connState;

  // Parse RSSI/SNR for quality evaluation
  int rssiValue = (rssi.length() > 0) ? rssi.substring(0, rssi.indexOf("d")).toInt() : -100;
  int snrValue = (snr.length() > 0) ? snr.substring(0, snr.indexOf("d")).toInt() : -10;
  int quality = calculateSignalQuality(rssiValue, snrValue);

  // Determine background color:
  // - Gray: No connection (UNKNOWN, LOST, CONNECTING)
  // - Orange: Connection active (OK, WEAK)
  uint16_t bgColor = COLOR_LABEL;  // Default: Gray
  String statusName = "NO LINK";

  if (connState == "OK" || connState == "CONNECTED" || connState == "WEAK") {
    bgColor = COLOR_WARN;  // Orange - connection active
    statusName = "ACTIVE";
  } else if (connState == "LOST") {
    bgColor = COLOR_LABEL;  // Gray
    statusName = "LOST";
  } else if (connState == "CONNECT") {
    bgColor = COLOR_LABEL;  // Gray
    statusName = "CONNECTING";
  }

  // Draw background
  tft.fillRect(0, ALERT_Y, TFT_WIDTH, ALERT_H, bgColor);

  // === UPPER LINE: Title and status ===
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(FONT_NORMAL);
  tft.setTextColor(COLOR_TEXT);

  int x = 5;
  int y = ALERT_Y + 5;

  // "LoRa:" label
  tft.drawString("LoRa:", x, y);
  x += tft.textWidth("LoRa:") + 8;

  // Status (ACTIVE/NO LINK/etc)
  tft.drawString(statusName, x, y);

  // === LOWER LINE: Signal metrics (normal font) ===
  x = 5;
  y = ALERT_Y + 25;
  tft.setTextSize(FONT_NORMAL);

  // RSSI
  if (rssi.length() > 0) {
    tft.drawString("RSSI:" + rssi, x, y);
    x += tft.textWidth("RSSI:" + rssi) + 12;
  }

  // SNR
  if (snr.length() > 0) {
    tft.drawString("SNR:" + snr, x, y);
    x += tft.textWidth("SNR:" + snr) + 12;
  }

  // Quality percentage
  if (connState == "OK" || connState == "WEAK") {
    tft.drawString("Q:" + String(quality) + "%", x, y);
  }
}
