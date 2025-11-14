/*=====================================================================
  fire_alarm_detector.h - Unified Fire Alarm Detection

  FEATURES 11 & 12: Palovaroittimen havaitseminen (ääni + valo)

  Yhdistetty palovaroittimen havaitsemismoduuli joka tukee kahta menetelmää:
  1. Äänihavaitsinta (audio_detector.h)
  2. Valohavaitsinta (light_detector.h)

  Molemmat menetelmät toimivat itsenäisesti tai yhdessä (redundanssi).

  ═══════════════════════════════════════════════════════════════════

  ÄÄNIHAVAITSINTA (Audio Detection):

  Laitteisto:
  - MAX4466 mikrofonivahvistin → GPIO 34 (ADC1_CH6)
  - Havaitsee palovaroittimen äänimerkin (85dB @ 3kHz)
  - Tunnistaa tyypillinen 3-4 piippaus/sekunti -rytmi

  Toimintaperiaate:
  - Laskee RMS (Root Mean Square) äänitasosta
  - Tunnistaa korkean äänitason (threshold: 200)
  - Varmistaa kesto (min 1 sekunti)
  - Laskee piippausmäärän (3-5 peaks/sec = palovaroitin)

  Käyttöönotto:
  1. Aseta config.h: ENABLE_AUDIO_DETECTION true
  2. Kytke MAX4466 OUT → GPIO 34, VCC → 3.3V, GND → GND
  3. Kalibroi AUDIO_THRESHOLD (2-3× hiljaisuuden taso)
  4. Testaa palovaroittimella tai ääniapplikaatiolla (3kHz)

  ═══════════════════════════════════════════════════════════════════

  VALOHAVAITSINTA (Light Detection):

  Laitteisto:
  - TCS34725 RGB-värisensori → I2C (SDA=21, SCL=22)
  - Havaitsee palovaroittimen vilkkuvan punaisen LED:n
  - Tunnistaa tyypillinen 1 Hz vilkkumisrytmi

  Toimintaperiaate:
  - Mittaa RGB-värit jatkuvasti
  - Tunnistaa punaisen valon dominanssin (R/G > 2.0, R/B > 2.0)
  - Seuraa vilkkumisrytmiä (~1 Hz = palovaroitin)
  - Kalibroituu automaattisesti ympäristön punaiseen valoon

  Käyttöönotto:
  1. Aseta config.h: ENABLE_LIGHT_DETECTION true
  2. Asenna Adafruit_TCS34725 -kirjasto
  3. Kytke TCS34725 I2C-väylään (0x29)
  4. Osoita sensori palovaroittimen LED:iin
  5. Testaa punaisella LED-taskulampulla (~1 Hz vilkkuminen)

  ═══════════════════════════════════════════════════════════════════

  YHDISTETTY KÄYTTÖ:

  Molemmat menetelmät voidaan ottaa käyttöön yhtä aikaa:
  - Parannettu luotettavuus (redundanssi)
  - Varmempi havaitseminen äänekkäissä ympäristöissä
  - Vähemmän vääriä hälytyksiä

  Hälytyslogiikka:
  - AUDIO tai LIGHT havaitsee → Lähetä ALERT
  - Molemmat havaitsevat → Korkeampi luotettavuus
  - 5 sekunnin cooldown välttää spämmiä

  ═══════════════════════════════════════════════════════════════════

  API:

  void initFireAlarmDetector()
    - Alustaa käytössä olevat detektorit

  void checkFireAlarm()
    - Tarkistaa hälytykset (kutsutaan loop():ssa)

  bool isFireAlarmActive()
    - Palauttaa true jos hälytys aktiivinen

  String getFireAlarmStatus()
    - Palauttaa tilan merkkijonona (debug)

=======================================================================*/

#ifndef FIRE_ALARM_DETECTOR_H
#define FIRE_ALARM_DETECTOR_H

#include <Arduino.h>
#include "config.h"

// Includaa molemmat detektorit konditionaalisesti
#if ENABLE_AUDIO_DETECTION
  #include "audio_detector.h"
#endif

#if ENABLE_LIGHT_DETECTION
  #include "light_detector.h"
  #include "i2c_manager.h"  // I2C-alustus vaaditaan
#endif

// Yhdistetyn detektorin tila
struct FireAlarmState {
  bool audioAlarmActive;
  bool lightAlarmActive;
  unsigned long lastAlertTime;
  unsigned long alertCount;

  // Statistiikka
  unsigned long audioDetections;
  unsigned long lightDetections;
  unsigned long combinedDetections;
};

static FireAlarmState fireAlarmState = {false, false, 0, 0, 0, 0, 0};

/**
 * Alustaa palovaroittimen havaitsemisen.
 * Kutsuu käytössä olevien detektorien init-funktioita.
 */
