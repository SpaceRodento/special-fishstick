# Hardware Testing Checklist

**Roboter Gruppe 9 - Laitteistotestauslista**

> 📋 Käytä tätä listaa varmistaaksesi, että kaikki ominaisuudet toimivat oikein ESP32-laitteissa.
>
> ✅ = Testattu ja toimii
> ⚠️ = Testattu, löytyi ongelmia
> 🔲 = Ei vielä testattu

---

## 📡 Perustoiminnot

### LoRa-moduuli (RYLR896)

- [ ] **LoRa-moduuli havaitaan käynnistyksessä**
  - Odotus: `✓ LoRa initialized` serialissa
  - Jos epäonnistuu: Tarkista kytkennät (TX→GPIO25, RX→GPIO26)

- [ ] **Osoite asetetaan oikein**
  - Sender: ADDRESS+5
  - Receiver: ADDRESS+6
  - Tarkista serialista: `✓ LoRa address set: X`

- [ ] **Parametrit asetetaan (SF12, BW125, CR4/5)**
  - Odotus: `✓ LoRa parameters set`

### Roolin tunnistus

- [ ] **Receiver tunnistetaan oikein**
  - GPIO 16 ↔ GPIO 17 jumpperilla
  - Odotus serialissa: `🔵 RECEIVER MODE`
  - LCD pitäisi näyttää: `Receiver Ready`

- [ ] **Sender tunnistetaan oikein**
  - GPIO 16 ilman kytkentää (floating)
  - Odotus serialissa: `🔴 SENDER MODE`
  - Ei LCD:tä (paitsi jos haluat testata)

---

## 📨 Viestinvälitys

### Yksisuuntainen viestintä (Sender → Receiver)

- [ ] **Sender lähettää viestejä**
  - Serial: `Messages TX: X` (kasvaa)
  - LED vilkkuu lähetyksen yhteydessä

- [ ] **Receiver vastaanottaa viestejä**
  - Serial: `Messages RX: X` (kasvaa)
  - LCD päivittyy
  - LED vilkkuu vastaanotettaessa

- [ ] **RSSI ja SNR näkyvät**
  - Receiver serial: `RSSI: -XX dBm, SNR: X dB`
  - LCD: Signal-palkit näkyvät

- [ ] **Sekvenssit kasvavat oikein**
  - Serial: `SEQ:X` (0, 1, 2, 3...)

### Kaksisuuntainen viestintä (Bi-directional)

- [ ] **Receiver lähettää ACK-viestejä**
  - Joka 5. viestin jälkeen
  - Receiver serial: `→ Sending ACK`

- [ ] **Sender vastaanottaa ACK-viestejä**
  - Sender serial: `✓ ACK #X received`
  - ACK-laskuri kasvaa

- [ ] **ACK timeout toimii**
  - Jos ACK ei tule 500ms:ssa, jatketaan
  - Serial: `⏱ ACK timeout`

---

## 🖥️ LCD-näyttö (Receiver)

### LCD Version 1 (Wide Bar)

- [ ] **Käynnistyy oikein**
  - Näyttää: `Receiver Ready` ja `Waiting...`

- [ ] **Signal-palkit näkyvät**
  - 10 palkkia vastaavat RSSI-arvoa
  - Parempi signaali = enemmän palkkeja

