/*=====================================================================
  Example_Roboter9.ino - Roboter Gruppe 9 integraatio

  Näyttää miten lisätä näyttötuki Roboter_Gruppe_9 projektiin.
  Lisää tämä koodi olemassa olevaan loop():iin.
=======================================================================*/

#include "DisplayClient.h"

// Luo display-client
DisplayClient display(17);  // TX pin 17

// Lisää setup():iin
void setup() {
  Serial.begin(115200);

  // ... muu setup-koodi ...

  // Alusta näyttö
  display.begin();
  display.alert("Roboter 9 online");
  delay(2000);
  display.clearAlert();
}

// Lisää loop():iin
void loop() {
  // ... olemassa oleva loop-koodi ...

  // Lähetä näytölle (esim. 2 sekunnin välein)
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate >= 2000) {
    lastDisplayUpdate = millis();

    // Lähetä tila
    display.clear();
    display.set("SEQ", local.sequenceNumber);
    display.set("LED", local.ledState ? "ON" : "OFF");
    display.set("TOUCH", local.touchState ? "YES" : "NO");

    // Jos on RSSI-tieto
    if (remote.rssi != 0) {
      display.set("RSSI", String(remote.rssi) + "dBm");
    }

    // Jos on akku
    #if ENABLE_BATTERY_MONITOR
      float batV = readBatteryVoltage();
      display.set("Battery", String(batV, 2) + "V");
    #endif

    // Jos on telemetry
    #if ENABLE_EXTENDED_TELEMETRY
      display.set("Heap", String(ESP.getFreeHeap() / 1024) + "KB");
      display.set("Uptime", String(millis() / 1000) + "s");
    #endif

    // Lähetä kaikki
    display.send();

    // Hälytykset
    #if ENABLE_AUDIO_DETECTION
      if (isFireAlarmActive()) {
        display.alert("FIRE: Audio detected!");
      }
    #endif

    #if ENABLE_LIGHT_DETECTION
      if (isFireLightActive()) {
        display.alert("FIRE: Light detected!");
      }
    #endif
  }
}

// Vaihtoehtoinen tapa: Luo oma funktio
void updateDisplay() {
  display.clear();

  // Perustiedot
  display.set("Mode", bRECEIVER ? "RECEIVER" : "SENDER");
  display.set("LED", local.ledState ? "ON" : "OFF");
  display.set("Touch", local.touchState ? "YES" : "NO");
  display.set("Count", local.messageCount);

  // Jos receiver, näytä remote-tiedot
  if (bRECEIVER && remote.messageCount > 0) {
    display.set("Remote_LED", remote.ledState ? "ON" : "OFF");
    display.set("RSSI", String(remote.rssi) + "dBm");
    display.set("SNR", String(remote.snr) + "dB");
  }

  display.send();
}
