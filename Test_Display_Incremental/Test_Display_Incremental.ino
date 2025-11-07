/*
  INCREMENTAL UART TEST

  Lisää UART:n vaiheittain TFT:n päälle.
  Jokainen vaihe tulostaa Serial.println() jotta näemme missä kaatuu.

  Jos näyttö menee valkoiseksi, katso Serial Monitor missä kohtaa se pysähtyy!
*/

#include <Arduino.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Simple TFT config
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

#define UART_RX_PIN 18
#define UART_TX_PIN 19
#define UART_BAUDRATE 115200

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=== INCREMENTAL UART TEST ===");
  Serial.println("Step 1: Starting...");

  // Step 2: Backlight
  Serial.println("Step 2: Backlight...");
  pinMode(0, OUTPUT);
  analogWrite(0, 200);
  Serial.println("  OK");

  // Step 3: TFT init
  Serial.println("Step 3: TFT init...");
  tft.init();
  Serial.println("  OK");

  // Step 4: TFT setup
  Serial.println("Step 4: TFT setup...");
  tft.setRotation(1);
  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.setTextSize(2);
  Serial.println("  OK");

  // Step 5: Draw text
  Serial.println("Step 5: Draw text...");
  tft.setCursor(10, 10);
  tft.println("UART TEST");
  Serial.println("  OK");

  // Step 6: pinMode for UART (TÄMÄ SAATTAA KAATAA!)
  Serial.println("Step 6: pinMode for UART...");
  pinMode(UART_RX_PIN, INPUT);
  pinMode(UART_TX_PIN, OUTPUT);
  Serial.println("  OK");

  // Step 7: UART begin (TÄMÄ SAATTAA KAATAA!)
  Serial.println("Step 7: UART begin...");
  DataSerial.begin(UART_BAUDRATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  Serial.println("  OK");

  // Step 8: Test read
  Serial.println("Step 8: Waiting for UART data...");
  tft.setCursor(10, 50);
  tft.println("Waiting...");
  Serial.println("  OK");

  Serial.println("\n=== ALL STEPS COMPLETED ===");
  Serial.println("If you see this, UART init worked!");
  Serial.println("Now send data from robot...\n");
}

void loop() {
  // Try to read UART
  if (DataSerial.available()) {
    String msg = DataSerial.readStringUntil('\n');
    msg.trim();

    if (msg.length() > 0) {
      Serial.print("RX: ");
      Serial.println(msg);

      // Show on TFT
      tft.fillRect(0, 100, 320, 30, 0x0000);  // Clear area
      tft.setCursor(10, 100);
      tft.setTextColor(0x07E0, 0x0000);  // Green
      tft.print("RX: ");
      tft.print(msg);
    }
  }

  delay(10);
}
