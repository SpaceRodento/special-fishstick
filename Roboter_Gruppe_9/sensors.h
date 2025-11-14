/*=====================================================================
  sensors.h - Unified Sensor Management (Battery & Current)

  FEATURES 1 & 13: Akkuseuranta ja virtamittaus

  Yhdistetty sensorimoduuli joka tukee kahta akkuseurantamenetelmää:
  1. ADC-pohjainen jännitemittaus (battery_monitor.h)
  2. INA219 I2C virtamittari (current_monitor.h)

  ═══════════════════════════════════════════════════════════════════

  VAIHTOEHTO 1: ADC-pohjainen jännitemittaus (yksinkertainen)

  Laitteisto:
  - Voltage divider (2× 10kΩ) → GPIO 35 (ADC1_CH7)
  - Sopii yksinkertaiseen akkuseurantaan
  - Ei tarvitse lisäkomponentteja

  Kytkentä:
  Akku+ ──[10kΩ]── GPIO 35 ──[10kΩ]── GND

  Ominaisuudet:
  - Jännitemittaus (~3mV tarkkuus)
  - Matala/kriittinen akku varoitukset
  - Min/max tracking
  - Prosenttilaskenta (LiPo 3.0-4.2V)

  Käyttöönotto:
  1. Aseta config.h: ENABLE_BATTERY_MONITOR true
  2. Kytke voltage divider GPIO 35:een
  3. Kalibroi BATTERY_VOLTAGE_DIVIDER tarvittaessa
  4. Tarkista varoitusrajat (LOW: 3.3V, CRITICAL: 3.0V)

  ═══════════════════════════════════════════════════════════════════

  VAIHTOEHTO 2: INA219 I2C virtamittari (tarkka + teho)

  Laitteisto:
  - INA219 moduuli → I2C (SDA=21, SCL=22, osoite 0x40)
  - Sopii tarkkaan seurantaan ja energiankulutuksen analysointiin
  - Vaatii Adafruit_INA219 kirjaston

  Kytkentä:
  Akku+ → INA219 VIN+ → INA219 VIN- → ESP32 VIN

  Ominaisuudet:
  - Jännitemittaus (4mV tarkkuus, PAREMPI kuin ADC!)
  - Virtamittaus (0.1mA tarkkuus)
  - Tehomittaus (mW)
  - Energiankulutus (mAh, Wh)
  - Peak current tracking
  - Käyttöaika-estimaatti

  Käyttöönotto:
  1. Asenna Adafruit_INA219 kirjasto
  2. Aseta config.h: ENABLE_CURRENT_MONITOR true
  3. Kytke INA219 I2C-väylään
  4. Kytke akku INA219:n läpi
  5. Tarkista varoitusrajat (HIGH: 200mA, MAX: 500mA)

  ═══════════════════════════════════════════════════════════════════

  SUOSITUS:

  - Jos tarvitset VAIN jännitteen: Käytä VAIHTOEHTO 1 (yksinkertaisempi)
  - Jos haluat virran/tehon/energian: Käytä VAIHTOEHTO 2 (tarkempi)
  - ÄLÄ käytä molempia yhtä aikaa (turhaa päällekkäisyyttä)

  HUOM: INA219 mittaa myös jännitteen, ja TARKEMMIN kuin ADC!
  Jos sinulla on INA219, ei tarvita erillistä ADC-mittausta.

  ═══════════════════════════════════════════════════════════════════

  API:

  void initSensors()
    - Alustaa käytössä olevat sensorit

  void checkSensors()
    - Lukee sensorit (kutsutaan loop():ssa)

  float getBatteryVoltage()
    - Palauttaa akkujännitteen (V)

  float getBatteryCurrent()
    - Palauttaa virran (mA, vain INA219)

  float getBatteryPower()
    - Palauttaa tehon (mW, vain INA219)

  String getSensorStatus()
    - Palauttaa sensorien tilan

=======================================================================*/

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "config.h"

// Includaa molemmat sensorit konditionaalisesti
#if ENABLE_BATTERY_MONITOR
  #include "battery_monitor.h"
#endif

#if ENABLE_CURRENT_MONITOR
  #include "current_monitor.h"
  #include "i2c_manager.h"  // I2C-alustus vaaditaan
#endif

// Yhdistetty sensorin tila
struct SensorsState {
  bool batteryMonitorActive;
  bool currentMonitorActive;
  float lastVoltage;
  float lastCurrent;
  float lastPower;
  unsigned long lastCheckTime;
};

static SensorsState sensorsState = {false, false, 0.0, 0.0, 0.0, 0};

/**
 * Alustaa sensorit.
 * Kutsuu käytössä olevien sensorien init-funktioita.
 */
