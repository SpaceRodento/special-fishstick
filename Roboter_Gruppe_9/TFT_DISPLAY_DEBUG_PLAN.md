# TFT-näytön debuggaus- ja testaussuunnitelma

**Päivämäärä:** 2025-11-13
**Ongelma:** TFT-näyttö näyttää grafiikkaa mutta "No Data" - UART-yhteys ei toimi
**Branch:** claude/integrate-esp32-display-011CUvsmjx7BzP8FjEu3t9E1

---

## 🔍 Ongelman analyysi

### Havaitut ongelmat

1. **DisplayClient käyttää väärää UART-instanssia**
   - Nykyinen: `serial = &Serial1;` (viittaus globaaliin, oletuspinnit GPIO 9/10)
   - Toimiva: `HardwareSerial DisplaySerial(1);` (uusi instanssi, vapaat pinnit)
   - **Ongelma:** Serial1 oletuspinnit (GPIO 9/10) ovat varattu flash-muistille!

2. **pinMode() puuttuu**
   - Toimiva versio (Robot_Sender): `pinMode(UART_TX_PIN, OUTPUT);` ENNEN begin()
   - Nykyinen (DisplayClient): EI pinMode() kutsua
   - **Ongelma:** TX-pinni ei ole konfiguroitu outputiksi

3. **Väärä UART vaihdettu branchien välillä**
   - Toimiva branch: DisplayClient käyttää Serial2
   - Integrate branch: DisplayClient muutettu Serial1:ksi
   - **Ongelma:** Muutos tehtiin väärään suuntaan commitissa 0a0e2fd

### Toimiva yhdistelmä (Robot_Sender + Display_Device)

```cpp
// LÄHETTÄJÄ (Robot_Sender.ino)
HardwareSerial DisplaySerial(1);  // Uusi instanssi

void setup() {
  pinMode(23, OUTPUT);  // ← KRIITTINEN!
  DisplaySerial.begin(115200, SERIAL_8N1, -1, 23);
}

// VASTAANOTTAJA (Display_Device.ino, TFT ESP32-2432S022)
void setup() {
  Serial.begin(115200);  // UART0, fyysinen RX-pinni GPIO 3
}
```

**Kytkentä:**
- Roboter_Gruppe_9 GPIO 23 (TX) → TFT fyysinen RX-pinni (GPIO 3 / UART0)
- GND → GND

---

## 🔧 Korjaussuunnitelma

### Korjaus 1: Muuta DisplayClient käyttämään uutta HardwareSerial-instanssia

**Tiedosto:** `Roboter_Gruppe_9/DisplayClient.h`

**Muutokset:**

1. Lisää private-osioon HardwareSerial-instanssi:
   ```cpp
   HardwareSerial* displaySerial;  // Oma instanssi
   ```

2. Konstruktorissa luo uusi instanssi:
   ```cpp
   displaySerial = new HardwareSerial(1);  // UART1 uusi instanssi
   ```

3. begin()-metodissa:
   - Lisää pinMode() kutsu
   - Käytä displaySerial-instanssia

**Vaihtoehtoisesti:** Käytä suoraan Serial2:sta (kuten toimivassa debug branchissa), koska:
- LoRa käyttää omaa HardwareSerial(2) instanssia eri pinneillä
- DisplayClient voi käyttää &Serial2:sta GPIO 23:lla
- Ei konfliktia!

### Korjaus 2: Testaa ensin yksinkertainen versio

Kopioi Robot_Sender logiikka suoraan Roboter_Gruppe_9.ino:on testaamista varten.

---

## ✅ Testaussuunnitelma (ominaisuus kerrallaan)

### Vaihe 1: Perus UART-yhteys (KRIITTINEN)

**Tavoite:** Varmista että TFT-näyttö saa dataa ylipäätään

**Testaus:**
1. [ ] Lataa Display_Device.ino TFT-näytölle (ESP32-2432S022)
2. [ ] Lataa Roboter_Gruppe_9.ino päälaittee  lle (muokattu DisplayClient)
3. [ ] Kytke kaapelit:
   - Roboter GPIO 23 → TFT RX (fyysinen pinni)
   - GND → GND
4. [ ] Tarkista Serial Monitor:
   - Roboter: "→ Display: Mode:SENDER,SEQ:0,..."
   - TFT: "📥 RX [1]: ..." tai vastaava
5. [ ] Tarkista TFT-näyttö:
   - Pitäisi näyttää dataa, EI "No Data"

**Onnistumiskriteerit:**
- ✅ Roboter lähettää dataa serialiin
- ✅ TFT vastaanottaa dataa
- ✅ TFT päivittää näyttöä

**Jos epäonnistuu:**
- Tarkista baudrate (115200 molemmissa)
- Tarkista GND-yhteys
- Tarkista että TX menee RX:ään (ei TX→TX!)
- Mittaa jännitettä GPIO 23:sta (pitäisi vaihdella)

---

### Vaihe 2: LoRa + TFT samanaikaisesti

**Tavoite:** Varmista että LoRa ja TFT toimivat yhdessä

**Testaus:**
1. [ ] Kytke LoRa-moduuli (GPIO 32/33)
2. [ ] Kytke TFT-näyttö (GPIO 23)
3. [ ] Aseta config.h:
   ```cpp
   #define ENABLE_LORA true
   #define DISPLAY_TYPE 2  // TFT
   ```
