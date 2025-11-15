# Roboter Gruppe 9 - Manual

**LoRa Communication System with TFT Display**

ESP32-based LoRa wireless communication with automatic role detection, real-time TFT display, and comprehensive signal monitoring.

Last updated: 2025-11-14

---

## 📖 Sisällysluettelo

1. [Projektin Yleiskatsaus](#projektin-yleiskatsaus)
2. [Laitteisto](#laitteisto)
3. [Pikaohje](#pikaohje)
4. [Ohjelmiston Rakenne](#ohjelmiston-rakenne)
5. [Konfigurointi](#konfigurointi)
6. [LoRa-asetukset](#lora-asetukset)
7. [TFT-näyttö](#tft-näyttö)
8. [PC-datan Tallennus](#pc-datan-tallennus)
9. [Vianmääritys](#vianmääritys)

---

## Projektin Yleiskatsaus

### Mikä tämä on?

Täysi langaton kommunikaatiojärjestelmä kahdelle ESP32-mikrokontrollerille LoRa-radioteknologialla. Kommunikaatioetäisyys jopa useita kilometrejä.

**Pääominaisuudet:**
- **Plug-and-play** - Identtinen koodi molempiin laitteisiin, rooli tunnistetaan automaattisesti
- **TFT-näyttö** - Erillinen 320x240 värinäyttö (ESP32-2432S022)
- **Kaksisuuntainen** - Molemmat laitteet lähettävät ja vastaanottavat (ACK-tuki)
- **Itseparantuva** - Automaattinen palautuminen yhteysvirheistä
- **Signaalin seuranta** - Reaaliaikainen RSSI, SNR, pakettihäviö
- **Kill-switch** - Fyysinen ja etä-hätäpysäytys
- **Datan tallennus** - Python-skriptit PC:lle

**Käyttökohteet:**
- Robotin etäohjaus telemetrialla
- Ympäristön sensorit (lämpötila, kosteus)
- Rakennusautomaatio
- Maatalouden seuranta
- Etälaitteiden ohjaus

---

## Laitteisto

### Tarvittavat Komponentit

**Perusjärjestelmä (2 kpl ESP32):**
- 2× ESP32 DevKit v1
- 2× RYLR896 LoRa-moduuli (868 MHz)
- Hyppylankalanka (roolivalinta)
- USB-kaapelit

**TFT-näyttöasema (valinnainen):**
- 1× ESP32-2432S022 (2.4" TFT 320x240)
- 2 johtoa (TX, GND) + oma USB-virtalähde

**Lisäanturit (valinnaiset):**
- I2C LCD 16x2 (vastaanottajalle)
- Jännitejakaja (akkuseuranta)
- MAX4466 mikrofoni (äänentunnistus)
- TCS34725 värisensori (valontunnistus)
- INA219 virtamittari

### Kytkennät

#### RYLR896 LoRa-moduuli
```
RYLR896 → ESP32
─────────────────
TX      → GPIO 25 (RXD2)
RX      → GPIO 26 (TXD2)
VCC     → 3.3V
GND     → GND
```

#### Roolivalinta (Mode Detection)
```
GPIO 17 → Asetetaan OUTPUT LOW (tarjoaa GND)
GPIO 16 → Luetaan INPUT_PULLUP:lla

VASTAANOTTAJA: Yhdistä GPIO 16 ↔ GPIO 17 hyppylangalla
LÄHETTÄJÄ:     Jätä GPIO 16 irti (ei yhteyttä)

Huom: GPIO 16 ja 17 ovat vierekkäin!
```

#### Kill-Switch
```
GPIO 14 → Asetetaan OUTPUT LOW (tarjoaa GND)
GPIO 13 → Luetaan INPUT_PULLUP:lla

Uudelleenkäynnistys: Yhdistä GPIO 13 ↔ GPIO 14 ja pidä 3 sekuntia
```

#### TFT-näyttöasema
```
Robot ESP32          →  Display ESP32-2432S022
──────────────────────────────────────────────
GPIO 23 (TX)         →  GPIO 18 (RX)
GND                  →  GND

Huom: Display-ESP32 saa virran OMASTA USB-kaapelista!
```

#### I2C LCD (valinnainen, vain vastaanottajalla)
```
LCD → ESP32
────────────
SDA → GPIO 21
SCL → GPIO 22
VCC → 5V
GND → GND
I2C-osoite: 0x27
```

---

## Pikaohje

### 10 minuutin käyttöönotto

**Vaihe 1: Yhdistä LoRa-moduulit**
- Yhdistä molemmat RYLR896:t ESP32:iin (katso yllä)

**Vaihe 2: Lataa koodi**
1. Avaa `Roboter_Gruppe_9.ino`
2. Valitse: **ESP32 Dev Module**
3. Lataa **MOLEMPIIN** ESP32:iin (sama koodi!)

**Vaihe 3: Aseta roolit**
- **Vastaanottaja:** Yhdistä GPIO 16 ↔ GPIO 17 hyppylangalla
- **Lähettäjä:** Jätä GPIO 16 irti

**Vaihe 4: Testaa**
1. Käynnistä molemmat
2. Avaa Serial Monitor (115200 baud) molemmille
3. Tarkista:
   - Lähettäjä: `>>> SENDER MODE` ja `📤 TX [1]: SEQ:0...`
   - Vastaanottaja: `>>> RECEIVER MODE` ja `📥 RX [1]: SEQ:0...`

**Vaihe 5: TFT-näyttö (valinnainen)**
1. Lataa `Roboter_Display_TFT.ino` → ESP32-2432S022
2. Yhdistä: Robot GPIO 23 → Display GPIO 18 ja GND → GND
3. Varmista `config.h`: `#define ENABLE_DISPLAY_OUTPUT true`
4. Lataa koodi uudelleen robottiin

**Valmis!** Järjestelmä toimii.

---

## Ohjelmiston Rakenne

### Hakemistorakenne
```
Roboter_Gruppe_9/
├── Roboter_Gruppe_9.ino    # Pääohjelma
├── config.h                # Konfiguraatio ja kytkennät
├── structs.h               # Datarakenteet
├── functions.h             # Apufunktiot
├── lora_handler.h          # LoRa-kommunikaatio
├── health_monitor.h        # Yhteyden valvonta
├── display_sender.h        # TFT-näytön UART-lähetys
├── DisplayClient.h         # Näyttökirjasto
├── MANUAL.md               # Tämä tiedosto
├── TESTING.md              # Testausohjeet
└── Python-skriptit/        # PC-datan tallennus

Roboter_Display_TFT/
└── Roboter_Display_TFT.ino # TFT-näytön koodi
```

### Tiedostot

**Ydinohjelma:**
- `Roboter_Gruppe_9.ino` (789 riviä) - Pääohjelma, setup(), loop()
- `config.h` (182 riviä) - Kaikki asetukset yhdessä paikassa
- `structs.h` (111 riviä) - Datarakenteet
- `functions.h` (145 riviä) - LCD ja apufunktiot
- `lora_handler.h` (264 riviä) - LoRa-kommunikaatio
- `health_monitor.h` (310 riviä) - Yhteyden valvonta ja tilastointi

**Näyttö:**
- `display_sender.h` (243 riviä) - UART-lähetys TFT:lle
- `DisplayClient.h` (194 riviä) - Näyttökirjasto
- `Roboter_Display_TFT.ino` (607 riviä) - TFT-näytön ohjelma

**Python:**
- `serial_monitor.py` - Reaaliaikainen värikäs seuranta
- `data_logger.py` - SQLite-tietokantaan tallennus

### Muistinkäyttö
- **Flash:** ~250 KB
- **RAM:** ~45 KB

---

## Konfigurointi

### config.h - Keskitetty konfiguraatio

Kaikki asetukset löytyvät `config.h` -tiedostosta. Muokkaa tätä yhtä tiedostoa.

#### Näyttö-ominaisuudet
```cpp
#define ENABLE_DISPLAY_OUTPUT true      // TFT-näyttöasema
#define DISPLAY_UPDATE_INTERVAL 2000    // Päivitys 2s välein
#define DISPLAY_TX_PIN 23               // TX-pinni näytölle
```

#### Kommunikaatio
```cpp
#define ENABLE_BIDIRECTIONAL true       // Kaksisuuntainen (ACK)
#define ACK_INTERVAL 5                  // ACK joka 5. viesti
#define LISTEN_TIMEOUT 500              // ACK odotus 500ms
```

#### PC-datan tallennus
```cpp
#define ENABLE_CSV_OUTPUT true          // CSV-muoto
#define DATA_OUTPUT_INTERVAL 2000       // Lähetys 2s välein
```

#### Valinnaiset anturit (oletuksena pois päältä)
```cpp
#define ENABLE_BATTERY_MONITOR false    // Akkuseuranta
#define ENABLE_AUDIO_DETECTION false    // Äänentunnistus
#define ENABLE_LIGHT_DETECTION false    // Valontunnistus
#define ENABLE_CURRENT_MONITOR false    // Virtamittaus
```

#### Järjestelmä-ominaisuudet
```cpp
#define ENABLE_EXTENDED_TELEMETRY false // Lisätiedot (uptime, heap, lämpötila)
#define ENABLE_PACKET_STATS false       // Yksityiskohtaiset tilastot
#define ENABLE_PERFORMANCE_MONITOR false// CPU/muisti-seuranta
#define ENABLE_WATCHDOG false           // Laitteisto-watchdog
```

**Vinkki:** Testaa ensin kaikki `false`, sitten kytke yksi kerrallaan `true`:ksi.

---

## LoRa-asetukset

### Optimoidut asetukset maksimietäisyydelle

```cpp
// config.h:
#define LORA_NETWORK_ID 6               // Verkkotunnus (sama molemmissa!)
#define LORA_ADDRESS_RECEIVER 1         // Vastaanottajan osoite
#define LORA_ADDRESS_SENDER 2           // Lähettäjän osoite
#define LORA_SPREADING_FACTOR 12        // SF12 = max etäisyys
#define LORA_BANDWIDTH 125              // 125 kHz
#define LORA_CODING_RATE 1              // 4/5
#define LORA_TX_POWER 20                // 20 dBm = max teho
```

**Spreading Factor (SF):**
- SF7 = nopea, lyhyt kantama
- SF12 = hidas, pitkä kantama (oletusarvo)

**Etäisyysarviot (SF12):**
- Lähellä (0-10m): RSSI > -70 dBm, pakettihäviö < 1%
- Keskietäisyys (10-100m): RSSI -70 to -90 dBm
- Pitkä (100m+): RSSI < -90 dBm, tarvitsee näköyhteyden

### Viestiformaatti

**Lähettäjä → Vastaanottaja:**
```
SEQ:42,LED:1,TOUCH:0,SPIN:2,COUNT:42
```

**Vastaanottaja → Lähettäjä (ACK):**
```
ACK,SEQ:5,LED:0,TOUCH:1,SPIN:3
```

### Signaalin laatu

**RSSI (Received Signal Strength Indicator):**
- -40 dBm = erinomainen (lähellä)
- -70 dBm = hyvä
- -90 dBm = heikko
- -120 dBm = huono (yhteys katkeaa pian)

**SNR (Signal-to-Noise Ratio):**
- +10 dB = erinomainen
- 0 dB = hyvä
- -10 dB = heikko
- -20 dB = huono

---

## TFT-näyttö

### ESP32-2432S022 TFT Display

**Tekniset tiedot:**
- Näyttö: 2.4" ST7789 TFT (320×240, landscape)
- Värit: 65k (16-bit RGB)
- Rajapinta: 8-bit parallel
- Kirjasto: LovyanGFX

### Kytkentä

**TÄRKEÄÄ: Vain 2 johtoa + virta erikseen!**

```
Robot ESP32          Display ESP32-2432S022
──────────────      ─────────────────────────
GPIO 23 (TX)    →   GPIO 18 (RX)
GND             →   GND

                    Oma USB-virta näytölle!
```

**ÄLÄ** syötä virtaa robotin ESP32:sta näytölle!

### Näytön asettelut

Nykyinen versio jakaa näytön kolmeen osaan:
1. **Header (yläosa, 30px):** Otsikko, LED-indikaattori, yhteyden tila
2. **Data (keskiosa, 180px):** Päätiedot ja signaalipalkki
3. **Alert (alaosa, 40px):** LoRa-tila ja signaalin laatutiedot

### Värit ja fontit

**Fontit (config.h):**
```cpp
#define FONT_SMALL 1       // Pienet tiedot, otsikot
#define FONT_NORMAL 2      // Pääteksti
#define FONT_LARGE 4       // Isot numerot
```

**Värit (RGB565):**
```cpp
#define COLOR_BG 0x0000         // Musta tausta
#define COLOR_HEADER 0x001F     // Sininen
#define COLOR_TEXT 0xFFFF       // Valkoinen
#define COLOR_LABEL 0x8410      // Harmaa
#define COLOR_GOOD 0x07E0       // Vihreä (hyvä)
#define COLOR_WARN 0xFD20       // Oranssi (varoitus)
#define COLOR_BAD 0xF800        // Punainen (huono)
```

### Signaalin laatupalkki

Oikeassa reunassa näkyy pystysuora palkki:
- Vihreä (70-100%): Erinomainen signaali
- Oranssi (40-69%): Keskinkertainen
- Punainen (0-39%): Heikko

Lasketaan RSSI:n ja SNR:n perusteella.

### TFT-näytön muokkaaminen

**Tiedosto:** `Roboter_Display_TFT/Roboter_Display_TFT.ino`

Tämä osio opastaa miten voit muokata TFT-näytön ulkoasua ja sisältöä.

#### 1. Layoutin rakenne ja koordinaatit

Näyttö on 320×240 pikseliä (landscape-tila). Layout on määritelty riveillä 141-180.

**Kolme pääaluetta:**

```cpp
// ┌────────────────────────────────────────┐
// │  YLÄ-OSA (Header) - 30px               │  Otsikko, LED, yhteys
// ├──────────────────┬─────────────────────┤
// │                  │                     │
// │  VASEN SARAKE    │   OIKEA SARAKE      │  Data jaettu kahtia
// │  - Aikaleima     │   - RSSI, SNR       │
// │  - Paketit       │   - Pakettihäviö    │  + Signaalipalkki
// │  - Aika viime    │   - Laatuprosentti  │    oikealla
// │                  │                     │
// │  KESKI-OSA (Data) - 150px              │
// ├────────────────────────────────────────┤
// │  ALA-OSA (Footer) - 60px               │  LoRa ONLINE + tiedot
// │  LoRa ONLINE                           │
// │  -85dBm | Addr:1 | RX                  │
// └────────────────────────────────────────┘
```

**Muokattavat koordinaatit (rivit 160-180):**

```cpp
// Ylä-osa (Header)
#define HEADER_Y        0        // Y-koordinaatti
#define HEADER_H        30       // Korkeus pikseleinä

// Keski-osa (Data) - jaettu vasempaan ja oikeaan
#define DATA_Y          30       // Alkaa headerin jälkeen
#define DATA_H          150      // Korkeus
#define DATA_LEFT_X     10       // Vasen sarake alkaa
#define DATA_LEFT_W     130      // Vasen sarake leveys
#define DATA_RIGHT_X    150      // Oikea sarake alkaa
#define DATA_RIGHT_W    120      // Oikea sarake leveys

// Ala-osa (Footer)
#define FOOTER_Y        180      // Alkaa datan jälkeen
#define FOOTER_H        60       // Korkeus

// Signaalipalkki (oikeassa reunassa)
#define SIGNAL_BAR_X    280      // X-koordinaatti
#define SIGNAL_BAR_Y    (DATA_Y + 10)
#define SIGNAL_BAR_W    30       // Leveys
#define SIGNAL_BAR_H    (DATA_H - 20)
```

**Esimerkki 1: Suurempi header**
```cpp
#define HEADER_H        50       // Oli 30 → nyt 50
#define DATA_Y          50       // Päivitä tämäkin!
```

**Esimerkki 2: Leveämpi vasen sarake**
```cpp
#define DATA_LEFT_W     180      // Oli 130 → nyt 180
#define DATA_RIGHT_X    200      // Siirrä oikeaa vastaavasti
```

#### 2. Värien muokkaaminen

**Värit määritellään RGB565-formaatissa** (rivit 79-103). RGB565 on 16-bittinen värimuoto.

**Värimuunnin:** https://rgbcolorpicker.com/565

**Perusvärit:**
```cpp
#define COLOR_BG           0x0000    // Musta tausta
#define COLOR_PRIMARY      0x001F    // Sininen (pääväri, header)
#define COLOR_SECONDARY    0xFD20    // Oranssi (toissijainen)
#define COLOR_TEXT_PRIMARY 0xFFFF    // Valkoinen teksti
#define COLOR_TEXT_SECONDARY 0x8410  // Harmaa teksti (labelit)
```

**Tilaindikaattorit:**
```cpp
#define COLOR_SUCCESS      0x07E0    // Vihreä (hyvä signaali)
#define COLOR_WARNING      0xFD20    // Oranssi (varoitus)
#define COLOR_ERROR        0xF800    // Punainen (virhe)
```

**Esimerkki 1: Sinisen headerin sijaan violetti**
```cpp
#define COLOR_PRIMARY      0x781F    // Violetti (R=15, G=0, B=31)
```

**Esimerkki 2: Tummansininen tausta valkoisensijaan musta**
```cpp
#define COLOR_BG           0x0010    // Tummansininen (R=0, G=2, B=0)
```

**RGB565-laskenta:**
- **Punainen (R):** 5 bittiä (0-31) → kerro 2048
- **Vihreä (G):** 6 bittiä (0-63) → kerro 32
- **Sininen (B):** 5 bittiä (0-31) → kerro 1
- **Esim. oranssi:** R=31, G=20, B=0 → (31×2048) + (20×32) + 0 = 0xFD20

#### 3. Fonttikoot

**Fontit määritellään riveillä 69-73:**

```cpp
#define FONT_SMALL 1     // Pienet tiedot, labelit (8px korkeus)
#define FONT_NORMAL 2    // Pääteksti, luettava data (16px)
#define FONT_LARGE 4     // Isot numerot, otsikot (32px)
```

Numerot ovat **kertoimen arvoja**. LovyanGFX skaalaa perusfontin:
- `1` = 8×8 pikseliä
- `2` = 16×16 pikseliä
- `4` = 32×32 pikseliä
- `6` = 48×48 pikseliä (käytä isoille numeroille!)

**Esimerkki: Suurempi RSSI-numero**

Muokkaa funktiota `drawData()` (rivi 577):
```cpp
// RSSI (FONT_LARGE - iso numero)
tft.setTextSize(6);           // Oli FONT_LARGE (4) → nyt 6
tft.setTextColor(COLOR_TEXT_PRIMARY);
String rssiValue = rssiStr.length() > 0 ? rssiStr.substring(0, rssiStr.indexOf("d")) : "-";
tft.drawString(rssiValue, rightX, rightY);
```

#### 4. Elementtien sijainnin muokkaaminen

**Funktiot jotka piirtävät näytön:**
- `drawHeader()` (rivit 452-502) - Yläosa
- `drawData()` (rivit 504-617) - Keskiosa
- `drawSignalQualityBar()` (rivit 619-660) - Signaalipalkki
- `drawAlert()` (rivit 662-737) - Alaosa (footer)

**Esimerkki: Siirrä LED-indikaattori oikealle**

Muokkaa funktiota `drawHeader()` (rivi 465):
```cpp
int ledX = 250;  // Oli 120 → nyt oikealla puolella
int ledY = HEADER_Y + 15;
```

**Esimerkki: Lisää uusi kenttä vasempaan sarakkeeseen**

Muokkaa funktiota `drawData()` (rivi 570 jälkeen):
```cpp
// Sekvenssinnumero (FONT_SMALL)
tft.setTextSize(FONT_SMALL);
tft.setTextColor(COLOR_LABEL);
tft.drawString("SEQ:", leftX, leftY);
tft.setTextColor(COLOR_TEXT_PRIMARY);
tft.drawString(seqStr.length() > 0 ? seqStr : "-", leftX + 35, leftY);
leftY += 18;

// *** UUSI KENTTÄ: Lämpötila ***
String tempStr = getFieldValue("TEMP");
tft.setTextColor(COLOR_LABEL);
tft.drawString("Temp:", leftX, leftY);
tft.setTextColor(COLOR_TEXT_PRIMARY);
tft.drawString(tempStr.length() > 0 ? tempStr + "C" : "-", leftX + 45, leftY);
leftY += 18;
```

#### 5. LovyanGFX-peruskomennot

**Piirtokomennot:**

```cpp
// Tekstin piirtäminen
tft.drawString("Teksti", x, y);           // Piirrä teksti koordinaatteihin
tft.setTextSize(2);                       // Aseta fonttikoko (1-6)
tft.setTextColor(COLOR_TEXT);             // Aseta tekstin väri
tft.setTextColor(COLOR_TEXT, COLOR_BG);   // Teksti + taustaväri

// Tekstin tasaus (datum)
tft.setTextDatum(TL_DATUM);  // Top-Left (vasen ylä)
tft.setTextDatum(TC_DATUM);  // Top-Center (keskellä ylä)
tft.setTextDatum(TR_DATUM);  // Top-Right (oikea ylä)
tft.setTextDatum(MC_DATUM);  // Middle-Center (keskellä)

// Suorakulmiot
tft.fillRect(x, y, leveys, korkeus, väri);    // Täytetty suorakulmio
tft.drawRect(x, y, leveys, korkeus, väri);    // Reunaviiva

// Ympyrät
tft.fillCircle(x, y, säde, väri);             // Täytetty ympyrä
tft.drawCircle(x, y, säde, väri);             // Ympyränreuna

// Viivat
tft.drawLine(x1, y1, x2, y2, väri);           // Suora viiva

// Näytön tyhjennys
tft.fillScreen(COLOR_BG);                     // Tyhjennä koko näyttö
```

**Esimerkki: Piirrä laatikko RSSI:n ympärille**

Muokkaa funktiota `drawData()` (rivi 577):
```cpp
// Piirrä laatikko RSSI:n taakse
tft.drawRect(rightX - 5, rightY - 5, 100, 45, COLOR_PRIMARY);

// RSSI (FONT_LARGE - iso numero)
tft.setTextSize(FONT_LARGE);
tft.setTextColor(COLOR_TEXT_PRIMARY);
String rssiValue = rssiStr.length() > 0 ? rssiStr.substring(0, rssiStr.indexOf("d")) : "-";
tft.drawString(rssiValue, rightX, rightY);
```

#### 6. Uusien kenttien lisääminen

Voit vastaanottaa mitä tahansa CSV-kenttiä robotilta ja näyttää ne.

**Vaihe 1: Lähetä data robotilta**

Muokkaa `display_sender.h` tai `DisplayClient` -kirjastoa:
```cpp
display.set("TEMP", 25);    // Lämpötila
display.set("HUM", 60);     // Kosteus
display.send();
```

**Vaihe 2: Hae arvo näytöllä**

Käytä funktiota `getFieldValue()`:
```cpp
String tempStr = getFieldValue("TEMP");
String humStr = getFieldValue("HUM");
```

**Vaihe 3: Piirrä näytölle**

Lisää funktioon `drawData()`:
```cpp
tft.setTextSize(FONT_NORMAL);
tft.setTextColor(COLOR_LABEL);
tft.drawString("Temp:", leftX, leftY);
tft.setTextColor(COLOR_TEXT_PRIMARY);
tft.drawString(tempStr + "C", leftX + 60, leftY);
leftY += 25;
```

#### 7. Signaalin laatupalkin muokkaaminen

**Signaalipalkki piirretään funktiossa `drawSignalQualityBar()` (rivit 619-660).**

**Esimerkki 1: Leveämpi palkki**
```cpp
#define SIGNAL_BAR_W    50       // Oli 30 → nyt 50
#define SIGNAL_BAR_X    270      // Siirrä vasemmalle (oli 280)
```

**Esimerkki 2: Muuta värirajoja**

Muokkaa funktiota `getSignalQualityColor()` (rivi 199):
```cpp
uint16_t getSignalQualityColor(int quality) {
  if (quality >= 80) return COLOR_GOOD;   // Oli 70 → nyt 80
  if (quality >= 50) return COLOR_WARN;   // Oli 40 → nyt 50
  return COLOR_BAD;
}
```

**Esimerkki 3: Vaakasuora palkki**

Muokkaa koordinaatit:
```cpp
#define SIGNAL_BAR_X    10       // Vasen reuna
#define SIGNAL_BAR_Y    220      // Alaosa
#define SIGNAL_BAR_W    300      // Lähes koko leveys
#define SIGNAL_BAR_H    15       // Matala
```

Muokkaa piirtokoodi (rivi 643):
```cpp
// Piirrä täyttö vasemmalta oikealle (ei alhaalta ylös)
int fillWidth = (SIGNAL_BAR_W - 4) * quality / 100;
tft.fillRect(SIGNAL_BAR_X + 2, SIGNAL_BAR_Y + 2, fillWidth, SIGNAL_BAR_H - 4, barColor);
```

### Signal Testing Mode - Signaalitestausnäyttö

**Tarkoitus:** Optimoitu näyttö LoRa-signaalin testaukseen ja analysointiin kenttäolosuhteissa.

**Aktivointi:** `SIGNAL_TESTING_MODE true` (rivi 67 Roboter_Display_TFT.ino:ssa)

#### Näytön ulkoasu (ASCII)

```
┌────────────────────────────────────────────────┐
│ ROBOTER 9  ●        UART ON    PKT:142        │ ← Header (30px)
│                       DATA ONLINE              │
├────────────────────────────────────┬───────────┤
│                                    │           │
│  Aika:     12:34                   │           │
│  Viime:    2s                      │    ███    │
│  dB:       -67dBm                  │    ███    │
│  SNR:      9dB                     │    ███    │
│  RSSI:     -67dBm                  │    ███    │ ← Signaalipalkki
│  SEQ:      142                     │    ███    │   (70-100% vihreä)
│  Paketit:  142                     │    ███    │
│  Häviöi:   0.7% (1/142)            │    ███    │
│                                    │    ███    │
│                                    │    ███    │
│                                    │     76%   │
├────────────────────────────────────┴───────────┤
│ LoRa ONLINE          -67dBm | Addr:1 | RX     │ ← Footer (30px)
└────────────────────────────────────────────────┘
```

#### Kenttien selitykset

**Header-alue (yläosa):**

| Kenttä | Kuvaus | Arvoalue |
|--------|--------|----------|
| **ROBOTER 9** | Projektin nimi | Kiinteä |
| **● LED** | LED-indikaattori, synkronoitu LoRa-lähetysten kanssa | ● Punainen = ON, ○ Harmaa = OFF |
| **UART ON/OFF** | UART-yhteyden tila displaylle | ON (vihreä) / OFF (punainen) |
| **DATA ONLINE/WAITING** | Datan saapumisen tila | ONLINE (vihreä) / WAITING (harmaa) |
| **PKT:n** | UART-pakettien määrä (vastaanotetut displayllä) | 0-∞ |

**Data-alue (keskiosa):**

| Kenttä | Kuvaus | Arvoalue | Tulkinta |
|--------|--------|----------|----------|
| **Aika** | Uptime-aikaleima (min:sek) | 0:00-∞ | Näyttää kuinka kauan laite ollut päällä |
| **Viime** | Aika viimeisestä paketista | 0s-∞ | >5s → keltainen varoitus |
| **dB** | RSSI (Received Signal Strength) | -40 to -120 dBm | Ks. RSSI-taulukko alla |
| **SNR** | Signal-to-Noise Ratio | -20 to +20 dB | Ks. SNR-taulukko alla |
| **RSSI** | RSSI-arvo (toisto) | Sama kuin dB | (duplikaatti, harkitse poistoa) |
| **SEQ** | Sekvenssinnumero | 0-∞ | Jatkuva laskuri, käytetään pakettihäviön laskentaan |
| **Paketit** | LoRa-paketit yhteensä (lähettäjältä) | 0-∞ | Odotetut paketit yhteensä |
| **Häviöi** | Pakettihäviöprosentti (Lost/Expected) | 0.0%-100% | <2% vihreä, 2-10% oranssi, >10% punainen |

**Signaalipalkki (oikea reuna):**

| Väri | Signaalin laatu | Prosentti |
|------|-----------------|-----------|
| 🟢 Vihreä | Erinomainen | 70-100% |
| 🟠 Oranssi | Keskinkertainen | 40-69% |
| 🔴 Punainen | Heikko | 0-39% |

Lasketaan: `quality = RSSI% (0-100) + SNR bonus (0-30%)`

**Footer-alue (alaosa):**

| Kenttä | Kuvaus | Arvoalue |
|--------|--------|----------|
| **LoRa ONLINE/OFFLINE** | LoRa-yhteyden tila | ONLINE (oranssi) / OFFLINE (harmaa) / LOST (harmaa) |
| **dBm** | RSSI lyhyt muoto | -40 to -120 dBm |
| **Addr** | LoRa-osoite (1=RX, 2=TX) | 1 tai 2 |
| **RX/TX** | Laitteen rooli | RX (vastaanottaja) / TX (lähettäjä) |

#### RSSI ja SNR tulkinta

**RSSI (Received Signal Strength Indicator) - Vastaanotettu signaalin voimakkuus:**

| RSSI (dBm) | Signaalin laatu | Etäisyysarvio (SF12) | Toimenpide |
|------------|-----------------|----------------------|------------|
| -40 to -60 | ⭐⭐⭐⭐⭐ Erinomainen | 0-10 m (erittäin lähellä) | Normaali toiminta |
| -60 to -75 | ⭐⭐⭐⭐ Erittäin hyvä | 10-50 m | Normaali toiminta |
| -75 to -85 | ⭐⭐⭐ Hyvä | 50-200 m | Normaali toiminta |
| -85 to -95 | ⭐⭐ Kohtalainen | 200-500 m | Toimii, mutta voi häiriintyä |
| -95 to -105 | ⭐ Heikko | 500-1000 m | Pakettihäviöitä, tarvitsee näköyhteyden |
| -105 to -120 | ⚠️ Erittäin heikko | 1000+ m | Yhteys katkeaa pian, paranna olosuhteita |
| < -120 | ❌ Ei yhteyttä | - | Ei yhteyttä, siirrä lähemmäs |

**SNR (Signal-to-Noise Ratio) - Signaalin ja kohinan suhde:**

| SNR (dB) | Signaalin laatu | Tulkinta | Toimenpide |
|----------|-----------------|----------|------------|
| +15 to +20 | ⭐⭐⭐⭐⭐ Erinomainen | Erittäin vähän kohinaa, selkeä signaali | Normaali toiminta |
| +10 to +15 | ⭐⭐⭐⭐ Erittäin hyvä | Vähän kohinaa | Normaali toiminta |
| +5 to +10 | ⭐⭐⭐ Hyvä | Kohtuullinen kohinataso | Normaali toiminta |
| 0 to +5 | ⭐⭐ Kohtalainen | Signaali ja kohina lähellä toisiaan | Toimii, mutta herkempi häiriöille |
| -5 to 0 | ⭐ Heikko | Kohina voimakkaampaa kuin signaali (LoRa demoduloi silti!) | Mahdollisia pakettihäviöitä |
| -10 to -5 | ⚠️ Erittäin heikko | Paljon kohinaa, LoRa:n rajalla | Yhteys epävakaa |
| < -10 | ❌ Kriittinen | Kohina peittää signaalin | Yhteys katkeaa pian |

**LoRa-erikoisuus:** LoRa pystyy demoduloimaan signaaleja jopa SNR -20 dB saakka (signaali kohinan alapuolella), mikä tekee siitä erittäin suorituskykyisen pitkillä etäisyyksillä!

**Signaalin laadun optimointi:**

| Ongelma | RSSI | SNR | Todennäköinen syy | Ratkaisu |
|---------|------|-----|-------------------|----------|
| Heikko signaali | < -95 | Mikä tahansa | Liian pitkä etäisyys | Siirrä lähemmäs tai lisää TX-tehoa |
| Voimakas kohina | Hyvä | < 0 | Sähkömagneettinen häiriö | Siirrä pois häiriölähteistä (WiFi, Bluetooth, teollisuuslaitteet) |
| Epävakaa yhteys | Vaihtelee | Vaihtelee | Esteet, ilman kosteus | Varmista näköyhteys, testaa eri säissä |
| Pakettihäviöitä | Hyvä | Hyvä | Ohjelmavirhe tai laitevika | Tarkista koodi ja LoRa-moduulin kytkennät |

#### SEQ ja pakettihäviö

**Sekvenssinnumero (SEQ):**
- Jokaisessa lähetetyssä paketissa on jatkuva laskuri (0, 1, 2, 3, ...)
- Vastaanottaja vertaa saapuneiden pakettien SEQ-numeroita
- Jos SEQ hyppää (esim. 42 → 44), paketti 43 on kadonnut

**Pakettihäviön laskenta:**

```
Häviöprosentti = (Menetetyt paketit / Odotetut paketit) × 100%

Esim: Häviöi: 0.7% (1/142)
→ 1 paketti kadonnut 142 odotetusta
→ 0.7% häviöprosentti
```

**Pakettihäviön värikoodaus:**

| Häviöprosentti | Väri | Tulkinta | Toimenpide |
|----------------|------|----------|------------|
| 0.0 - 2.0% | 🟢 Vihreä | Normaali, hyväksyttävä taso | Ei toimenpiteitä |
| 2.1 - 10.0% | 🟠 Oranssi | Kohtalainen häiriö | Tarkista RSSI/SNR, poista esteitä |
| > 10.0% | 🔴 Punainen | Vakava ongelma | Siirrä lähemmäs, tarkista kytkennät |

**Huom:** Jopa 1-2% pakettihäviö on normaalia langattomissa verkoissa!

**Debuggaus Serial Monitorilla:**

Kun paketteja katoaa, display-ESP32:n Serial Monitor näyttää:
```
⚠️  Lost packets detected: 3 (SEQ 45 to 47)
```

Tämä auttaa tunnistamaan häiriöt reaaliajassa.

#### 8. Päivitysvälin muokkaaminen

**Näyttö päivittyy määrätyin väliajoin (rivi 76):**

```cpp
#define DISPLAY_UPDATE_INTERVAL 500     // Päivitys 500ms välein
```

**Nopea päivitys (200ms):**
```cpp
#define DISPLAY_UPDATE_INTERVAL 200
```

**Hidas päivitys (1000ms):**
```cpp
#define DISPLAY_UPDATE_INTERVAL 1000
```

**HUOM:** Liian nopea päivitys (alle 100ms) voi aiheuttaa välkkymistä!

#### 9. Vinkkejä muokkaamiseen

**1. Testaa pienin muutoksin**
- Muuta yksi asia kerrallaan
- Käännä ja lataa koodi
- Tarkista näyttö

**2. Kommentoi vanha koodi**
```cpp
// int ledX = 120;  // Vanha sijainti
int ledX = 250;     // Uusi sijainti
```

**3. Käytä Serial-tulosteita debuggaukseen**
```cpp
Serial.print("RSSI value: ");
Serial.println(rssiValue);
```

**4. Piirrä reunat alueiden havaitsemiseksi**
```cpp
tft.drawRect(DATA_LEFT_X, DATA_Y, DATA_LEFT_W, DATA_H, COLOR_WARN);  // Vasen laatikko
tft.drawRect(DATA_RIGHT_X, DATA_Y, DATA_RIGHT_W, DATA_H, COLOR_GOOD); // Oikea laatikko
```

**5. Käytä väliaikaisia testejä**
```cpp
// Testaa tekstin sijaintia
tft.fillCircle(rightX, rightY, 3, COLOR_ERROR);  // Piirrä piste koordinaatteihin
```

#### 10. Yleisiä ongelmia

**Teksti ei näy:**
- Tarkista että tekstiväri ei ole sama kuin tausta
- Varmista että koordinaatit ovat näytön sisällä (0-320, 0-240)
- Tarkista että `tft.setTextSize()` on asetettu

**Elementit menevät päällekkäin:**
- Tarkista koordinaatit ja leveydet
- Varmista että `leftY += 25;` kasvattaa y-koordinaattia riittävästi

**Näyttö vilkkuu:**
- Älä piirrä elementtejä joka silmukassa
- Käytä `DISPLAY_UPDATE_INTERVAL` rajoittamaan päivityksiä
- Piirrä vain muuttuneet osat (älä koko näyttöä)

**Värit näyttävät vääriltä:**
- Käytä RGB565-muunninta: https://rgbcolorpicker.com/565
- Muista 0x-etuliite heksaluvuille (esim. 0xFFFF)

---

## PC-datan Tallennus

### Python-skriptit

**Vaatimukset:**
```bash
pip install pyserial
```

### 1. serial_monitor.py - Reaaliaikainen seuranta

```bash
python serial_monitor.py /dev/ttyUSB0 115200
```

**Ominaisuudet:**
- Värjätty terminaalituloste
- RSSI-palkit
- Elävä data
- Virheet ja varoitukset korostettuna

### 2. data_logger.py - Tietokantatallennus

```bash
python data_logger.py /dev/ttyUSB0 115200 lora_data.db
```

**Ominaisuudet:**
- Automaattinen SQLite-tietokannan luonti
- Kaikki CSV-data aikaleimoilla
- Tapahtumaloki
- Indeksoitu nopeaan hakuun

### CSV-dataformaatti

```
DATA_CSV,TIMESTAMP,ROLE,RSSI,SNR,SEQ,MSG_COUNT,CONN_STATE,PACKET_LOSS,LED,TOUCH
```

**Esimerkki:**
```
DATA_CSV,45632,RX,-67,9,142,142,OK,0.00,1,0
```

### Datan analysointi

**SQLite-komentorivi:**
```bash
sqlite3 lora_data.db
SELECT AVG(rssi) FROM lora_messages;
SELECT timestamp, packet_loss FROM lora_messages ORDER BY timestamp;
```

**Python/Pandas:**
```python
import sqlite3
import pandas as pd

conn = sqlite3.connect('lora_data.db')
df = pd.read_sql_query("SELECT * FROM lora_messages", conn)
df.plot(x='timestamp', y='rssi')
```

---

## Vianmääritys

### Ei LoRa-kommunikaatiota

**Tarkista:**
1. LoRa-moduuli saa virran (3.3V, GND)
2. Kytkennät: TX→25, RX→26
3. Molemmat laitteet: Sama `LORA_NETWORK_ID` (oletus: 6)
4. Serial näyttää: `✓ LoRa initialized`

**Korjaa:**
- Käynnistä LoRa-moduulit uudelleen
- Tarkista juotokset
- Kokeile eri USB-virtalähdettä

### Väärä rooli tunnistettu

**Ongelma:** Lähettäjä toimii vastaanottajana tai päinvastoin

**Tarkista:**
- Vastaanottaja: GPIO 16 ↔ GPIO 17 **yhdistetty**
- Lähettäjä: GPIO 16 **irti** (ei yhteyttä)

**Korjaa:**
- Lisää/poista hyppylanka
- Käynnistä ESP32 uudelleen

### TFT-näyttö ei toimi

**Tarkista:**
1. Näyttökoodi ladattu (`Roboter_Display_TFT.ino`)
2. Kytkennät: Robot GPIO 23 → Display GPIO 18
3. Yhteinen GND yhdistetty
4. config.h: `ENABLE_DISPLAY_OUTPUT true`

**Korjaa:**
- Tarkista Serial Monitorit (sekä robotti että näyttö)
- Robotti näyttää: `→ Display: MODE:...`
- Näyttö näyttää: `📥 RX [1]: ...`
- Vaihda johdot tarvittaessa (TX menee RX:ään!)

### RSSI/SNR-arvot vääriä

**Ongelma:** RSSI näyttää -30 dBm (epärealistinen)

**Tarkista:**
- LoRa-moduulit ovat liian lähellä (<50cm)
- Siirrä kauemmas (>1m)
- RSSI -50 to -120 dBm on normaali

### Kill-switch ei toimi

**Tarkista:**
1. GPIO 13 ↔ GPIO 14 yhdistetty
2. Pidä 3 sekuntia (katso Serial Monitor)
3. Serial näyttää: `🔴 Kill-switch PRESSED...`

**Huom:** Kill-switch toimii ilman LoRa-moduulia (testattavissa erikseen)

### CSV-data ei näy

**Tarkista:**
1. `ENABLE_CSV_OUTPUT true` config.h:ssa
2. ESP32 käynnissä (tarkista boot-viestit)
3. Baudrate 115200
4. LoRa-moduuli yhdistetty (vastaanottajarooli)

### Kääntövirheet

**Tarkista:**
- Kaikki .h-tiedostot samassa kansiossa kuin .ino
- `LiquidCrystal_I2C` -kirjasto asennettu (jos LCD käytössä)
- Oikea levy valittu: ESP32 Dev Module

---

## Kill-Switch käyttö

**Fyysinen kill-switch:**
1. Yhdistä GPIO 13 ↔ GPIO 14
2. Pidä 3 sekuntia
3. Laite käynnistyy uudelleen

**Etä-kill-switch (LoRa):**
- Lähetä komento: `CMD:RESTART`
- Laite käynnistyy uudelleen automaattisesti

**Käyttötapaukset:**
- Hätäpysäytys testauksen aikana
- Nopea uudelleenkäynnistys ilman virtakytkentöjä
- Turvaominaisuus robotin ohjauksessa

---

## Lisätiedot

### Yhteysvalvonta (Connection Watchdog)

Automaattinen yhteyden tilan seuranta:
- `CONN_CONNECTED` - Normaali käyttö
- `CONN_WEAK` - Viivästyneet viestit (3-8s)
- `CONN_LOST` - Ei viestejä >8s

**Automaattinen palautuminen:**
1. Tila vaihtuu `CONN_LOST`:iin
2. 3 palautumisyritystä
3. LoRa-moduuli alustetaan uudelleen
4. Paluu normaaliin toimintaan

### Pakettihäviön seuranta

Järjestelmä käyttää sekvenssinnumeroita pakettihäviön havaitsemiseen:
```
Häviöprosentti = (Puuttuvat paketit / Odotetut paketit) × 100%
```

Serial Monitor näyttää:
```
Packet loss: 2.5% (3/120 lost)
```

### Suorituskyky

- **Silmukan taajuus:** ~100 Hz (10ms viive)
- **Lähetysväli:** 2 sekuntia (muokattavissa)
- **ACK-vastausaika:** <500ms
- **LCD-päivitysnopeus:** 10 Hz (100ms)
- **Datan tallennus:** 0.5 Hz (2 sekuntia)

---

## Kehitysohjeet

### Uuden ominaisuuden lisääminen

1. Luo uusi `.h` -tiedosto projektikansioon
2. Lisää ominaisuusvalitsin `config.h`:hon
3. Kääri koodi `#if ENABLE_YOUR_FEATURE` -lohkoon
4. Sisällytä `Roboter_Gruppe_9.ino`:hon
5. Päivitä dokumentaatio
6. Testaa ominaisuus PÄÄLLÄ ja POIS

### Koodityyli

- Käytä selkeitä, kuvaavia muuttujan nimiä
- Lisää kommentit monimutkaiseen logiikkaan
- Pidä funktiot pieninä ja keskittyvinä
- Käytä `const` vakioille
- Vältä globaaleja muuttujia (käytä struct:eja)

---

## Tekniset tiedot

**Kehitys- ja testiympäristö:**
- ESP32 DevKit v1
- RYLR896 LoRa (868 MHz)
- Arduino IDE / PlatformIO
- LovyanGFX (TFT)
- Python 3.8+

**Kantama:** Jopa 5+ km (näköyhteys, SF12)

**Virrankulutus:** ~100mA tyypillinen

---

## Yhteenveto

Tämä on tuotantovalmis LoRa-kommunikaatiojärjestelmä, joka sisältää:
- ✅ Automaattinen roolintunnistus
- ✅ Kaksisuuntainen kommunikaatio
- ✅ Reaaliaikainen TFT-näyttö
- ✅ Yhteyden valvonta ja palautuminen
- ✅ Pakettihäviön seuranta
- ✅ PC-datan tallennus
- ✅ Kill-switch (fyysinen + etä)
- ✅ Kattava dokumentaatio

**Aloita testaus:** Katso [TESTING.md](TESTING.md)

---

*Viimeksi päivitetty: 14.11.2025*
