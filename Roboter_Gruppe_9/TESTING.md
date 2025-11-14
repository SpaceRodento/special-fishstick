# Roboter Gruppe 9 - Testing Guide

**Kattava testausopas kaikille ominaisuuksille**

Käytä tätä ohjetta varmistaaksesi, että kaikki toiminnot toimivat oikein.

Viimeksi päivitetty: 2025-11-14

---

## 📋 Testausmerkinnät

- ✅ = Testattu ja toimii
- ❌ = Testattu mutta ei toimi
- ⏭️ = Ohitettu (ominaisuus pois päältä tai ei tarvita)
- 🔲 = Ei vielä testattu

**Testausjärjestys:**
1. Ydintoiminnot (LoRa, roolintunnistus)
2. Näyttö-ominaisuudet (LCD, TFT)
3. Valinnaiset ominaisuudet (tarpeen mukaan)

---

## 🎯 Osa 1: Ydintoiminnot (PAKOLLINEN)

### 1.1 Laitteiston asennus

- [ ] **LoRa-moduuli kytketty oikein**
  - RYLR896 TX → ESP32 GPIO 25
  - RYLR896 RX → ESP32 GPIO 26
  - Virta: 3.3V, GND

- [ ] **Roolivalinnan kytkentä**
  - VASTAANOTTAJA: GPIO 16 ↔ GPIO 17 (hyppylanka)
  - LÄHETTÄJÄ: GPIO 16 irti (ei yhteyttä)

### 1.2 Koodin lataus

- [ ] **Koodi kääntyy ilman virheitä**
  - Avaa `Roboter_Gruppe_9.ino`
  - Valitse: ESP32 Dev Module
  - Käännä (Ctrl+R / Cmd+R)

- [ ] **Koodi latautuu onnistuneesti**
  - Lataa molempiin ESP32-laitteisiin
  - Sama koodi molemmille!

### 1.3 Serial Monitor -tarkistus

- [ ] **LÄHETTÄJÄ käynnistyy oikein**
  ```
  >>> SENDER MODE
  ✓ LoRa initialized
  ✓ Setup complete!
  ```

- [ ] **VASTAANOTTAJA käynnistyy oikein**
  ```
  >>> RECEIVER MODE
  ✓ LoRa initialized
  ✓ LCD initialized (jos LCD kytkettynä)
  ✓ Setup complete!
  ```

### 1.4 LoRa-kommunikaatio

- [ ] **Lähettäjä lähettää viestejä**
  - Serial: `📤 TX [1]: SEQ:0,LED:1,...`
  - 2 sekunnin välein
  - Viestin numero kasvaa

- [ ] **Vastaanottaja saa viestejä**
  - Serial: `📥 RX [1]: SEQ:0,LED:1,...`
  - RSSI ja SNR arvot näkyvissä
  - Viestin numero kasvaa

- [ ] **RSSI/SNR-arvot järkeviä**
  - RSSI: -50 to -120 dBm (lähempänä = parempi)
  - SNR: -20 to +10 dB (korkeampi = parempi)
  - Liian lähellä (<50cm): RSSI voi olla "liian hyvä" (-30 dBm)

### 1.5 Perustoiminnot

- [ ] **LED vilkkuu**
  - GPIO 2 LED vilkkuu 500ms välein
  - Sekä lähettäjä että vastaanottaja

- [ ] **Kosketussensori toimii**
  - Kosketa GPIO 4 (T0)
  - Serial: `Touch: YES` tai `touchState: 1`

- [ ] **Kill-switch toimii**
  - Yhdistä GPIO 13 ↔ GPIO 14
  - Pidä 3 sekuntia
  - ESP32 käynnistyy uudelleen

**Tulos:** ✅ Kaikki ydintoiminnot toimivat → Jatka testaamista

---

## 🖥️ Osa 2: Näyttö-ominaisuudet

### 2.1 I2C LCD (valinnainen, vain vastaanottaja)

