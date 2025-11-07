/*
  TFT TESTI - ESP32-2432S022

  Testaa vain TFT-näyttöä ilman UART:a.
  Jos näyttö muuttuu mustaksi ja näkyy teksti, TFT toimii!
*/

#include <Arduino.h>

// LovyanGFX for ESP32-2432S022 TFT display
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Yksinkertainen TFT config (kopioi display_config.h:sta)
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

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=== TFT TEST START ===");

  // Backlight on
  pinMode(0, OUTPUT);
  analogWrite(0, 200);  // GPIO 0 = backlight
  Serial.println("Backlight ON");

  // Initialize TFT
  Serial.println("Initializing TFT...");
  tft.init();
  Serial.println("TFT init OK");

  tft.setRotation(1);  // Landscape
  Serial.println("Rotation set");

  tft.fillScreen(0x0000);  // Black
  Serial.println("Screen filled black");

  tft.setTextColor(0xFFFF, 0x0000);  // White on black
  tft.setTextSize(2);
  Serial.println("Text color set");

  tft.setCursor(10, 10);
  tft.println("TFT TEST");
  Serial.println("Text drawn");

  tft.setCursor(10, 40);
  tft.println("If you see this,");
  tft.setCursor(10, 70);
  tft.println("TFT works!");

  Serial.println("\n=== TFT TEST COMPLETE ===");
  Serial.println("Check display - should show 'TFT TEST'");
}

int counter = 0;

void loop() {
  counter++;

  Serial.print("Counter: ");
  Serial.println(counter);

  // Update display
  tft.setCursor(10, 110);
  tft.setTextColor(0x07E0, 0x0000);  // Green
  tft.print("Count: ");
  tft.print(counter);
  tft.print("  ");

  delay(1000);
}