void initSensors() {
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║  SENSORS INIT                          ║");
  Serial.println("╚════════════════════════════════════════╝");

  #if ENABLE_BATTERY_MONITOR && ENABLE_CURRENT_MONITOR
  Serial.println("  ⚠️  WARNING: Both battery monitoring methods enabled!");
  Serial.println("  INA219 provides more accurate voltage measurement.");
  Serial.println("  Consider disabling ENABLE_BATTERY_MONITOR.");
  Serial.println();
  #endif

  #if ENABLE_BATTERY_MONITOR
  Serial.println("  Initializing ADC battery monitor (GPIO 35)...");
  initBatteryMonitor();
  sensorsState.batteryMonitorActive = true;
  #endif

  #if ENABLE_CURRENT_MONITOR
  Serial.println("  Initializing INA219 current monitor (I2C 0x40)...");
  ensureI2CInitialized();  // Varmista I2C alustus
  initCurrentMonitor();
  sensorsState.currentMonitorActive = true;
  #endif

  #if !ENABLE_BATTERY_MONITOR && !ENABLE_CURRENT_MONITOR
  Serial.println("  ℹ️  NO SENSORS ENABLED.");
  Serial.println("  Set ENABLE_BATTERY_MONITOR or ENABLE_CURRENT_MONITOR");
  Serial.println("  in config.h to enable battery monitoring.");
  #endif

  Serial.println();
  Serial.println("Sensors ready.");
  Serial.println();
}

/**
 * Lukee sensorit.
 * Kutsutaan loop():ssa säännöllisesti.
 */
void checkSensors() {
  unsigned long now = millis();

  // Tarkista aikaväli
  #if ENABLE_BATTERY_MONITOR
  if (now - sensorsState.lastCheckTime >= BATTERY_CHECK_INTERVAL) {
    checkBattery();
    sensorsState.lastCheckTime = now;
  }
  #endif

  #if ENABLE_CURRENT_MONITOR
  if (now - sensorsState.lastCheckTime >= CURRENT_CHECK_INTERVAL) {
    checkCurrent();
    sensorsState.lastCheckTime = now;
  }
  #endif
}

/**
 * Palauttaa akkujännitteen (V).
 * Käyttää INA219:ää jos saatavilla, muuten ADC:tä.
 * @return Jännite (V) tai 0.0 jos ei saatavilla
 */
float getBatteryVoltage() {
  #if ENABLE_CURRENT_MONITOR
  // INA219 on tarkempi, käytä sitä jos saatavilla
  return getCurrentVoltage();
  #elif ENABLE_BATTERY_MONITOR
  // Käytä ADC:tä
  return getBatteryVoltageADC();
  #else
  return 0.0;
  #endif
}

/**
 * Palauttaa virran (mA).
 * Vaatii INA219:n.
 * @return Virta (mA) tai 0.0 jos ei saatavilla
 */
float getBatteryCurrent() {
  #if ENABLE_CURRENT_MONITOR
  return getCurrentCurrent();
  #else
  return 0.0;
  #endif
}

/**
 * Palauttaa tehon (mW).
 * Vaatii INA219:n.
 * @return Teho (mW) tai 0.0 jos ei saatavilla
 */
float getBatteryPower() {
  #if ENABLE_CURRENT_MONITOR
  return getCurrentPower();
  #else
  return 0.0;
  #endif
}

/**
 * Palauttaa energiankulutuksen (mAh).
 * Vaatii INA219:n.
 * @return Energia (mAh) tai 0.0 jos ei saatavilla
 */
float getBatteryEnergy() {
  #if ENABLE_CURRENT_MONITOR
  return getCurrentEnergy();
  #else
  return 0.0;
  #endif
}

/**
 * Palauttaa sensorien tilan merkkijonona.
 * @return Tila (esim. "ADC_ONLY", "INA219_ONLY", "BOTH", "NONE")
 */
String getSensorStatus() {
  #if ENABLE_BATTERY_MONITOR && ENABLE_CURRENT_MONITOR
  return "BOTH";
  #elif ENABLE_BATTERY_MONITOR
  return "ADC_ONLY";
  #elif ENABLE_CURRENT_MONITOR
  return "INA219_ONLY";
  #else
  return "NONE";
  #endif
}

/**
 * Tulostaa sensorien diagnostiikka.
 */
void printSensorDiagnostics() {
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║  SENSOR DIAGNOSTICS                    ║");
  Serial.println("╚════════════════════════════════════════╝");

  Serial.print("  Configuration: ");
  Serial.println(getSensorStatus());

  Serial.println();

  #if ENABLE_BATTERY_MONITOR
  Serial.println("  ADC Battery Monitor:");
  Serial.print("    Voltage: ");
  Serial.print(getBatteryVoltageADC(), 2);
  Serial.println(" V");
  #endif

  #if ENABLE_CURRENT_MONITOR
  Serial.println("  INA219 Current Monitor:");
  Serial.print("    Voltage: ");
  Serial.print(getCurrentVoltage(), 3);
  Serial.println(" V");
  Serial.print("    Current: ");
  Serial.print(getCurrentCurrent(), 1);
  Serial.println(" mA");
  Serial.print("    Power: ");
  Serial.print(getCurrentPower(), 1);
  Serial.println(" mW");
  Serial.print("    Energy: ");
  Serial.print(getCurrentEnergy(), 2);
  Serial.println(" mAh");
  #endif

  #if ENABLE_BATTERY_MONITOR && ENABLE_CURRENT_MONITOR
  Serial.println();
  Serial.println("  ⚠️  NOTE: Both methods enabled!");
  float voltDiff = abs(getBatteryVoltageADC() - getCurrentVoltage());
  Serial.print("  Voltage difference: ");
  Serial.print(voltDiff * 1000, 1);
  Serial.println(" mV");
  if (voltDiff > 0.1) {
    Serial.println("  → Significant difference! Check calibration.");
  }
  #endif

  Serial.println();
}

#endif // SENSORS_H