- [ ] **LCD kytketty**
  - SDA → GPIO 21
  - SCL → GPIO 22
  - I2C-osoite: 0x27

- [ ] **LCD näyttää dataa**
  - Rivi 1: Otsikko tai tila
  - Rivi 2: RSSI, viestimäärä
  - Päivittyy 2s välein

- [ ] **LCD-versiot (valitse yksi)**
  - Version 1: Leveä RSSI-palkki
  - Version 2: Kompakti numeroilla (oletus)
  - Version 3: Yksityiskohtainen SNR:llä
  - Version 4: Alkuperäinen yksinkertainen

### 2.2 TFT-näyttöasema (ESP32-2432S022)

#### Laitteisto
- [ ] **Näyttölaite kytketty**
  - Robot GPIO 23 (TX) → Display GPIO 18 (RX)
  - Robot GND → Display GND
  - Display saa virran OMASTA USB:stä

- [ ] **config.h asetettu**
  ```cpp
  #define ENABLE_DISPLAY_OUTPUT true
  ```

- [ ] **Näyttökoodi ladattu**
  - Avaa `Roboter_Display_TFT.ino`
  - Valitse: ESP32 Dev Module
  - Lataa ESP32-2432S022:een

#### Toiminta
- [ ] **Näyttö käynnistyy**
  - Boot-ruutu: "ROBOTER 9"
  - Sitten: "Waiting for data..."

- [ ] **Näyttö vastaanottaa dataa**
  - Serial: `📥 RX [1]: MODE:RX,...`
  - Pakettilaskuri kasvaa

- [ ] **Näyttö päivittyy**
  - Header: "ROBOTER 9" + LED-indikaattori + yhteys
  - Data-alue: Mode, Packets, RSSI, SNR
  - Alert-alue: "LoRa ACTIVE" tai "NO LINK"

- [ ] **Signaalipalkki toimii**
  - Oikeassa reunassa pystysuora palkki
  - Väri: Vihreä (hyvä), Oranssi (keskinkertainen), Punainen (heikko)
  - Täyttöaste vastaa signaalin laatua

#### Vianmääritys
- [ ] Jos ei dataa:
  - Tarkista kytkennät (GPIO 23 → GPIO 18, GND → GND)
  - Varmista 115200 baud molemmissa
  - Robot Serial: `→ Display: MODE:...`
  - Display Serial: `📥 RX [1]: ...`

---

## 🔌 Osa 3: Kaksisuuntainen kommunikaatio (ACK)

- [ ] **Kaksisuuntainen käytössä**
  ```cpp
  #define ENABLE_BIDIRECTIONAL true  // config.h
  ```

- [ ] **ACK-viestit toimivat**
  - Vastaanottaja lähettää ACK joka 5. viesti
  - Lähettäjän Serial: `✓ ACK #X received`
  - ACK-laskuri kasvaa

- [ ] **ACK-RSSI näkyy**
  - Lähettäjän Serial: RSSI ja SNR ACK-viestistä
  - Signaalin laatu molempiin suuntiin

---

## 💻 Osa 4: PC-datan tallennus

### 4.1 CSV-tuloste

- [ ] **CSV käytössä**
  ```cpp
  #define ENABLE_CSV_OUTPUT true  // config.h
  ```

- [ ] **CSV-data Serial Monitorissa**
  ```
  DATA_CSV,45632,RX,-67,9,142,142,OK,0.00,1,0
  ```
  - 2 sekunnin välein
  - Sisältää: TIMESTAMP, ROLE, RSSI, SNR, SEQ, jne.

### 4.2 Python-skriptit

#### serial_monitor.py
- [ ] **Asennus**
  ```bash
  pip install pyserial
  ```

- [ ] **Käynnistys**
  ```bash
  python serial_monitor.py /dev/ttyUSB0 115200
  ```

- [ ] **Toiminta**
  - Värjätty tuloste
  - RSSI-palkit näkyvissä
  - Viestit päivittyvät reaaliajassa

#### data_logger.py
- [ ] **Käynnistys**
  ```bash
  python data_logger.py /dev/ttyUSB0 115200 lora_data.db
  ```

