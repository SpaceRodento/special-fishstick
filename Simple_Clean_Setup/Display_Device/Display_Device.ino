/*
  SIMPLE DISPLAY DEVICE

  Minimaalinen näyttölaite ESP32-2432S022:lle
  - Näyttää vastaanotetun UART-datan
  - Toimii heti ilman konfigurointia
  - Testattu ja varma toimivuus
*/

#include <Arduino.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// TFT Configuration for ESP32-2432S022
class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_Parallel8 _bus_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = 25000000;
      cfg.pin_wr = 4;
      cfg.pin_rd = 2;
      cfg.pin_rs = 16;
      cfg.pin_d0 = 15;
      cfg.pin_d1 = 13;
      cfg.pin_d2 = 12;
      cfg.pin_d3 = 14;
      cfg.pin_d4 = 27;
      cfg.pin_d5 = 25;
      cfg.pin_d6 = 33;
      cfg.pin_d7 = 32;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 17;
      cfg.pin_rst = -1;
      cfg.pin_busy = -1;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.readable = false;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

static LGFX tft;
HardwareSerial DataSerial(1);

// Pins
#define UART_RX_PIN 18
#define UART_TX_PIN 19
#define UART_BAUDRATE 115200
#define BACKLIGHT_PIN 0

// Colors
#define COLOR_BG 0x0000        // Black
#define COLOR_HEADER 0x001F    // Blue
#define COLOR_TEXT 0xFFFF      // White
#define COLOR_GOOD 0x07E0      // Green
#define COLOR_WARN 0xFD20      // Orange

unsigned long lastDataTime = 0;
int messageCount = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Backlight
  pinMode(BACKLIGHT_PIN, OUTPUT);
  analogWrite(BACKLIGHT_PIN, 200);

  // TFT init
  tft.init();
  tft.setRotation(1);  // Landscape
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);

  // Header
  tft.fillRect(0, 0, 320, 30, COLOR_HEADER);
  tft.setTextSize(2);
  tft.setCursor(10, 8);
  tft.print("SIMPLE DISPLAY");

  // UART
  pinMode(UART_RX_PIN, INPUT);
  pinMode(UART_TX_PIN, OUTPUT);
  DataSerial.begin(UART_BAUDRATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  // Status
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  tft.setTextColor(COLOR_GOOD, COLOR_BG);
  tft.println("Ready - Waiting for data...");

  Serial.println("Display ready!");
  lastDataTime = millis();
}

void loop() {
  // Check for incoming data
  if (DataSerial.available()) {
    String msg = DataSerial.readStringUntil('\n');
    msg.trim();

    if (msg.length() > 0) {
      lastDataTime = millis();
      messageCount++;

      // Debug to serial FIRST
      Serial.print("RX [");
      Serial.print(messageCount);
      Serial.print("]: ");
      Serial.println(msg);

      // Parse CSV data: KEY:VALUE,KEY2:VALUE2,...
      tft.fillRect(0, 60, 320, 180, COLOR_BG);  // Clear data area
      yield();  // Feed watchdog after fillRect

      int y = 70;
      int start = 0;
      int itemCount = 0;
      tft.setTextSize(2);

      while (start < msg.length() && itemCount < 8) {  // Limit to 8 items
        yield();  // Feed watchdog in loop!

        int comma = msg.indexOf(',', start);
        if (comma == -1) comma = msg.length();

        String pair = msg.substring(start, comma);
        int colon = pair.indexOf(':');

        if (colon > 0) {
          String key = pair.substring(0, colon);
          String value = pair.substring(colon + 1);

          // Display key:value pair
          tft.setCursor(10, y);
          tft.setTextColor(COLOR_WARN, COLOR_BG);
          tft.print(key);
          tft.print(": ");
          tft.setTextColor(COLOR_TEXT, COLOR_BG);
          tft.println(value);

          y += 20;
          itemCount++;
          if (y > 200) break;  // Don't overflow screen
        }

        start = comma + 1;
      }

      // Show message count
      yield();  // Feed watchdog before more TFT ops
      tft.fillRect(0, 210, 320, 20, COLOR_BG);
      tft.setTextSize(1);
      tft.setCursor(10, 215);
      tft.setTextColor(COLOR_GOOD, COLOR_BG);
      tft.print("Messages: ");
      tft.print(messageCount);
    }
  }

  // Check for timeout (no data for 5 seconds)
  if (millis() - lastDataTime > 5000 && messageCount > 0) {
    tft.fillRect(0, 60, 320, 20, COLOR_BG);
    tft.setTextSize(1);
    tft.setCursor(10, 65);
    tft.setTextColor(COLOR_WARN, COLOR_BG);
    tft.println("NO SIGNAL - Check wiring!");
    lastDataTime = millis();  // Reset to avoid spam
  }

  delay(10);  // Small delay to prevent tight loop
}
