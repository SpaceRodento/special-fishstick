/*
  SIMPLE DISPLAY DEVICE (UART0 VERSION)

  Uses physical RX/TX connector (GPIO 3/1 = UART0)

  IMPORTANT: Serial Monitor won't work properly since UART0
  is shared between USB and physical connector!
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

// Pins - Physical RX/TX connector uses UART0 (GPIO 3/1)
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
  // Serial (UART0) is used for physical RX/TX connector
  Serial.begin(UART_BAUDRATE);
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

  // Status
  tft.setTextSize(1);
  tft.setCursor(10, 40);
  tft.setTextColor(COLOR_GOOD, COLOR_BG);
  tft.println("Ready - Waiting for data...");
  tft.setCursor(10, 55);
  tft.setTextColor(COLOR_WARN, COLOR_BG);
  tft.println("(Using physical RX connector)");

  lastDataTime = millis();
}

void loop() {
  // Check for incoming data on Serial (UART0 - physical connector)
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();

    if (msg.length() > 0) {
      lastDataTime = millis();
      messageCount++;

      // Parse CSV data: KEY:VALUE,KEY2:VALUE2,...
      tft.fillRect(0, 70, 320, 140, COLOR_BG);  // Clear data area
      yield();  // Feed watchdog after fillRect

      int y = 80;
      int start = 0;
      int itemCount = 0;
      tft.setTextSize(2);

      while (start < msg.length() && itemCount < 7) {  // Limit to 7 items
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
    tft.fillRect(0, 70, 320, 20, COLOR_BG);
    tft.setTextSize(1);
    tft.setCursor(10, 75);
    tft.setTextColor(COLOR_WARN, COLOR_BG);
    tft.println("NO SIGNAL - Check wiring!");
    lastDataTime = millis();  // Reset to avoid spam
  }

  delay(10);  // Small delay to prevent tight loop
}