- [ ] **Toiminta**
  - SQLite-tietokanta luotu
  - Viestit tallennetaan
  - Aikaleima PC:n kellosta

- [ ] **Tietokannan tarkistus**
  ```bash
  sqlite3 lora_data.db
  SELECT COUNT(*) FROM lora_messages;
  ```

---

## 🔋 Osa 5: Valinnaiset anturit

### 5.1 Akkuseuranta

**Laitteisto:**
```
Akku+ ──[10kΩ]── GPIO 35 ──[10kΩ]── GND
```

- [ ] **Asetukset**
  ```cpp
  #define ENABLE_BATTERY_MONITOR true
  ```

- [ ] **Toiminta**
  - Serial: `🔋 Battery: 3.85V ✓ OK`
  - 60s välein (oletuksena)

- [ ] **Varoitukset**
  - Matala akku (<3.3V): `⚠️ Low battery`
  - Kriittinen (<3.0V): `🔴 Critical battery`

### 5.2 Äänentunnistus (palovaroitin)

**Laitteisto:**
- MAX4466 mikrofoni → GPIO 34

- [ ] **Asetukset**
  ```cpp
  #define ENABLE_AUDIO_DETECTION true
  ```

- [ ] **Testaus**
  - Käytä palovaroitinta TAI 3kHz ääniä 3-4 piippauksen rytmillä
  - Serial: `🚨 FIRE ALARM DETECTED (audio)!`

### 5.3 Valontunnistus (palovaroitin)

**Laitteisto:**
- TCS34725 RGB-sensori → I2C

- [ ] **Asetukset**
  ```cpp
  #define ENABLE_LIGHT_DETECTION true
  ```

- [ ] **Testaus**
  - Osoita sensori vilkkuvaan punaiseen LED:iin (~1 Hz)
  - Serial: `🚨 FIRE ALARM DETECTED (light)!`

### 5.4 Virtamittaus

**Laitteisto:**
- INA219 virtamittari → I2C

- [ ] **Asetukset**
  ```cpp
  #define ENABLE_CURRENT_MONITOR true
  ```

- [ ] **Toiminta**
  - Serial: `⚡ 3.85V, 85mA, 328mW`
  - Energian seuranta: `🔋 Energy: 12.5 mAh`

---

## ⚙️ Osa 6: Järjestelmä-ominaisuudet

### 6.1 Laajennettu telemetria

- [ ] **Asetukset**
  ```cpp
  #define ENABLE_EXTENDED_TELEMETRY true
  ```

- [ ] **Toiminta**
  - Uptime (sekuntia käynnistyksestä)
  - Free heap (vapaa muisti KB)
  - Temperature (sisäinen lämpötila °C)

### 6.2 Adaptiivinen SF (Spreading Factor)

- [ ] **Asetukset**
  ```cpp
  #define ENABLE_ADAPTIVE_SF true
  ```

- [ ] **Testaus**
  - Siirrä lähettäjä kauemmaksi
  - Serial: `📡 SF adjusted: 12 → 10` tai päinvastoin
  - SF kasvaa heikon signaalin kanssa

### 6.3 Pakettitilastot

- [ ] **Asetukset**
  ```cpp
  #define ENABLE_PACKET_STATS true
  ```

- [ ] **Toiminta**
  - Uudelleenyritykset
  - Duplikaatit
  - Järjestyksestä poikkeavat paketit
  - Tilastoraportti 30s välein

### 6.4 Suorituskykyseuranta

- [ ] **Asetukset**
  ```cpp
  #define ENABLE_PERFORMANCE_MONITOR true
  ```

- [ ] **Toiminta**
  - Silmukan taajuus (Hz)
  - CPU-käyttö (%)
  - Muistin kulutus
  - Raportti 60s välein

### 6.5 Watchdog-ajastin

- [ ] **Asetukset**
  ```cpp
  #define ENABLE_WATCHDOG true
  ```

