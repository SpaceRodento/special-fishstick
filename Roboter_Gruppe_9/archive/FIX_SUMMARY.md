# TFT-näytön UART-yhteyden korjaus

**Päivämäärä:** 2025-11-13
**Branch:** claude/integrate-esp32-display-011CUvsmjx7BzP8FjEu3t9E1
**Ongelma:** TFT-näyttö näyttää grafiikkaa mutta "No Data" - UART-yhteys ei toimi

---

## 🔍 Ongelman juurisyy

### Mikä meni pieleen?

Commitissa **0a0e2fd** ("CRITICAL FIX: Change LoRa to actually use Serial2") tapahtui kaksi muutosta:

1. ✅ **LoRa siirrettiin Serial1:stä → Serial2:lle** (OIKEIN)
2. ❌ **DisplayClient muutettiin Serial2:sta → Serial1:ksi** (VÄÄRIN!)

### Tekniset ongelmat

**1. Väärä UART-instanssi**
```cpp
// VANHA (TOIMIVA):
serial = &Serial2;  // Viittaus Serial2:een, custom pins

// UUSI (RIKKI):
serial = &Serial1;  // Viittaus Serial1:een, DEFAULT pins GPIO 9/10
```

**Ongelma:** `&Serial1` viittaa globaaliin Serial1-instanssiin, jolla on oletuspinnit GPIO 9/10. Nämä pinnit ovat varattu flash-muistille ESP32:ssa!

**2. pinMode() puuttuu**
```cpp
// Robot_Sender.ino (TOIMIVA):
pinMode(UART_TX_PIN, OUTPUT);  // ← KRIITTINEN!
DisplaySerial.begin(115200, SERIAL_8N1, -1, 23);

// DisplayClient.h (RIKKI):
// EI pinMode() kutsua!
serial->begin(115200, SERIAL_8N1, -1, 23);
```

**Ongelma:** Ilman pinMode() kutsua, GPIO 23 ei ole konfiguroitu outputiksi.

### Miksi toimiva versio toimi?

**Robot_Sender + Display_Device yhdistelmä:**
- Robot_Sender loi **UUDEN** HardwareSerial instanssin: `HardwareSerial DisplaySerial(1);`
- Kutsui `pinMode(23, OUTPUT);` ennen `begin()`
- GPIO 23 toimi TX:nä, TFT:n GPIO 3 (UART0 RX) vastaanotti

---

## ✅ Korjaus

### Muutokset tiedostoon `DisplayClient.h`

**1. Luodaan uusi HardwareSerial-instanssi**

```cpp
// ENNEN:
serial = &Serial1;  // Viittaus globaaliin, oletuspinnit

// JÄLKEEN:
serial = new HardwareSerial(1);  // Uusi instanssi, custom pins
```

**Perustelu:** Uusi instanssi mahdollistaa custom pin-kartoituksen ilman konflikteja.

**2. Lisätään pinMode() kutsu**

```cpp
// ENNEN:
void begin() {
  // EI pinMode() kutsua!
  serial->begin(baudrate, SERIAL_8N1, -1, txPin);
}

// JÄLKEEN:
void begin() {
  pinMode(txPin, OUTPUT);  // ← LISÄTTY!
  if (rxPin != -1) {
    pinMode(rxPin, INPUT);
  }
  serial->begin(baudrate, SERIAL_8N1, -1, txPin);
}
```

**Perustelu:** pinMode() on pakollinen kun käytetään custom pinneja uudessa HardwareSerial-instanssissa.

---

## 🧪 Testaussuunnitelma

Katso yksityiskohtainen testaussuunnitelma tiedostosta: **TFT_DISPLAY_DEBUG_PLAN.md**

### Pikalista:

1. ✅ **Käännä koodi** Arduino IDE:ssä (tarkista virheet)
2. ✅ **Lataa Display_Device.ino** TFT-näytölle
3. ✅ **Lataa Roboter_Gruppe_9.ino** pää-ESP32:lle
4. ✅ **Kytke kaapelit:**
   - Roboter GPIO 23 → TFT RX (fyysinen pinni)
   - GND → GND
5. ✅ **Tarkista Serial Monitor:**
   - Roboter: "→ Display: Mode:SENDER,..."
   - TFT: Datan vastaanotto