void initFireAlarmDetector() {
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║  FIRE ALARM DETECTOR INIT              ║");
  Serial.println("╚════════════════════════════════════════╝");

  #if ENABLE_AUDIO_DETECTION
  Serial.println("  Initializing AUDIO detector...");
  initAudioDetector();
  #endif

  #if ENABLE_LIGHT_DETECTION
  Serial.println("  Initializing LIGHT detector...");
  ensureI2CInitialized();  // Varmista I2C alustus
  initLightDetector();
  #endif

  #if !ENABLE_AUDIO_DETECTION && !ENABLE_LIGHT_DETECTION
  Serial.println("  ⚠️  NO DETECTORS ENABLED!");
  Serial.println("  Set ENABLE_AUDIO_DETECTION or ENABLE_LIGHT_DETECTION");
  Serial.println("  in config.h to enable fire alarm detection.");
  #endif

  Serial.println();
  Serial.println("Fire alarm detector ready.");
  Serial.println();
}

/**
 * Tarkistaa palovaroittimen hälytykset.
 * Kutsutaan loop():ssa säännöllisesti.
 */
void checkFireAlarm() {
  bool audioTriggered = false;
  bool lightTriggered = false;

  // Tarkista äänihavaitsinta
  #if ENABLE_AUDIO_DETECTION
  audioTriggered = checkAudioAlarm();
  fireAlarmState.audioAlarmActive = audioTriggered;
  #endif

  // Tarkista valohavaitsinta
  #if ENABLE_LIGHT_DETECTION
  lightTriggered = checkLightAlarm();
  fireAlarmState.lightAlarmActive = lightTriggered;
  #endif

  // Jos jompikumpi tai molemmat havaitsi
  if (audioTriggered || lightTriggered) {
    unsigned long now = millis();

    // Cooldown: Vältetään liiallista hälytysspämmiä
    if (now - fireAlarmState.lastAlertTime > AUDIO_COOLDOWN) {
      fireAlarmState.lastAlertTime = now;
      fireAlarmState.alertCount++;

      // Päivitä statistiikka
      if (audioTriggered) fireAlarmState.audioDetections++;
      if (lightTriggered) fireAlarmState.lightDetections++;
      if (audioTriggered && lightTriggered) fireAlarmState.combinedDetections++;

      // Tulosta hälytys
      Serial.println("╔════════════════════════════════════════╗");
      Serial.println("║  🚨 FIRE ALARM DETECTED! 🚨            ║");
      Serial.println("╚════════════════════════════════════════╝");

      Serial.print("  Detection method: ");
      if (audioTriggered && lightTriggered) {
        Serial.println("AUDIO + LIGHT (CONFIRMED!)");
      } else if (audioTriggered) {
        Serial.println("AUDIO ONLY");
      } else {
        Serial.println("LIGHT ONLY");
      }

      Serial.print("  Alert count: ");
      Serial.println(fireAlarmState.alertCount);

      Serial.print("  Total detections: ");
      Serial.print("Audio=");
      Serial.print(fireAlarmState.audioDetections);
      Serial.print(", Light=");
      Serial.print(fireAlarmState.lightDetections);
      Serial.print(", Combined=");
      Serial.println(fireAlarmState.combinedDetections);

      Serial.println();

      // Tässä voidaan lähettää LoRa-hälytys
      // Esim: sendLoRaMessage("ALERT:FIRE_ALARM");
    }
  }
}

/**
 * Palauttaa onko palovaroitus aktiivinen.
 * @return true jos jompikumpi detektori on havainnut hälytyksen
 */
bool isFireAlarmActive() {
  return fireAlarmState.audioAlarmActive || fireAlarmState.lightAlarmActive;
}

/**
 * Palauttaa palovaroittimen tilan merkkijonona.
 * @return Tila (esim. "IDLE", "AUDIO_ALARM", "LIGHT_ALARM", "COMBINED_ALARM")
 */
String getFireAlarmStatus() {
  if (fireAlarmState.audioAlarmActive && fireAlarmState.lightAlarmActive) {
    return "COMBINED_ALARM";
  } else if (fireAlarmState.audioAlarmActive) {
    return "AUDIO_ALARM";
  } else if (fireAlarmState.lightAlarmActive) {
    return "LIGHT_ALARM";
  } else {
    return "IDLE";
  }
}

/**
 * Tulostaa palovaroittimen statistiikka.
 */
void printFireAlarmStats() {
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║  FIRE ALARM STATISTICS                 ║");
  Serial.println("╚════════════════════════════════════════╝");

  Serial.print("  Total alerts: ");
  Serial.println(fireAlarmState.alertCount);

  Serial.print("  Audio detections: ");
  Serial.println(fireAlarmState.audioDetections);

  Serial.print("  Light detections: ");
  Serial.println(fireAlarmState.lightDetections);

  Serial.print("  Combined detections: ");
  Serial.println(fireAlarmState.combinedDetections);

  if (fireAlarmState.alertCount > 0) {
    float confirmRate = (float)fireAlarmState.combinedDetections / fireAlarmState.alertCount * 100.0;
    Serial.print("  Confirmation rate: ");
    Serial.print(confirmRate, 1);
    Serial.println("%");
  }

  Serial.print("  Current status: ");
  Serial.println(getFireAlarmStatus());

  Serial.println();
}

/**
 * Nollaa palovaroittimen statistiikan.
 */
void resetFireAlarmStats() {
  fireAlarmState.alertCount = 0;
  fireAlarmState.audioDetections = 0;
  fireAlarmState.lightDetections = 0;
  fireAlarmState.combinedDetections = 0;

  Serial.println("✓ Fire alarm statistics reset.");
}

#endif // FIRE_ALARM_DETECTOR_H