- [ ] **Spinner animaatio**
  - Paikallinen spinner: `|/-\`
  - Remote spinner näkyy, kun vastaanotetaan

- [ ] **Sekvenssit päivittyvät**
  - Näyttää: `S:XX R:YY`

### LCD Version 2 (Vertical Bars)

- [ ] **Pystypalkit näkyvät**
  - 16 pystypalkkia
  - Täyttöaste vastaa RSSI:tä

### LCD Version 3 (Numbers)

- [ ] **Numeerinen RSSI**
  - Näyttää: `-85dBm 7dB`
  - SNR erikseen

### LCD Version 4 (Advanced)

- [ ] **Health-indikaattorit**
  - Packet loss %
  - Connection state
  - Uptime

---

## 🔗 Connection Watchdog

### Yhteyden tilan muutokset

- [ ] **UNKNOWN → CONNECTING**
  - Käynnistyksen jälkeen
  - Serial: `Connection state: CONNECTING`

- [ ] **CONNECTING → CONNECTED**
  - Kun ensimmäinen viesti vastaanotettu
  - LCD: Näyttää `OK` tai vihreän indikaattorin

- [ ] **CONNECTED → WEAK**
  - Kun viestit viivästyvät 3-8s
  - Serial: `Connection state: WEAK`
  - LCD: Keltainen varoitus

- [ ] **WEAK → LOST**
  - Kun >8s ei viestejä
  - Serial: `Connection state: LOST`
  - LCD: Punainen varoitus

- [ ] **LOST → CONNECTING (Auto-recovery)**
  - Receiver yrittää uudelleenyhdistää
  - 3 yritystä, 5s välein
  - Serial: `📡 Recovery attempt X/3`

### Packet Loss -seuranta

- [ ] **Packet loss lasketaan oikein**
  - Serial: `Packet loss: X.XX%`
  - Kasvaa, jos viestejä puuttuu
  - LCD: Näyttää % (Version 4)

- [ ] **Sequence errors havaitaan**
  - Jos sequence hyppää: esim. 10→12 (puuttuu 11)
  - Serial: `⚠ Missed sequence: expected X, got Y`

---

## 🔴 Kill-Switch

### Fyysinen Kill-Switch

- [ ] **GPIO 13 ↔ 14 yhdistäminen havaitaan**
  - 2s välein: `🔴 Kill-switch: X seconds`

- [ ] **3 sekunnin pito käynnistää uudelleen**
  - Countdown: `3... 2... 1...`
  - Serial: `🔴 KILL SWITCH ACTIVATED - RESTARTING...`
  - ESP32 käynnistyy uudelleen

- [ ] **Kill-switch toimii myös jos LoRa ei toimi**
  - Katkaise LoRa-moduulin virta
  - Kill-switch pitää silti toimia

### Remote Kill-Switch

- [ ] **CMD:RESTART-komento toimii**
  - Lähetä komento toiselta laitteelta
  - Receiver: `🔴 REMOTE RESTART COMMAND`
  - Käynnistyy uudelleen

- [ ] **Turvallisuusviive (100ms)**
  - Komennon jälkeen 100ms viive
  - Antaa aikaa lähettää vahvistus

---

## 💻 PC-dataloggaus

### CSV-muotoinen output

- [ ] **CSV-data tulostetaan 2s välein**
  - Muoto: `TIMESTAMP,RSSI,SNR,SEQ,LED,TOUCH,STATE,LOSS`
  - Esim: `12345,-85,7,42,1,0,OK,1.23`

- [ ] **CSV on parsittavissa Pythonilla**
  - Testaa: `python data_logger.py /dev/ttyUSB0`

### Serial Monitor (serial_monitor.py)

- [ ] **Serial monitor käynnistyy**
  - Komento: `python serial_monitor.py /dev/ttyUSB0`
  - Näyttää värillisen outputin

- [ ] **RSSI-palkit näkyvät**
  - Signal quality bars terminaalissa

- [ ] **CSV-data tunnistetaan**
  - Erottuu debug-viesteistä

### Data Logger (data_logger.py)

- [ ] **SQLite-tietokanta luodaan**
  - Tiedosto: `lora_data.db`
  - Sisältää taulut: `lora_messages`, `events`

- [ ] **Viestit tallennetaan tietokantaan**
  - Jokainen CSV-rivi → database row
  - Tarkista: `sqlite3 lora_data.db "SELECT COUNT(*) FROM lora_messages;"`

- [ ] **Timestamp oikein**
  - Sekä PC:n timestamp että ESP32:n millis()

### Real-time Plotter (realtime_plotter.py)

- [ ] **Graafinen ikkuna avautuu**
  - 4 kuvaajaa: RSSI, SNR, Packet Loss, Connection State

- [ ] **Kuvaajat päivittyvät reaaliajassa**
  - 100ms välein
  - Näyttää viimeiset 100 datapistettä

- [ ] **Värit ja vyöhykkeet näkyvät**
  - RSSI: Vihreä/keltainen/punainen vyöhykkeet
  - Connection state: Värikoodit

- [ ] **Status bar päivittyy**
  - Ylhäällä: Packets, RSSI, SNR, Loss, State

### Data Analysis (analyze_data.py)

- [ ] **Lataa SQLite-tietokannan**
  - Komento: `python analyze_data.py lora_data.db`

- [ ] **Tilastot tulostetaan**
  - RSSI mean/median/min/max
  - SNR statistics
  - Packet loss
  - Connection states

- [ ] **4 kuvaajaa luodaan**
  - RSSI timeline
  - Packet loss
  - RSSI histogram
  - Connection states

- [ ] **PDF-raportti (valinnainen)**
  - `python analyze_data.py lora_data.db --output report.pdf`

### Jupyter Notebook (lora_analysis_notebook.ipynb)

- [ ] **Notebook avautuu**
  - `jupyter notebook lora_analysis_notebook.ipynb`

- [ ] **Datan lataus toimii**
  - Cell 2: Lataa SQLite-tietokannan

- [ ] **Kuvaajat renderöityvät**
  - Kaikki 11 analyysikohtaa

---

## 🔧 Konfiguraatio

### config.h -asetukset

- [ ] **SEND_INTERVAL muuttaminen**
  - Testaa: 1000ms, 2000ms, 5000ms
  - Varmista että viestit tulevat oikealla intervallilla

- [ ] **LCD_VERSION vaihtaminen**
  - Testaa kaikki 4 versiota: V1, V2, V3, V4
  - Jokainen pitäisi toimia

- [ ] **ENABLE_BIDIRECTIONAL**
  - true: ACK-viestit lähetetään
  - false: Vain yksisuuntainen

- [ ] **ACK_INTERVAL muuttaminen**
  - Testaa: 2, 5, 10
  - Varmista että ACK lähetetään oikealla välillä

- [ ] **CONNECTION_TIMEOUT**
  - Testaa: 3000ms, 5000ms, 8000ms
  - LOST-tila tulee oikealla ajalla

- [ ] **RECOVERY_ATTEMPTS**
  - Testaa: 1, 3, 5
  - Varmista että yritysten määrä on oikein

---

## 🏗️ Rangetestauk set

### Lähietäisyys (0-10m)

- [ ] **RSSI parempi kuin -70 dBm**
  - Yleensä: -50 ... -70 dBm

- [ ] **Packet loss < 1%**

- [ ] **Connection state: OK**

### Keskietäisyys (10-100m)

- [ ] **RSSI välillä -70 ... -90 dBm**

- [ ] **Packet loss < 5%**

- [ ] **Connection state: OK tai WEAK**

### Pitkä etäisyys (100m-1km+)

- [ ] **RSSI heikompi kuin -90 dBm**
  - Jopa -110 ... -120 dBm

- [ ] **Packet loss 5-20%**

- [ ] **Connection state: WEAK**
  - Mahdollisesti: LOST ja recovery

### Esteet

- [ ] **Betoniseinien läpi**
  - RSSI heikkenee ~20-30 dBm
  - Packet loss kasvaa

- [ ] **Metalliesteiden läpi**
  - Signaali voimakkaasti heikkenee
  - Mahdollisesti: Connection lost

---

## 🔋 Pitkäaikaistestit

### 1 tunnin testi

- [ ] **Ei kaatumisia/uudelleenkäynnistyksiä**
  - Seuraa serialia
  - ESP32 ei saa reboot:ata

- [ ] **Packet loss pysyy vakaana**
  - Ei kasva jatkuvasti

- [ ] **Memory leak -testaus**
  - Serial: Ei "Low memory" -varoituksia
  - ESP32 free heap pysyy vakaana

### 4 tunnin testi

- [ ] **Yhteydenpito katkeamaton**
  - Ei pitkiä LOST-tiloja (paitsi tarkoituksella)

- [ ] **LCD toimii edelleen**
  - Ei jumittuneita pikselihitä

### 24 tunnin testi (valinnainen)

- [ ] **Järjestelmä pysyy vakaana**

- [ ] **SQLite-tietokanta kasvaa järkevästi**
  - ~1800 riviä/tunti (jos 2s interval)
  - 24h = ~43,000 riviä
  - Tiedostokoko: ~5-10 MB

---

## 🐛 Error-tilanteet

### LoRa-moduuli irrotetaan

- [ ] **Käynnistys epäonnistuu graafullisesti**
  - Serial: `❌ LoRa init failed!`
  - Serial: `⚠️ Continuing anyway - kill-switch still works!`
  - **EI** jumitu while(1) -looppiin

- [ ] **Kill-switch toimii silti**
  - GPIO 13↔14 pito käynnistää uudelleen

### Receiver sammutetaan

- [ ] **Sender havaitsee connection loss:n**
  - ACK timeout
  - Ei viestejä vastaan

### Sender sammutetaan

- [ ] **Receiver menee LOST-tilaan**
  - 8 sekunnin jälkeen: `Connection state: LOST`

- [ ] **Auto-recovery yrittää yhdistää**
  - 3 yritystä, 5s välein
  - Serial: `📡 Recovery attempt X/3`

### WiFi/Bluetooth-häiriöt

- [ ] **LoRa toimii normaalisti**
  - 868 MHz ≠ 2.4 GHz (WiFi/BT)

### Virransyöttö heikko

- [ ] **Brownout detection**
  - ESP32 saattaa reboot:ata
  - Serial: `Brownout detector was triggered`

---

## 📊 Suorituskyky

### Message Rate

- [ ] **2s interval toimii vakaasti**
  - 30 viestiä/minuutti
  - 1800 viestiä/tunti

- [ ] **1s interval toimii**
  - 60 viestiä/minuutti
  - Mahdollisesti enemmän collisioneja

- [ ] **500ms interval**
  - 120 viestiä/minuutti
  - Air time usage ~16%
  - Mahdollisesti packet loss kasvaa

### Memory Usage

- [ ] **Free heap vakaa**
  - Serial: Check ESP.getFreeHeap()
  - Pitäisi olla >100 KB

- [ ] **Ei memory leakeja**
  - Minimum free heap ei laske ajan myötä

### CPU Usage

- [ ] **Loop frequency**
  - Normaali: ~100-1000 Hz
  - Ei saa olla <10 Hz (liian hidas)

---

## 🎛️ Konfigurointikohteet

### Muuttujat joita kannattaa testata:

1. **SEND_INTERVAL** (config.h)
   - [ ] 500ms
   - [ ] 1000ms
   - [ ] 2000ms (default)
   - [ ] 5000ms

2. **LCD_VERSION** (config.h)
   - [ ] LCD_VERSION_1 (Wide bar)
   - [ ] LCD_VERSION_2 (Vertical bars)
   - [ ] LCD_VERSION_3 (Numbers)
   - [ ] LCD_VERSION_4 (Advanced)

3. **ENABLE_BIDIRECTIONAL** (config.h)
   - [ ] true (ACK enabled)
   - [ ] false (One-way only)

4. **ACK_INTERVAL** (config.h)
   - [ ] 2 (ACK every 2 messages)
   - [ ] 5 (default)
   - [ ] 10

5. **SPREADING_FACTOR** (lora_handler.h)
   - [ ] SF12 (default, max range)
   - [ ] SF10 (faster, less range)
   - [ ] SF7 (fastest, shortest range)

---

## 📝 Testaustulokset

### Käytä tätä osiota omille muistiinpanoillesi:

**Päivämäärä:** ___________

**Testausympäristö:**
- Sisätila / Ulkotila
- Etäisyys: ______ metriä
- Esteet: _________________

**RSSI-arvot:**
- Keskiarvo: ______ dBm
- Min: ______ dBm
- Max: ______ dBm

**SNR-arvot:**
- Keskiarvo: ______ dB

**Packet loss:**
- Prosentti: ______ %

**Ongelmat:**
_________________________________
_________________________________
_________________________________

**Huomiot:**
_________________________________
_________________________________
_________________________________

---

## ✅ Yhteenveto

Kun kaikki kohdat on testattu, sinulla on:

✅ Toimiva LoRa-kommunikaatio
✅ Vakaa yhteys ja auto-recovery
✅ Kill-switch turvallisuustoiminto
✅ PC-dataloggaus ja visualisointi
✅ Kaksisuuntainen viestintä
✅ Kattava dokumentaatio

**Seuraavat vaiheet:**
1. Deployment tuotantoympäristöön
2. Pitkäaikainen seuranta
3. Mahdolliset optimoinnit

---

**Onnea testauksiin! 🚀**