6. ✅ **Tarkista TFT-näyttö:**
   - Pitäisi näyttää dataa (EI "No Data")

---

## 📋 Seuraavat askeleet

### Ennen fyysistä testausta:

1. [ ] Käännä koodi Arduino IDE:ssä
2. [ ] Tarkista että ei compile erroreja
3. [ ] Tarkista että ENABLE_DISPLAY_OUTPUT on true config.h:ssa
4. [ ] Tarkista että DISPLAY_TYPE = 2 (TFT)

### Fyysinen testaus:

1. [ ] Lataa koodi molempiin laitteisiin
2. [ ] Kytke kaapelit (GPIO 23 → RX, GND → GND)
3. [ ] Käynnistä molemmat laitteet
4. [ ] Tarkista Serial Monitor (molemmat laitteet)
5. [ ] Tarkista TFT-näyttö (pitäisi päivittyä 2s välein)

### Jos ongelma jatkuu:

1. [ ] Mittaa GPIO 23 jännitettä (pitäisi vaihdella 0-3.3V)
2. [ ] Tarkista GND-yhteys (multimetrillä)
3. [ ] Tarkista baudrate (molemmat 115200)
4. [ ] Kokeile yksinkertaista Robot_Sender.ino koodia

---

## 🔗 Liittyvät tiedostot

- **Korjattu tiedosto:** `Roboter_Gruppe_9/DisplayClient.h`
- **Testaussuunnitelma:** `Roboter_Gruppe_9/TFT_DISPLAY_DEBUG_PLAN.md`
- **Konfiguraatio:** `Roboter_Gruppe_9/config.h`
- **Pääohjelma:** `Roboter_Gruppe_9/Roboter_Gruppe_9.ino`
- **TFT-näytön koodi:** `Roboter_Display_TFT/Roboter_Display_TFT.ino`
- **Toimiva esimerkki:** `Robot_Sender/Robot_Sender.ino` (debug branchissa)

---

## 🐛 Debuggaus-vinkit

### Serial Monitor näyttää "→ Display: ..." mutta TFT ei reagoi

**Syyt:**
1. TX-pinni ei ole oikein (GPIO 23?)
2. pinMode() ei ole asetettu
3. Väärä baudrate
4. GND puuttuu
5. TX menee väärään pinniin TFT:ssä

**Testaa:**
```cpp
void setup() {
  Serial.begin(115200);
  pinMode(23, OUTPUT);

  // Testaa blinkkausta
  for (int i = 0; i < 10; i++) {
    digitalWrite(23, HIGH);
    delay(100);
    digitalWrite(23, LOW);
    delay(100);
  }

  // Jos LED vilkkuu TFT:n RX-pinnissä, kytkentä on OK!
}
```

### TFT näyttää "No Data" vaikka Serial Monitor näyttää lähetykset

**Syyt:**
1. DisplayClient ei lähetä oikeasti (Serial.print vs serial->print)
2. UART ei ole alustettu oikein
3. HardwareSerial instanssi on väärä

**Testaa:**
```cpp
// Lisää DisplayClient.h send()-metodiin:
void send() {
  serial->println(dataBuffer);
  Serial.print("→ Display: ");
  Serial.println(dataBuffer);

  // TESTAA: Lähetä myös USB-serialiin
  Serial.println("DEBUG: serial->println() called!");
}
```

---

## ✨ Miksi tämä korjaus toimii?

1. **Uusi HardwareSerial-instanssi** mahdollistaa custom pin-kartoituksen
2. **pinMode()** asettaa GPIO 23:n outputiksi
3. **HardwareSerial(1).begin(-1, 23)** käyttää GPIO 23:a TX:nä
4. **TFT UART0 (GPIO 3 RX)** vastaanottaa datan
5. **Ei konflikteja** LoRa:n kanssa (LoRa käyttää HardwareSerial(2) GPIO 32/33:lla)

---

## 📚 Lähteet ja referenssit

- Toimiva esimerkki: `Robot_Sender.ino` (claude/debug-esp32-display-connection-011CV5ppr9SvLCPTtr4txtoW)
- ESP32 UART dokumentaatio: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html
- HardwareSerial ESP32: https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.h

---

**Testi tämä korjaus ja raportoi tulokset!** 🚀