4. [ ] Lataa koodi ja testaa:
   - LoRa lähettää/vastaanottaa
   - TFT näyttää dataa
   - Ei konflikteja

**Onnistumiskriteerit:**
- ✅ LoRa viestit kulkevat (RSSI näkyy)
- ✅ TFT päivittyy 2s välein
- ✅ Ei kaatumisia tai virheitä

---

### Vaihe 3: LCD + LoRa (valinnainen)

**Tavoite:** Testaa myös LCD-näyttö

**Testaus:**
1. [ ] Aseta config.h: `#define DISPLAY_TYPE 1` (LCD)
2. [ ] Kytke I2C LCD (SDA=21, SCL=22)
3. [ ] Testaa receiver-mode:
   - Jumper GPIO 16↔17
   - LCD näyttää RSSI-palkit
   - Connection status: OK/WEAK/LOST

**Onnistumiskriteerit:**
- ✅ LCD näyttää dataa
- ✅ Signal bars päivittyvät
- ✅ Spinner animaatio toimii

---

### Vaihe 4: Kaikki ominaisuudet yhdessä

**Tavoite:** Täydellinen integraatiotesti

**Testaus:**
1. [ ] Sender + Receiver + TFT Display Station
2. [ ] Kaikki kolme laitetta päällä samanaikaisesti
3. [ ] Viestit kulkevat molempiin suuntiin (jos ENABLE_BIDIRECTIONAL=true)
4. [ ] TFT näyttää sekä sender- että receiver-dataa

**Onnistumiskriteerit:**
- ✅ Viestit kulkevat sender→receiver
- ✅ ACK-viestit kulkevat receiver→sender
- ✅ TFT näyttää ajantasaista dataa
- ✅ Connection watchdog toimii (LOST/CONNECTED)
- ✅ Ei packet loss:ia lähietäisyydellä

---

### Vaihe 5: Rangetestauk set

**Tavoite:** Testaa yhteyden toimivuus eri etäisyyksillä

**Testaus:**
1. [ ] Lähietäisyys (0-10m):
   - RSSI > -70 dBm
   - TFT päivittyy joka kerta
   - Packet loss < 1%

2. [ ] Keskietäisyys (10-50m):
   - RSSI -70...-90 dBm
   - TFT päivittyy säännöllisesti
   - Packet loss < 5%

3. [ ] Pitkä etäisyys (50-100m):
   - RSSI < -90 dBm
   - TFT näyttää WEAK connection
   - Auto-recovery toimii

**Onnistumiskriteerit:**
- ✅ Yhteys säilyy koko matkan
- ✅ TFT näyttää aina viimeisimmän datan
- ✅ RSSI-arvot laskevat etäisyyden kasvaessa

---

### Vaihe 6: Pitkäaikaistesti (1-4h)

**Tavoite:** Varmista stabiilisuus

**Testaus:**
1. [ ] Jätä laitteet päälle 1 tunniksi
2. [ ] Seuraa Serial Monitor:ia
3. [ ] Tarkista:
   - Ei kaatumisia
   - Ei memory leak:ia
   - Packet loss pysyy vakaana
   - TFT päivittyy koko ajan

**Onnistumiskriteerit:**
- ✅ Ei reboot:eja
- ✅ Free heap pysyy vakaana
- ✅ TFT toimii koko testin ajan

---

## 🐛 Troubleshooting

### TFT näyttää "No Data"

**Syyt:**
1. UART-yhteys ei toimi (pinMode() puuttuu)
2. Väärä baudrate
3. TX ei yhdisty RX:ään (väärä pinni)
4. GND puuttuu

**Korjaus:**
1. Lisää `pinMode(23, OUTPUT);` ennen UART begin()
2. Tarkista että molemmat käyttävät 115200
3. Mittaa GPIO 23 jännitettä (pitäisi vaihdella)
4. Tarkista GND-yhteys

### LoRa ja TFT konflikti

**Syyt:**
1. Molemmat käyttävät samaa UART:ia samoilla pinneillä
2. pinMode() asetettu väärässä järjestyksessä

**Korjaus:**
1. LoRa: HardwareSerial(2) GPIO 32/33
2. TFT: HardwareSerial(1) GPIO 23
3. Erilliset instanssit, ei konfliktia!

### Compile error: "Serial1 not defined"

**Syyt:**
- Yritetään käyttää &Serial1 viitettä

**Korjaus:**
- Käytä `new HardwareSerial(1)` sen sijaan

---

## 📝 Testausdokumentointi

Täytä jokaisen testin jälkeen:

**Testi:** _______________________
**Päivämäärä:** _______________________
**Tulos:** ✅ PASS / ❌ FAIL
**RSSI:** _______ dBm
**Packet loss:** _______ %
**Huomiot:**
_________________________________
_________________________________

---

## 🎯 Seuraavat askeleet korjauksen jälkeen

1. [ ] Merge korjaukset debug branchiin
2. [ ] Testaa kaikki ominaisuudet läpi (Vaiheet 1-6)
3. [ ] Päivitä dokumentaatio (TESTING_CHECKLIST.md)
4. [ ] Luo pull request mainiin
5. [ ] Tagaa toimiva versio (esim. v2.3-stable)

---

**Muistiinpanot:**

_________________________________
_________________________________
_________________________________
