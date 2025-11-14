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
// Kolme fonttikokoa helposti muokattaviksi
#define FONT_SMALL 1     // Pienet tiedot, labelit (8px)
#define FONT_NORMAL 2    // Pääteksti, luettava data (16px)
#define FONT_LARGE 4     // Isot numerot, otsikot (32px)

// =============== UPDATE INTERVALS ================================
#define DISPLAY_UPDATE_INTERVAL 500     // Näytön päivitys 500ms välein
#define DATA_TIMEOUT 5000               // Data-yhteyden timeout (5s)

// =============== VÄRIPALETTI ================================
// Helposti muokattavat värit - voit vaihtaa RGB565-arvot tarpeen mukaan
// RGB565 laskin: https://rgbcolorpicker.com/565

// Perusvärit
#define COLOR_BG           0x0000    // Musta tausta
#define COLOR_PRIMARY      0x001F    // Sininen (pääväri, header)
#define COLOR_SECONDARY    0xFD20    // Oranssi (toissijainen, aktiivinen tila)
#define COLOR_TEXT_PRIMARY 0xFFFF    // Valkoinen teksti
#define COLOR_TEXT_SECONDARY 0x8410  // Harmaa teksti (labelit)

// Tilaindikaattorit
#define COLOR_SUCCESS      0x07E0    // Vihreä (hyvä signaali, yhteys OK)
#define COLOR_WARNING      0xFD20    // Oranssi (varoitus, heikko signaali)
#define COLOR_ERROR        0xF800    // Punainen (virhe, yhteys poikki)

// Vanhemmat nimitykset (yhteensopivuus)
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

// Pakettihäviön seuranta (packet loss tracking)
int lastReceivedSeq = -1;       // Viimeisin vastaanotettu sekvenssinnumero
int expectedSeq = 0;            // Odotettu seuraava sekvenssinnumero
int totalPacketsExpected = 0;   // Odotetut paketit yhteensä
int totalPacketsReceived = 0;   // Vastaanotetut paketit yhteensä
int totalPacketsLost = 0;       // Menetetyt paketit yhteensä
float packetLossPercent = 0.0;  // Pakettihäviöprosentti

// Aikaleiman tallennus
unsigned long lastPacketTime = 0;  // Milloin viimeisin paketti saapui
String currentTimestamp = "00:00"; // Nykyinen aikaleima (esim. "12:34")

// =============== NÄYTÖN LAYOUT (Landscape 320x240) ===============
// Näyttö jaettu kolmeen pääosaan: YLÄ, KESKI, ALA
//
// ┌────────────────────────────────────────┐
// │  YLÄ-OSA (Header) - 30px               │  Otsikko, LED, yhteys
// ├──────────────────┬─────────────────────┤
// │                  │                     │
// │  VASEN SARAKE    │   OIKEA SARAKE      │  Data jaettu kahtia
// │  - Aikaleima     │   - RSSI, SNR       │
// │  - Paketit       │   - Pakettihäviö    │  + Signaalipalkki
// │  - Aika viime    │   - Laatuprosentti  │    oikealla
// │                  │                     │
// │  KESKI-OSA (Data) - 150px              │
// ├────────────────────────────────────────┤
// │  ALA-OSA (Footer) - 60px               │  LoRa ONLINE + tiedot
// │  LoRa ONLINE                           │
// │  -85dBm | Addr:1 | RX                  │
// └────────────────────────────────────────┘

// Ylä-osa (Header)
#define HEADER_Y        0
#define HEADER_H        30

// Keski-osa (Data) - jaettu vasempaan ja oikeaan
#define DATA_Y          30
#define DATA_H          150
#define DATA_LEFT_X     10              // Vasen sarake alkaa
#define DATA_LEFT_W     130             // Vasen sarake leveys
#define DATA_RIGHT_X    150             // Oikea sarake alkaa
#define DATA_RIGHT_W    120             // Oikea sarake leveys (ilman signaalipalkkia)

// Ala-osa (Footer)
#define FOOTER_Y        180
#define FOOTER_H        60

