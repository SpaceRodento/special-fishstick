# Roboter Gruppe 9 - Koodikatselmusraportti

**Päivämäärä:** 2025-11-14
**Katselmoija:** Claude
**Projektin koko:** 6427 riviä koodia (20 tiedostoa)

---

## 📊 Yhteenveto

**Kokonaisarvio:** ⚠️ **HYVÄ, mutta parannettavaa**

- ✅ Modulaarinen rakenne
- ✅ Selkeät feature flagit
- ⚠️ Merkittäviä päällekkäisyyksiä
- ⚠️ Potentiaalisia ristiriitoja jos kaikki ominaisuudet käytössä
- ⚠️ Ei muistinhallintaongelmia, mutta voisi optimoida

---

## 🔴 KRIITTISET ONGELMAT

### 1. **Päällekkäinen pakettihäviön seuranta** ⚠️ KORKEA PRIORITEETTI

**Ongelma:**
Pakettihäviötä lasketaan KOLMESSA eri paikassa:

1. **health_monitor.h** (aina päällä, pakollinen):
   ```cpp
   health.expectedSeq
   health.packetsReceived
   health.packetsLost
   ```

2. **packet_stats.h** (ENABLE_PACKET_STATS):
   ```cpp
   stats.packetsReceived
   stats.packetsLost
   stats.duplicates
   stats.outOfOrder
   ```

3. **Roboter_Display_TFT.ino** (näyttölaite):
   ```cpp
   lastReceivedSeq
   totalPacketsExpected
   totalPacketsReceived
   totalPacketsLost
   ```

**Miksi ongelma:**
- Jos sekä health_monitor JA packet_stats ovat käytössä, sama data lasketaan kahdesti
- Molemmat päivittävät omia muuttujia → eri tulokset
- Käyttäjä ei tiedä kumpaa lukemaa uskoa
- Turhaa muistinkäyttöä

**Ratkaisu:**
```cpp
// VAIHTOEHTO A: Tee packet_stats.h wrapperiksi
// packet_stats käyttää health_monitor.h:n perusdataa ja lisää vain
// lisätilastot (duplicates, outOfOrder, jitter)

// VAIHTOEHTO B: Yhdistä health_monitor.h + packet_stats.h
// → telemetry.h (yksi yhtenäinen tilastointimoduuli)
```

**Koodiesimerkit ristiriidasta:**
```cpp
// health_monitor.h:92
health.expectedSeq = receivedSeq + 1;
health.packetsLost++;

// packet_stats.h:145 (jos enabled)
stats.packetsLost++;        // SAMA TAPAHTUMA, eri muuttuja!
stats.currentLossStreak++;
```

**Vaikutus:**
- 🔴 Eri lukemia serial outputissa
- 🟡 Turhaa RAM-käyttöä (~50 bytes)
- 🟡 CPU-kuorma kahdesta laskennasta

---

### 2. **RSSI/SNR-statistiikan päällekkäisyys** ⚠️ KESKITASO

**Ongelma:**
RSSI/SNR-statistiikka lasketaan KAHDESSA paikassa:

1. **health_monitor.h** (aina päällä):
   ```cpp
   int rssiMin, rssiMax;
   long rssiSum;
   int rssiSamples;
   // Keskiarvo lasketaan: rssiSum / rssiSamples
   ```

2. **packet_stats.h** (ENABLE_PACKET_STATS):
   ```cpp
   int rssiMin, rssiMax;
   long rssiSum;
   int rssiCount;
   float rssiAvg;
   // TÄSMÄLLEEN SAMA LOGIIKKA!
   ```

