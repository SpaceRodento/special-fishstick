/*=====================================================================
  i2c_manager.h - Unified I2C Bus Management

  Keskitetty I2C-väylän hallinta kaikille I2C-laitteille.
  Estää Wire.begin():n turhia kutsuja ja tarjoaa diagnostiikkaa.

  I2C-väylä ESP32:lla:
  - SDA: GPIO 21
  - SCL: GPIO 22
  - Nopeus: 100 kHz (oletuksena)

  Tuetut laitteet:
  - LCD 16x2 (0x27) - Aina päällä vastaanottajalla
  - TCS34725 (0x29) - Värisensori (ENABLE_LIGHT_DETECTION)
  - INA219 (0x40) - Virtamittari (ENABLE_CURRENT_MONITOR)

  Käyttö:
  1. Kutsu ensureI2CInitialized() ennen I2C-laitteiden käyttöä
  2. Kutsu scanI2CBus() diagnostiikkaan (valinnainen)

  Esimerkki:
    #include "i2c_manager.h"

    void setup() {
      ensureI2CInitialized();  // Alustaa I2C:n kerran

      // Nyt voit käyttää I2C-laitteita:
      lcd.init();
      tcs.begin();
      ina219.begin();
    }
=======================================================================*/

#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// I2C-väylän tila
static bool i2cInitialized = false;
static unsigned long i2cInitTime = 0;

// I2C-laiteosoitteet (dokumentointi)
#define I2C_LCD_ADDRESS     0x27  // LCD 16x2 (vastaanottaja)
#define I2C_TCS34725_ADDRESS 0x29  // TCS34725 värisensori
#define I2C_INA219_ADDRESS  0x40  // INA219 virtamittari

/**
 * Alustaa I2C-väylän vain kerran.
 * Turvallinen kutsua useasti - alustaa vain ensimmäisellä kerralla.
 */
void ensureI2CInitialized() {
  if (!i2cInitialized) {
    Wire.begin();  // SDA=21, SCL=22 (ESP32 default)
    i2cInitialized = true;
    i2cInitTime = millis();

    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║    I2C BUS INITIALIZED                 ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println("  SDA: GPIO 21");
    Serial.println("  SCL: GPIO 22");
    Serial.println("  Speed: 100 kHz (default)");
    Serial.println();

    // Listaa mitä laitteita odotetaan
    Serial.println("Expected I2C devices:");
    Serial.println("  - 0x27: LCD 16x2 (receiver only)");

    #if ENABLE_LIGHT_DETECTION
    Serial.println("  - 0x29: TCS34725 RGB sensor");
    #endif

    #if ENABLE_CURRENT_MONITOR
    Serial.println("  - 0x40: INA219 current sensor");
    #endif

    Serial.println();
  }
}

/**
 * Skannaa I2C-väylän ja listaa löydetyt laitteet.
 * Käyttö: Diagnostiikka ja laitteiden tunnistus.
 */
void scanI2CBus() {
  if (!i2cInitialized) {
    Serial.println("⚠️  I2C not initialized! Call ensureI2CInitialized() first.");
    return;
  }

  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║    I2C BUS SCAN                        ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();

  int devicesFound = 0;

  Serial.println("Scanning I2C bus (0x01 - 0x7F)...");
  Serial.println();

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();

    if (error == 0) {
      // Laite löytyi
      devicesFound++;

      Serial.print("✓ Device found at 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      Serial.print("  ");

      // Tunnista laite
      switch (addr) {
        case 0x27:
          Serial.println("(LCD 16x2)");
          break;
        case 0x29:
          Serial.println("(TCS34725 RGB sensor)");
          break;
        case 0x3C:
          Serial.println("(OLED display)");
          break;
        case 0x40:
          Serial.println("(INA219 current sensor)");
          break;
        case 0x48:
          Serial.println("(ADS1115 ADC)");
          break;
        case 0x68:
          Serial.println("(MPU6050 / DS1307 RTC)");
          break;
        case 0x76:
        case 0x77:
          Serial.println("(BME280 / BMP280)");
          break;
        default:
          Serial.println("(Unknown device)");
      }
    }
    else if (error == 4) {
      // Tuntematon virhe
      Serial.print("⚠️  Error at address 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
    }
  }

  Serial.println();

  if (devicesFound == 0) {
    Serial.println("❌ No I2C devices found!");
    Serial.println();
    Serial.println("Troubleshooting:");
    Serial.println("  1. Check wiring (SDA=21, SCL=22, GND, VCC)");
    Serial.println("  2. Verify device has power");
    Serial.println("  3. Check pull-up resistors (usually built-in)");
    Serial.println("  4. Try different I2C address (some devices configurable)");
  } else {
    Serial.print("✓ Found ");
    Serial.print(devicesFound);
    Serial.println(" device(s) on I2C bus.");
  }

  Serial.println();
}

/**
 * Palauttaa onko I2C alustettu.
 */
bool isI2CInitialized() {
  return i2cInitialized;
}

/**
 * Palauttaa milloin I2C alustettiin (millisekunteina).
 */
unsigned long getI2CInitTime() {
  return i2cInitTime;
}

/**
 * Tarkistaa onko tietty I2C-laite saavutettavissa.
 *
 * @param address I2C-osoite (0x00-0x7F)
 * @return true jos laite vastaa, false jos ei
 */
bool isI2CDevicePresent(byte address) {
  if (!i2cInitialized) {
    return false;
  }

  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();

  return (error == 0);
}

/**
 * Tulostaa I2C-väylän diagnostiikkatiedot.
 */
void printI2CDiagnostics() {
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║    I2C DIAGNOSTICS                     ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();

  Serial.print("Initialized: ");
  Serial.println(i2cInitialized ? "YES" : "NO");

  if (i2cInitialized) {
    Serial.print("Init time: ");
    Serial.print(i2cInitTime);
    Serial.println(" ms");

    Serial.print("Uptime: ");
    Serial.print((millis() - i2cInitTime) / 1000);
    Serial.println(" seconds");

    Serial.println();
    Serial.println("Expected devices:");

    // LCD (aina vastaanottajalla)
    Serial.print("  LCD 16x2 (0x27): ");
    Serial.println(isI2CDevicePresent(I2C_LCD_ADDRESS) ? "✓ FOUND" : "❌ NOT FOUND");

    #if ENABLE_LIGHT_DETECTION
    Serial.print("  TCS34725 (0x29): ");
    Serial.println(isI2CDevicePresent(I2C_TCS34725_ADDRESS) ? "✓ FOUND" : "❌ NOT FOUND");
    #endif

    #if ENABLE_CURRENT_MONITOR
    Serial.print("  INA219 (0x40): ");
    Serial.println(isI2CDevicePresent(I2C_INA219_ADDRESS) ? "✓ FOUND" : "❌ NOT FOUND");
    #endif
  }

  Serial.println();
}

#endif // I2C_MANAGER_H
