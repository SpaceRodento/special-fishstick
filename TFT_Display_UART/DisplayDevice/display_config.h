/*=====================================================================
  display_config.h - ESP32-2432S022 TFT Display Configuration

  LovyanGFX configuration for 2.4" ST7789 TFT (240x320)
  with 8-bit parallel interface (MCU8080)

  Hardware: ESP32-2432S022
  - Display: ST7789 240x320
  - Interface: 8-bit Parallel
  - Touch: CST820 (I2C) - not used

  Pin Configuration:
  - WR:   GPIO 4
  - RD:   GPIO 2
  - RS:   GPIO 16
  - CS:   GPIO 17
  - D0-7: GPIO 15,13,12,14,27,25,33,32
  - BL:   GPIO 0

  Based on factory example from Makerfabs
=======================================================================*/

#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789 _panel_instance;  // ST7789 controller
  lgfx::Bus_Parallel8 _bus_instance;   // 8-bit parallel interface

public:
  LGFX(void)
  {
    // Configure 8-bit parallel bus
    {
      auto cfg = _bus_instance.config();

      cfg.freq_write = 25000000;    // SPI write frequency (25 MHz)
      cfg.pin_wr = 4;              // Write strobe pin
      cfg.pin_rd = 2;              // Read strobe pin
      cfg.pin_rs = 16;             // Register select (D/C)

      // Data pins D0-D7
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

    // Configure ST7789 panel
    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = 17;             // Chip select
      cfg.pin_rst = -1;            // Reset (not connected, use software reset)
      cfg.pin_busy = -1;           // Busy (not used)

      cfg.panel_width = 240;       // Physical width
      cfg.panel_height = 320;      // Physical height
      cfg.offset_x = 0;            // X offset
      cfg.offset_y = 0;            // Y offset
      cfg.offset_rotation = 0;     // Rotation offset

      cfg.readable = false;        // Can't read pixels back
      cfg.invert = false;          // Color inversion
      cfg.rgb_order = false;       // RGB order
      cfg.dlen_16bit = false;      // 16-bit data length
      cfg.bus_shared = true;       // Bus is shared (important!)

      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
  }
};

#endif // DISPLAY_CONFIG_H