- [ ] **Toiminta**
  - Järjestelmä ei jumitu
  - Automaattinen uudelleenkäynnistys jos ei vastausta 10s

---

## 🧪 Osa 7: Signaalin laadun testaus

### Etäisyystestit

#### Lähietäisyys (0-10m)
- [ ] RSSI > -70 dBm
- [ ] SNR > 5 dB
- [ ] Pakettihäviö < 1%
- [ ] Yhteys: CONNECTED

#### Keskietäisyys (10-100m)
- [ ] RSSI: -70 to -90 dBm
- [ ] SNR: 0 to 5 dB
- [ ] Pakettihäviö < 5%
- [ ] Yhteys: CONNECTED tai WEAK

#### Pitkä etäisyys (100m+)
- [ ] RSSI < -90 dBm
- [ ] SNR < 0 dB
- [ ] Pakettihäviö < 10% (näköyhteys)
- [ ] Yhteys: WEAK tai LOST (esteiden kanssa)

### Signaalin häiriötestit

- [ ] **Metallieste välissä**
  - RSSI laskee
  - Pakettihäviö kasvaa
  - Yhteys palautuu esteen poistamisen jälkeen

- [ ] **Sisätilat (seinät)**
  - Testaa eri huoneissa
  - Kirjaa RSSI/SNR jokaisessa paikassa

- [ ] **Ulkotilat (näköyhteys)**
  - Maksimietäisyys (SF12)
  - Kirjaa milloin yhteys katkeaa

---

## 📊 Osa 8: Pakettihäviön seuranta

### Sekvenssinnumeroiden testaus

- [ ] **Normaali toiminta**
  - Sekvenssit kasvavat: 0, 1, 2, 3, 4...
  - Ei puuttuvia numeroita
  - Pakettihäviö: 0.0%

- [ ] **Heikko signaali**
  - Siirrä laitteet kauemmaksi
  - Huomaa puuttuvat sekvenssit: 0, 1, 3, 5...
  - Serial: `⚠️ Packet loss: X% (Y/Z lost)`

- [ ] **Yhteyden katkeaminen**
  - Sammuta lähettäjä
  - Vastaanottaja: `🔴 Connection LOST`
  - Käynnistä lähettäjä uudelleen
  - Yhteys palautuu: `✓ Connection restored`

### Pakettihäviön laskenta

```
Häviöprosentti = (Puuttuvat paketit / Odotetut paketit) × 100%
```

- [ ] Lasketaan oikein Serial Monitorissa
- [ ] Päivittyy reaaliajassa
- [ ] Nollautuu uudelleenkäynnistyksessä

---

## 🛠️ Osa 9: Vianhaku-testit

### 9.1 LoRa-moduulin uudelleenalustus

- [ ] Irrota LoRa-moduulin virta
- [ ] ESP32 pysyy käynnissä
- [ ] Yhdistä LoRa-moduuli takaisin
- [ ] Käynnistä ESP32 uudelleen
- [ ] LoRa alustuu: `✓ LoRa initialized`

### 9.2 Kill-switch testit

- [ ] **Fyysinen kill-switch**
  - GPIO 13 ↔ GPIO 14
  - Pidä 3s
  - Serial: `🔴 RESTART: Physical kill-switch`
  - Laite käynnistyy uudelleen

- [ ] **Etä-kill-switch (jos käytössä)**
  - Lähetä LoRa-komento: `CMD:RESTART`
  - Laite käynnistyy uudelleen

### 9.3 Muistin käyttö

- [ ] **Vapaan muistin tarkistus**
  - Ota käyttöön: `ENABLE_EXTENDED_TELEMETRY true`
  - Serial: `Free heap: XX KB`
  - Varmista että muisti ei lopu käytön aikana

- [ ] **Pitkäaikaistesti**
  - Anna järjestelmän pyöriä 1+ tunti
  - Tarkista että ei muistivuotoja
  - Free heap pysyy vakaana

---

## ✅ Lopputarkistus