**Miksi ongelma:**
- Identtinen koodi kahdessa paikassa
- DRY-periaatteen rikkominen (Don't Repeat Yourself)
- Ylläpito hankalampaa (bugit kahdessa paikassa)

**Ratkaisu:**
```cpp
// Säilytä health_monitor.h:ssä (ydin)
// packet_stats.h käyttää health_monitor.h:n dataa:
#if ENABLE_PACKET_STATS
  // Käytä suoraan:
  rssiMin = health.rssiMin;
  rssiMax = health.rssiMax;
  rssiAvg = (float)health.rssiSum / health.rssiSamples;
#endif
```

**Vaikutus:**
- 🟡 Turhaa muistinkäyttöä (~24 bytes)
- 🟡 Mahdollisia ristiriitoja tuloksissa

---

### 3. **Jännitemittauksen päällekkäisyys** ⚠️ KESKITASO

**Ongelma:**
Akkujännitettä mitataan KAHDELLA eri tavalla:

1. **battery_monitor.h** (ENABLE_BATTERY_MONITOR):
   ```cpp
   Pin: GPIO 35 (ADC1_CH7)
   Menetelmä: Voltage divider (2:1)
   Tarkkuus: 12-bit ADC (~3mV)
   ```

2. **current_monitor.h** (ENABLE_CURRENT_MONITOR):
   ```cpp
   Laite: INA219 (I2C 0x40)
   Menetelmä: High-side current sensor
   Tarkkuus: 4mV (tarkempi!)
   Bonus: Mittaa myös virran ja tehon
   ```

**Miksi ongelma:**
- Jos molemmat enabled → jännite mitataan kahdesti
- INA219 on tarkempi, mutta battery_monitor yksinkertaisempi
- Käyttäjä voi saada eri lukemat samasta akusta

**Ratkaisu:**
```cpp
// VAIHTOEHTO A: Konfiguraatiovaroitus
#if ENABLE_BATTERY_MONITOR && ENABLE_CURRENT_MONITOR
  #warning "Both battery monitoring methods enabled!"
  #warning "INA219 (current_monitor) is more accurate."
  #warning "Disable ENABLE_BATTERY_MONITOR to save resources."
#endif

// VAIHTOEHTO B: Yhdistä moduuleihin
// Luo sensors.h joka tukee molempia tapoja:
// - Pelkkä ADC (yksinkertainen)
// - INA219 (tarkka + virta/teho)
```

**Vaikutus:**
- 🟡 Mahdollisesti eri lukemat (~0.1-0.2V ero)
- 🟡 Turhaa muistia (~150 bytes)
- 🟢 Ei teknistä ongelmaa (toimii rinnakkain)

---

## 🟡 KESKITASON ONGELMAT

### 4. **I2C-bus kuormitus** ⚠️ PIENI RISKI

**Tilanne:**
Samalla I2C-väylällä (SDA=21, SCL=22) on:

| Laite | I2C-osoite | Feature flag | Kirjasto |
|-------|------------|--------------|----------|
| LCD 16x2 | 0x27 | Aina päällä | LiquidCrystal_I2C |
| INA219 | 0x40 | ENABLE_CURRENT_MONITOR | Adafruit_INA219 |
| TCS34725 | 0x29 | ENABLE_LIGHT_DETECTION | Adafruit_TCS34725 |

**Analyysi:**
- ✅ Osoitteet eivät törmää (0x27, 0x29, 0x40 kaikki erilaiset)
- ✅ I2C tukee useita laitteita samalla väylällä
- ⚠️ Jos kaikki päällä: 3 laitetta samalla väylällä
- ⚠️ Wire.begin() kutsutaan useasti (ei haittaa, mutta turha)

**Suositus:**
```cpp
// Lisää config.h:hon varoitus:
#if ENABLE_LIGHT_DETECTION && ENABLE_CURRENT_MONITOR
  #info "Multiple I2C devices enabled (TCS34725 + INA219)"
  #info "Verify I2C bus connections (SDA=21, SCL=22)"
#endif
```

**Vaikutus:**
- 🟢 Ei teknistä ongelmaa
- 🟡 Lisää I2C-väylän kuormitusta
- 🟡 Yhden laitteen vika voi vaikuttaa muihin

---

### 5. **Wire.begin() kutsutaan useasti** ⚠️ PIENI

**Ongelma:**
I2C-väylä alustetaan useassa moduulissa:

```cpp
// functions.h (LCD):
void initLCD() {
  lcd.init();  // Kutsuu sisäisesti Wire.begin()
}

// current_monitor.h:117
Wire.begin();

// light_detector.h (implisiittisesti):
tcs.begin();  // Kutsuu Wire.begin()
```

**Miksi ongelma:**
- Wire.begin() on turvallista kutsua useasti (ei haittaa)
- Mutta ei optimaalista
- Voi aiheuttaa viiveitä käynnistyksessä

**Ratkaisu:**
```cpp
// Luo i2c_manager.h:
bool i2cInitialized = false;

void ensureI2CInitialized() {
  if (!i2cInitialized) {
    Wire.begin();
    i2cInitialized = true;
    Serial.println("✓ I2C initialized (SDA=21, SCL=22)");
  }
}

// Jokainen moduuli kutsuu:
ensureI2CInitialized();
```

**Vaikutus:**
- 🟢 Ei teknistä ongelmaa (toimii)
- 🟡 Ei optimaalista
- 🟡 Hidastaa käynnistystä (~10-50ms per kutsu)

---

### 6. **Feature flagien sisäkkäisyys puuttuu** ⚠️ KESKITASO

**Ongelma:**
Ei tarkisteta ristiriitaisia konfiguraatioita:

```cpp
// Esimerkki: Molemmat salaukset päällä
#define ENABLE_ENCRYPTION true
#define ENABLE_ADVANCED_COMMANDS true  // Sisältää omat komennot

// Tai: Adaptive SF + manuaaliset SF-komennot
#define ENABLE_ADAPTIVE_SF true
#define ENABLE_RUNTIME_CONFIG true  // CONFIG:SF:10 ristiriidassa!
```

**Ratkaisu:**
```cpp
// config.h:n loppuun, lisää CONFIGURATION VALIDATION:

// =============== CONFIGURATION VALIDATION ================================
#if ENABLE_ADAPTIVE_SF && ENABLE_RUNTIME_CONFIG
  #warning "Adaptive SF + Runtime Config: SF commands may conflict!"
  #warning "Adaptive SF will override manual SF settings."
#endif

#if ENABLE_BATTERY_MONITOR && ENABLE_CURRENT_MONITOR
  #warning "Both battery monitoring methods enabled!"
  #warning "INA219 (current_monitor) provides more accurate voltage."
  #warning "Consider disabling ENABLE_BATTERY_MONITOR."
#endif

#if ENABLE_ENCRYPTION && ENABLE_ADVANCED_COMMANDS
  #info "Encryption enabled with advanced commands."
  #info "Ensure remote commands are also encrypted on sender."
#endif

#if (ENABLE_LIGHT_DETECTION || ENABLE_CURRENT_MONITOR) && !defined(Wire_h)
  #error "I2C features enabled but Wire.h not included!"
  #error "Add: #include <Wire.h> to main .ino file"
#endif

// Muistivaroitukset
#define ESTIMATED_RAM_USAGE \
  (ENABLE_PACKET_STATS * 100) + \
  (ENABLE_EXTENDED_TELEMETRY * 50) + \
  (ENABLE_PERFORMANCE_MONITOR * 30) + \
  (ENABLE_BATTERY_MONITOR * 20) + \
  (ENABLE_CURRENT_MONITOR * 30)

#if ESTIMATED_RAM_USAGE > 500
  #warning "High RAM usage estimated!"
  #warning "Consider disabling some features if stability issues occur."
#endif
```

**Vaikutus:**
- 🟡 Käyttäjä voi vahingossa aktivoida ristiriitaiset ominaisuudet
- 🟡 Vaikea debugata kun toiminta on odottamatonta

---

## 🟢 PIENET HUOMIOT

### 7. **Tiedostojen määrä (19 .h-tiedostoa)**

**Tilanne:**
- 1× .ino (pääohjelma)
- 19× .h (moduulit)

**Suositus: Yhdistä loogisesti**

#### VAIHTOEHTO 1: Minimaalinen (suositus)
```
Yhdistä:
- audio_detector.h + light_detector.h → fire_alarm_detector.h

Tulos: 19 → 18 tiedostoa (-5%)
```

#### VAIHTOEHTO 2: Keskitaso
```
Yhdistä:
- audio_detector.h + light_detector.h → fire_alarm_detector.h
- battery_monitor.h + current_monitor.h → sensors.h
- packet_stats.h + extended_telemetry.h → detailed_telemetry.h

Tulos: 19 → 15 tiedostoa (-21%)
```

#### VAIHTOEHTO 3: Aggressiivinen (EI suositella)
```
Yhdistä kaikki feature-moduulit → features.h

Tulos: 19 → 8 tiedostoa (-58%)
Ongelma: Vaikea ylläpitää, feature flagit menettävät merkityksen
```

**Suositus:** Vaihtoehto 1 tai 2

**Vaikutus:**
- 🟢 Helpompi navigoida
- 🟢 Vähemmän tiedostoja käännettävänä
- 🟡 Ei teknistä hyötyä

---

### 8. **String-käyttö**

**Tilanne:**
Arduino String-luokkaa käytetään laajalti:

```cpp
// Esimerkkejä:
String message = "...";
String payload = "SEQ:" + String(seq) + ",LED:...";
```

**Ongelma:**
- String aiheuttaa heap-fragmentaatiota
- Dynaaminen muistinvaraus (hidas, riskialtis)
- Suositellaan char-taulukoita embedded-järjestelmissä

**Ratkaisu:**
```cpp
// Vaihda kriittisissä kohdissa:
char payload[128];
snprintf(payload, sizeof(payload), "SEQ:%d,LED:%d", seq, led);
```

**Vaikutus:**
- 🟡 Ei akuuttia ongelmaa (ESP32:lla on muistia)
- 🟡 Pitkäaikaisessa käytössä (päivät/viikot) voi aiheuttaa fragmentaatiota
- 🟢 Nykyinen toteutus toimii

**Suositus:**
Älä muuta ellei ongelmia ilmene. ESP32:lla 320KB RAM riittää.

---

### 9. **Global-muuttujat**

**Tilanne:**
Useita globaaleja muuttujia .h-tiedostoissa:

```cpp
// Esim. health_monitor.h:
HealthMonitor health;  // Global

// packet_stats.h:
PacketStatistics stats;  // Global

// Jne.
```

**Ongelma:**
- Rikastuu namespace (mahdolliset nimiristiriidat)
- Vaikea testata yksikkötesteillä

**Ratkaisu:**
```cpp
// Käytä namespaceja tai static-muuttujia:
namespace HealthMonitoring {
  HealthMonitor health;

  void init() { ... }
  void update() { ... }
}

// Tai state-struct:
struct RobotState {
  HealthMonitor health;
  PacketStatistics stats;
  BatteryStatus battery;
  // ...
};

extern RobotState robotState;
```

**Vaikutus:**
- 🟡 Ei akuuttia ongelmaa (projekti ei ole valtava)
- 🟡 Parempi arkkitehtuuri suuremmissa projekteissa

**Suositus:**
Nykyinen toteutus on OK tälle projektin koolle.

---

### 10. **Kommenttien kieli (englanti vs suomi)**

**Tilanne:**
Sekaisin englantia ja suomea:

```cpp
// Englanniksi:
// Battery voltage monitoring

// Suomeksi:
// Aikaleiman tallennus
```

**Suositus:**
Valitse yksi kieli ja pidä siitä kiinni. Tässä projektissa suomi on luonteva valinta.

**Vaikutus:**
- 🟢 Esteettinen ongelma
- 🟢 Ei teknistä ongelmaa

---

## 📋 SUOSITELTAVAT TOIMENPITEET

### Prioriteetti 1 (TÄRKEÄÄ)

1. ✅ **Korjaa pakettihäviön päällekkäislaskenta**
   - Tee packet_stats.h wrapperiksi health_monitor.h:lle
   - Tai yhdistä moduulit

2. ✅ **Lisää konfiguraatiovalidointi config.h:hon**
   - Varoitukset ristiriitaisista asetuksista
   - Muistin käytön estimaatti

3. ✅ **Dokumentoi I2C-laitteet selkeästi**
   - Listaa kaikki I2C-osoitteet
   - Varoita jos useita laitteita

### Prioriteetti 2 (SUOSITELTAVAA)

4. ⚠️ **Yhdistä fire alarm -detektorit**
   - audio_detector.h + light_detector.h → fire_alarm_detector.h
   - Helpompi ylläpitää

5. ⚠️ **Harkitse battery/current yhdistämistä**
   - sensors.h joka tukee molempia tapoja
   - Vaihtoehtoinen: Lisää varoitus jos molemmat päällä

6. ⚠️ **Luo i2c_manager.h**
   - Yhtenäinen I2C-alustus
   - Laitelistaus ja diagnostiikka

### Prioriteetti 3 (VALINNAISTA)

7. 🟢 **Refaktoroi RSSI/SNR-statistiikka**
   - Yhteinen funktio molemmille moduuleille

8. 🟢 **Namespace-käyttö**
   - Estä nimiristiriidat suuremmissa projekteissa

9. 🟢 **Yhtenäistä kommenttikieli**
   - Valitse suomi tai englanti

---

## 🎯 EHDOTETTU TIEDOSTORAKENNE

### Nykyinen (19 .h + 1 .ino):
```
config.h
structs.h
functions.h
lora_handler.h
health_monitor.h
display_sender.h
DisplayClient.h
battery_monitor.h
current_monitor.h
audio_detector.h
light_detector.h
encryption.h
adaptive_sf.h
advanced_commands.h
extended_telemetry.h
packet_stats.h
performance_monitor.h
runtime_config.h
watchdog_timer.h
Roboter_Gruppe_9.ino
```

### Ehdotettu (15 .h + 1 .ino):
```
config.h                    (+ validointi)
structs.h
functions.h
lora_handler.h
telemetry.h                 (yhdistää: health_monitor + packet_stats + extended_telemetry)
display_sender.h
DisplayClient.h
sensors.h                   (yhdistää: battery_monitor + current_monitor)
fire_alarm_detector.h       (yhdistää: audio_detector + light_detector)
encryption.h
adaptive_sf.h
advanced_commands.h
performance_monitor.h
runtime_config.h
watchdog_timer.h
Roboter_Gruppe_9.ino
```

**Hyödyt:**
- ✅ 21% vähemmän tiedostoja
- ✅ Loogiset kokonaisuudet
- ✅ Vähemmän päällekkäisyyksiä
- ✅ Helpompi navigoida

---

## 📊 MUISTINKÄYTTÖ-ANALYYSI

### Per-moduuli muistinkäyttö (estimaatti):

| Moduuli | RAM (bytes) | Flash (bytes) |
|---------|-------------|---------------|
| health_monitor | 60 | 2500 |
| packet_stats | 100 | 3000 |
| extended_telemetry | 50 | 2000 |
| battery_monitor | 20 | 1500 |
| current_monitor | 30 | 3000 |
| audio_detector | 80 | 3500 |
| light_detector | 90 | 4000 |
| encryption | 10 | 1000 |
| adaptive_sf | 40 | 3000 |
| advanced_commands | 30 | 2500 |
| performance_monitor | 30 | 2000 |
| watchdog_timer | 10 | 1000 |
| **YHTEENSÄ** | **550** | **29000** |

**ESP32 resurssit:**
- RAM: 320 KB (550 bytes = 0.17% käytössä)
- Flash: 4 MB (29 KB = 0.7% käytössä)

**Johtopäätös:**
✅ Muisti ei ole ongelma. ESP32:lla on runsaasti varaa.

---

## ✅ VAHVUUDET

1. **Modulaarinen rakenne** - Helppo lisätä/poistaa ominaisuuksia
2. **Feature flagit** - Käyttäjä voi valita mitä tarvitsee
3. **Hyvä dokumentaatio** - Jokainen moduuli hyvin kommentoitu
4. **Toimiva koodi** - Perusominaisuudet testattu ja toimivat
5. **Laajennettavuus** - Helppo lisätä uusia sensoreita/ominaisuuksia

---

## ⚠️ HEIKKOUDET

1. **Päällekkäisyydet** - Sama toiminto useassa paikassa
2. **Ei validointia** - Ristiriitaiset asetukset mahdollisia
3. **Liian monta tiedostoa** - Voisi yhdistää loogisesti
4. **Globaalit muuttujat** - Namespace-ongelmat suuremmissa projekteissa
5. **String-käyttö** - Voi aiheuttaa fragmentaatiota pitkässä käytössä

---

## 🎓 JOHTOPÄÄTÖS

**Arvosana: 8/10** ⭐⭐⭐⭐⭐⭐⭐⭐☆☆

**Projekti on hyvin toteutettu**, mutta kaipaa hiomista:
- ✅ Perusominaisuudet toimivat hyvin
- ✅ Modulaarinen ja laajennettava
- ⚠️ Päällekkäisyydet aiheuttavat sekaannusta
- ⚠️ Puutteellinen validointi

**Suositus:**
Toteuta Prioriteetti 1 -korjaukset ennen tuotantokäyttöä.
Prioriteetti 2-3 ovat nice-to-have, mutta eivät kriittisiä.

---

**Seuraavat askeleet:**
1. Päätä haluatko toteuttaa korjaukset
2. Valitse yhdistettävät moduulit (Vaihtoehto 1 tai 2)
3. Lisää konfiguraatiovalidointi
4. Testaa kaikki ominaisuudet uudelleen

---

*Raportin luonti: Claude Code*
*Analysoitu: 6427 riviä koodia, 20 tiedostoa*