// Signaalipalkki (oikeassa reunassa)
#define SIGNAL_BAR_X    280
#define SIGNAL_BAR_Y    (DATA_Y + 10)
#define SIGNAL_BAR_W    30
#define SIGNAL_BAR_H    (DATA_H - 20)

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
  int receivedSeq = -1;

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

      // Tallenna sekvenssinnumero pakettihäviön laskentaa varten
      if (key == "SEQ") {
        receivedSeq = value.toInt();
      }
    }

    startPos = commaPos + 1;
  }

  // Laske pakettihäviö sekvenssinnumeroiden perusteella
  if (receivedSeq >= 0) {
    if (lastReceivedSeq == -1) {
      // Ensimmäinen paketti
      lastReceivedSeq = receivedSeq;
      expectedSeq = receivedSeq + 1;
      totalPacketsReceived = 1;
      totalPacketsExpected = 1;
    } else {
      // Laske kuinka monta pakettia odotettiin
      int packetsExpectedSinceLastn = receivedSeq - lastReceivedSeq;

      if (packetsExpectedSinceLastn > 0) {
        totalPacketsExpected += packetsExpectedSinceLastn;
        totalPacketsReceived += 1;  // Saimme yhden paketin

        // Jos sekvenssinnumero hyppäsi, paketit puuttuvat
        if (packetsExpectedSinceLastn > 1) {
          int lostPackets = packetsExpectedSinceLastn - 1;
          totalPacketsLost += lostPackets;
          Serial.print("⚠️  Lost packets detected: ");
          Serial.print(lostPackets);
          Serial.print(" (SEQ ");
          Serial.print(lastReceivedSeq + 1);
          Serial.print(" to ");
          Serial.print(receivedSeq - 1);
          Serial.println(")");
        }

        lastReceivedSeq = receivedSeq;
        expectedSeq = receivedSeq + 1;
      }
    }

    // Laske pakettihäviöprosentti
    if (totalPacketsExpected > 0) {
      packetLossPercent = (float)totalPacketsLost / (float)totalPacketsExpected * 100.0;
    }
  }

  // Päivitä aikaleima
  lastPacketTime = millis();
  unsigned long seconds = millis() / 1000;
  int minutes = (seconds / 60) % 60;
  int secs = seconds % 60;
  currentTimestamp = String(minutes) + ":" + (secs < 10 ? "0" : "") + String(secs);
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
  // Tyhjennä data-alue (jätä tilaa signaalipalkilleVaraa oikealla)
  #if SIGNAL_TESTING_MODE
  int dataWidth = SIGNAL_BAR_X - 5;  // Jätä tilaa signaalipalkilleile
  #else
  int dataWidth = TFT_WIDTH;
  #endif
  tft.fillRect(0, DATA_Y, dataWidth, DATA_H, COLOR_BG);

  if (fieldCount == 0) {
    // Ei dataa vielä
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(FONT_SMALL);
    tft.setTextColor(COLOR_LABEL);
    tft.drawString("Ei dataa", dataWidth/2, DATA_Y + DATA_H/2);
    return;
  }

  // Hae tärkeät kentät
  String rssiStr = getFieldValue("RSSI");
  String snrStr = getFieldValue("SNR");
  String loraPkts = getFieldValue("LoRaPkts");
  String seqStr = getFieldValue("SEQ");

  // Laske aika viimeisestä paketista
  unsigned long timeSinceLastPacket = (millis() - lastPacketTime) / 1000;  // sekunteja
  String timeSinceStr = String(timeSinceLastPacket) + "s";

  // ═══════════════════════════════════════════════════════════
  // VASEN SARAKE - Aikaleima, paketit, aika viimeisestä
  // ═══════════════════════════════════════════════════════════
  tft.setTextDatum(TL_DATUM);
  int leftX = DATA_LEFT_X;
  int leftY = DATA_Y + 15;
  int lineHeight = 25;

  // Aikaleima (FONT_NORMAL)
  tft.setTextSize(FONT_NORMAL);
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("Aika:", leftX, leftY);
  tft.setTextColor(COLOR_TEXT_PRIMARY);
  tft.drawString(currentTimestamp, leftX + 70, leftY);
  leftY += lineHeight;

  // Paketit (FONT_NORMAL)
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("Paketit:", leftX, leftY);
  tft.setTextColor(COLOR_TEXT_PRIMARY);
  String pktsStr = loraPkts.length() > 0 ? loraPkts : "0";
  tft.drawString(pktsStr, leftX + 70, leftY);
  leftY += lineHeight;

  // Sekvenssinnumero (FONT_SMALL)
  tft.setTextSize(FONT_SMALL);
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("SEQ:", leftX, leftY);
  tft.setTextColor(COLOR_TEXT_PRIMARY);
  tft.drawString(seqStr.length() > 0 ? seqStr : "-", leftX + 35, leftY);
  leftY += 18;

  // Aika viime paketista (FONT_SMALL)
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("Viime:", leftX, leftY);
  tft.setTextColor(timeSinceLastPacket > 5 ? COLOR_WARNING : COLOR_TEXT_PRIMARY);
  tft.drawString(timeSinceStr, leftX + 40, leftY);
  leftY += 18;

  // ═══════════════════════════════════════════════════════════
  // OIKEA SARAKE - RSSI, SNR, pakettihäviö
  // ═══════════════════════════════════════════════════════════
  int rightX = DATA_RIGHT_X;
  int rightY = DATA_Y + 15;

  // RSSI (FONT_LARGE - iso numero)
  tft.setTextSize(FONT_LARGE);
  tft.setTextColor(COLOR_TEXT_PRIMARY);
  String rssiValue = rssiStr.length() > 0 ? rssiStr.substring(0, rssiStr.indexOf("d")) : "-";
  tft.drawString(rssiValue, rightX, rightY);

  // dBm (FONT_SMALL - pieni yksikkö)
  tft.setTextSize(FONT_SMALL);
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("dBm", rightX + 55, rightY + 20);
  rightY += 50;

  // SNR (FONT_NORMAL)
  tft.setTextSize(FONT_NORMAL);
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("SNR:", rightX, rightY);
  tft.setTextColor(COLOR_TEXT_PRIMARY);
  String snrValue = snrStr.length() > 0 ? snrStr : "-";
  tft.drawString(snrValue, rightX + 50, rightY);
  rightY += 25;

  // Pakettihäviö (FONT_SMALL)
  tft.setTextSize(FONT_SMALL);
  tft.setTextColor(COLOR_LABEL);
  tft.drawString("Havioi:", rightX, rightY);

  // Väritä pakettihäviöprosentti
  uint16_t lossColor = COLOR_SUCCESS;
  if (packetLossPercent > 10.0) lossColor = COLOR_ERROR;
  else if (packetLossPercent > 2.0) lossColor = COLOR_WARNING;

  tft.setTextColor(lossColor);
  String lossStr = String(packetLossPercent, 1) + "%";
  tft.drawString(lossStr, rightX + 45, rightY);
  rightY += 18;

  // Menetetyt paketit (FONT_SMALL)
  tft.setTextColor(COLOR_LABEL);
  String lostPacketsStr = "(" + String(totalPacketsLost) + "/" + String(totalPacketsExpected) + ")";
  tft.drawString(lostPacketsStr, rightX, rightY);
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
  // ═══════════════════════════════════════════════════════════
  // ALA-OSA (Footer) - LoRa-yhteystila ja -tiedot
  // ═══════════════════════════════════════════════════════════

  // Hae LoRa-tiedot kentistä
  String connState = getFieldValue("ConnState");
  String rssi = getFieldValue("RSSI");
  String mode = getFieldValue("Mode");  // RX tai TX

  if (connState.length() == 0) {
    connState = loraConnectionState;  // Käytä globaalia jos ei kentissä
  }
  loraConnectionState = connState;

  // Parse RSSI-arvo
  String rssiValue = rssi.length() > 0 ? rssi.substring(0, rssi.indexOf("d")) : "-";

  // LoRa-osoite (oletetaan vastaanottaja=1, lähettäjä=2)
  String address = (mode == "RX" || mode == "RECEIVER") ? "1" : "2";

  // Rooli (lyhyt muoto)
  String role = (mode == "RX" || mode == "RECEIVER") ? "RX" : "TX";

  // Määrittele taustaväri yhteyden tilan mukaan:
  // - Harmaa: Ei yhteyttä (UNKNOWN, LOST, CONNECTING)
  // - Oranssi: Yhteys aktiivinen (OK, WEAK)
  uint16_t bgColor = COLOR_LABEL;  // Oletus: Harmaa
  String statusText = "LoRa OFFLINE";

  if (connState == "OK" || connState == "CONNECTED" || connState == "WEAK") {
    bgColor = COLOR_SECONDARY;  // Oranssi - yhteys aktiivinen
    statusText = "LoRa ONLINE";
  } else if (connState == "LOST") {
    bgColor = COLOR_LABEL;  // Harmaa
    statusText = "LoRa LOST";
  } else if (connState == "CONNECT") {
    bgColor = COLOR_LABEL;  // Harmaa
    statusText = "LoRa CONNECTING";
  }

  // Piirrä tausta
  tft.fillRect(0, FOOTER_Y, TFT_WIDTH, FOOTER_H, bgColor);

  // ═══════════════════════════════════════════════════════════
  // YLÄ RIVI: LoRa ONLINE/OFFLINE (FONT_LARGE)
  // ═══════════════════════════════════════════════════════════
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(FONT_LARGE);
  tft.setTextColor(COLOR_TEXT_PRIMARY);

  int x = 10;
  int y = FOOTER_Y + 5;

  tft.drawString(statusText, x, y);

  // ═══════════════════════════════════════════════════════════
  // ALA RIVI: Pienellä fontilla dBm, address, rooli (FONT_SMALL)
  // ═══════════════════════════════════════════════════════════
  tft.setTextSize(FONT_SMALL);
  tft.setTextColor(COLOR_TEXT_PRIMARY);

  x = 10;
  y = FOOTER_Y + 40;

  // dBm
  String detailsLine = rssiValue + "dBm";

  // Address
  detailsLine += " | Addr:" + address;

  // Rooli
  detailsLine += " | " + role;

  tft.drawString(detailsLine, x, y);
}