### Minimikonfiguraatio (oletus)
```cpp
#define ENABLE_DISPLAY_OUTPUT true       // ✅
#define ENABLE_BIDIRECTIONAL true        // ✅
#define ENABLE_CSV_OUTPUT true           // ✅
// Kaikki muut: false
```

**Tarkistuslista:**
- [ ] LoRa-kommunikaatio toimii
- [ ] TFT-näyttö päivittyy
- [ ] ACK-viestit toimivat
- [ ] CSV-data tulostetaan
- [ ] Kill-switch toimii
- [ ] Ei virheviestejä Serial Monitorissa

### Laajennettu konfiguraatio

Lisäksi:
```cpp
#define ENABLE_EXTENDED_TELEMETRY true   // ✅
#define ENABLE_PACKET_STATS true         // ✅
```

**Tarkistuslista:**
- [ ] Uptime, heap, lämpötila näkyvät
- [ ] Pakettitilastot raportoidaan
- [ ] Kaikki perustoiminnot toimivat

---

## 📝 Testausraportti

**Projekti:** Roboter Gruppe 9
**Päivämäärä:** _______________
**Testaaja:** _______________

**Laitteisto:**
- ESP32 #1 (Lähettäjä): _______________
- ESP32 #2 (Vastaanottaja): _______________
- ESP32-2432S022 (Näyttö): _______________
- LoRa-moduulit: RYLR896 868 MHz

**Testin tulos:**
- Ydintoiminnot: ✅ / ❌
- Näyttö: ✅ / ❌
- PC-datan tallennus: ✅ / ❌
- Valinnaiset ominaisuudet: ✅ / ❌ / ⏭️

**Huomiot:**
_______________________________________
_______________________________________
_______________________________________

**Signaalin laatu:**
- Etäisyys: _____ m
- RSSI: _____ dBm
- SNR: _____ dB
- Pakettihäviö: _____ %

**Suositus:**
- [ ] Valmis tuotantokäyttöön
- [ ] Vaatii lisätestausta
- [ ] Vaatii korjauksia

---

## 🎯 Testausstrategia

### Uusille käyttäjille

**Päivä 1:** Perustoiminnot
1. Lataa koodi, testaa LoRa
2. Tarkista roolintunnistus
3. Varmista viestien vaihto

**Päivä 2:** Näyttö
1. Kytke TFT-näyttö
2. Testaa datan näkyminen
3. Tarkista signaalipalkki

**Päivä 3:** Lisäominaisuudet
1. Testaa PC-datan tallennus
2. Kokeile ACK-viestejä
3. Testaa kill-switch

### Kokeneille käyttäjille

**Nopea tarkistus (15 min):**
1. Lataa koodi molempiin
2. Tarkista Serial Monitor
3. Testaa TFT-näyttö
4. Varmista CSV-data
5. Kokeile etäisyystestejä

**Kattava testaus (1-2h):**
1. Kaikki ydintoiminnot
2. Kaikki näyttö-ominaisuudet
3. PC-datan tallennus
4. Signaalin laadun testit
5. Pakettihäviön testit
6. Pitkäaikaistesti (1h+)

---

## 💡 Testaustoiminnon vinkit

1. **Testaa minimalistisesti ensin**
   - Poista kaikki valinnaiset ominaisuudet
   - Saa LoRa toimimaan
   - Lisää ominaisuuksia yksi kerrallaan

2. **Käytä Serial Monitoria ahkerasti**
   - Välttämätön vianhakuun
   - Näyttää RSSI, SNR, virheet
   - Sekä lähettäjä että vastaanottaja

3. **Tarkista kytkennät**
   - Useimmat ongelmat ovat kytkentöjä
   - Tarkista multimittarilla tarvittaessa
   - Yhteinen GND on kriittinen!

4. **Säästä virtaa**
   - Poista käyttämättömät ominaisuudet
   - Kasvata lähetysväliä
   - Laske LoRa TX-teho (tarvittaessa)

---

**Onnea testaukseen! 🚀**

*Viimeksi päivitetty: 14.11.2025*
