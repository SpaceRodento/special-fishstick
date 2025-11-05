# Feature Testing Guide

**Roboter Gruppe 9 - Uusien ominaisuuksien testausopas**

> 🎯 Tämä opas kertoo miten testata jokainen uusi ominaisuus **erikseen**
>
> ⚠️ Testaa yksi kerrallaan! Älä kytke kaikkia päälle yhtä aikaa.
>
> 📝 Jokainen ominaisuus on helposti päälle/pois kytkettävissä config.h:ssa

---

## 🔧 Testausvalmistelujen yleisohje

**Ennen jokaista testiä:**

1. Avaa `config.h`
2. Etsi "FEATURE FLAGS" -osio (rivi ~58)
3. Aseta haluamasi ominaisuus `true` (muut `false`)
4. Tallenna
5. Lataa koodi ESP32:lle
6. Avaa Serial Monitor (115200 baud)
7. Tarkkaile outputia

**Oletustila:**
- Kaikki uudet ominaisuudet ovat `false` oletuksena
- Vanhat ominaisuudet (bi-directional, kill-switch) ovat `true`
- Testaa ensin että perusominaisuudet toimivat

---

## FEATURE 1: Battery Monitoring 🔋

**Tarkoitus:** Seuraa akun jännitettä ja varoita kun akku on lopussa

### Laitteistokytkentä

```
Akku (+) ──┬─── R1 (10kΩ) ──┬─── GPIO 35 (ESP32)
           │                 │
           │                 └─── R2 (10kΩ) ─── GND
           │
         (Akku)
           │
Akku (-) ───────────────────────── GND (ESP32)
```

**Miksi jännitejakopiiri?**
- ESP32 ADC maksimi: 3.3V
- LiPo-akku maksimi: 4.2V → Tarvitaan 2:1 jako
- Jos R1 = R2 → 4.2V → 2.1V (turvallinen!)

### config.h -asetukset

```cpp
#define ENABLE_BATTERY_MONITOR true   // ← Muuta tämä
#define BATTERY_PIN 35
#define BATTERY_VOLTAGE_DIVIDER 2.0
#define BATTERY_CHECK_INTERVAL 60000  // 60s
#define BATTERY_LOW_THRESHOLD 3.3     // Varoitus
#define BATTERY_CRITICAL_THRESHOLD 3.0 // Kriittinen
```

### Testausvaiheet

#### Testi 1: Perustoiminta
1. ✅ Kytke laitteisto ylläolevan kaavion mukaan
2. ✅ Lataa koodi
3. ✅ Odota Serial outputissa:
   ```
   ✓ Battery monitor initialized
     Pin: GPIO 35
     Voltage divider: 1:2.0
     Low threshold: 3.3 V
     Critical threshold: 3.0 V
   ```

#### Testi 2: Jännitemittaus
1. ✅ Odota 60 sekuntia (tai muuta `BATTERY_CHECK_INTERVAL`)
2. ✅ Serial näyttää:
   ```
   🔋 Battery 1: 3.85 V ✓ OK
     Raw ADC: 2380 / 4095, Range: 3.82 - 3.87 V
   ```
3. ✅ Tarkista multimittarilla todellinen jännite
4. ✅ Vertaa: Mitattu ≈ Reported × 2.0

#### Testi 3: Kalibrointi (jos jännite väärä)
1. ✅ Mittaa akun jännite: esim. 4.15V
2. ✅ Mittaa GPIO 35:n jännite: esim. 2.05V
3. ✅ Laske jako: 4.15 / 2.05 = 2.024
4. ✅ Päivitä config.h:
   ```cpp
   #define BATTERY_VOLTAGE_DIVIDER 2.024
   ```

#### Testi 4: Matalan akun varoitus
1. ✅ Simuloi matala akku:
   - Vaihda `BATTERY_LOW_THRESHOLD` → 4.0 (keinotekoinen testi)
   - TAI kytke 3.2V jännitelähde
2. ✅ Serial näyttää:
   ```
   🔋 Battery X: 3.25 V ⚠️ LOW (below 3.3 V)
   ```

#### Testi 5: Kriittinen akku
1. ✅ Simuloi kriittinen tila:
   - Vaihda `BATTERY_CRITICAL_THRESHOLD` → 4.0
   - TAI kytke 2.9V jännitelähde
2. ✅ Serial näyttää:
   ```
   🔋 Battery X: 2.95 V ⚠️ CRITICAL! (below 3.0 V)
   ```

### CSV-output testi
- ✅ Akun jännite näkyy CSV-datassa
- ✅ Python-skriptit lukevat arvon oikein

### Mahdolliset ongelmat

| Ongelma | Syy | Ratkaisu |
|---------|-----|----------|
| Näyttää 0.00V | Ei kytkentää | Tarkista kytkennät |
| Väärä jännite | Väärä jako | Kalibroi divider |
| Ei mittaa | Väärä pin | Käytä GPIO 35 (ADC1!) |
| WiFi konflikti | ADC2 käytössä | GPIO 35 on ADC1 → OK |

---

## FEATURE 2: Runtime Configuration ⚙️

**Tarkoitus:** Muuta asetuksia lennossa ilman uudelleenlataamista

### config.h -asetukset

```cpp
#define ENABLE_RUNTIME_CONFIG true  // ← Muuta tämä
#define CONFIG_COMMAND_PREFIX "CONFIG:"
```

### Testausvaiheet

#### Testi 1: Perustoiminta
1. ✅ Lataa koodi
2. ✅ Avaa Serial Monitor (115200 baud)
3. ✅ Odota outputissa:
   ```
   ✓ Runtime configuration enabled
     Commands:
       CONFIG:SHOW           - Show current settings
       CONFIG:INTERVAL:ms    - Set send interval
       ...
   ```

#### Testi 2: Näytä asetukset
1. ✅ Kirjoita Serial Monitoriin: `CONFIG:SHOW`
2. ✅ Paina Enter
3. ✅ Odota output:
   ```
   ╔═══════════ CURRENT CONFIGURATION ═══════════╗
   ║ Send Interval:     2000 ms
   ║ Spreading Factor:  SF12
   ║ TX Power:          15 dBm
   ║ ACK Interval:      5
   ║ Listen Timeout:    500 ms
   ║ Data Output:       2000 ms
   ║ CSV Output:        ON
   ║ Bi-directional:    ON
   ╚═════════════════════════════════════════════╝
   ```

#### Testi 3: Muuta lähetysintervallia
1. ✅ Kirjoita: `CONFIG:INTERVAL:1000`
2. ✅ Odota:
   ```
   📝 Config command: INTERVAL:1000
   ✓ Send interval set to 1000 ms
   ```
3. ✅ Tarkista: Viestit lähetetään nyt sekunnin välein

#### Testi 4: Muuta Spreading Factoria
1. ✅ Kirjoita: `CONFIG:SF:10`
2. ✅ Odota:
   ```
   📝 Config command: SF:10
   ✓ Spreading factor set to SF10
   → Applying: AT+PARAMETER=10,7,1,4
   ✓ LoRa parameters updated
   ```
3. ✅ Tarkista: LoRa käyttää nyt SF10 (nopeampi, lyhyempi kantama)

#### Testi 5: Muuta TX tehoa
1. ✅ Kirjoita: `CONFIG:POWER:10`
2. ✅ Odota:
   ```
   ✓ TX power set to 10 dBm
   → Applying: AT+CRFOP=10
   ✓ TX power updated
   ```
3. ✅ Tarkista: RSSI etälaitteella heikompi (pienempi teho)

#### Testi 6: Virheenkäsittely
1. ✅ Kokeile väärä arvo: `CONFIG:SF:20`
2. ✅ Odota:
   ```
   ❌ Invalid SF (7-12)
   ```
3. ✅ Kokeile tuntematon komento: `CONFIG:FOO:BAR`
4. ✅ Odota:
   ```
   ❌ Unknown config key: FOO
   ```

#### Testi 7: Palauta oletukset
1. ✅ Kirjoita: `CONFIG:RESET`
2. ✅ Odota:
   ```
   ✓ Configuration reset to defaults
   (Näyttää asetukset)
   ```

### Testattavat komennot

| Komento | Kelvollinen arvo | Testi |
|---------|------------------|-------|
| `CONFIG:SHOW` | - | Näyttää asetukset ✅ |
| `CONFIG:INTERVAL:X` | 100-60000 ms | 1000 ✅, 500 ✅, 99 ❌ |
| `CONFIG:SF:X` | 7-12 | 10 ✅, 6 ❌, 15 ❌ |
| `CONFIG:POWER:X` | 0-20 dBm | 10 ✅, -5 ❌, 25 ❌ |
| `CONFIG:ACK:X` | 1-100 | 10 ✅, 0 ❌ |
| `CONFIG:CSV:X` | ON/OFF | ON ✅, OFF ✅ |
| `CONFIG:RESET` | - | Palauttaa ✅ |

### Mahdolliset ongelmat

| Ongelma | Syy | Ratkaisu |
|---------|-----|----------|
| Ei reagoi komentoihin | Feature disabled | Tarkista config.h |
| LoRa ei päivity | AT-komento epäonnistuu | Tarkista serial output |
| Asetukset nollautuvat | Reboot | Normaali, ei tallennu EEPROM:iin |

---

## FEATURE 3: WiFi Access Point 📡

**Tarkoitus:** Luo WiFi-verkko ja websivu konfigurointia varten

### config.h -asetukset

```cpp
#define ENABLE_WIFI_AP true  // ← Muuta tämä
#define WIFI_AP_SSID "LoRa_Roboter_9"
#define WIFI_AP_PASSWORD "roboter123"
#define WIFI_AP_CHANNEL 6
#define WEB_SERVER_PORT 80
```

### ⚠️ HUOM: Ei vielä toteutettu täysin!

Tämä ominaisuus on valmisteltu, mutta vaatii:
1. WiFi.h ja WebServer.h kirjastot
2. HTML-sivun koodin
3. Lisätoteutus main .ino -tiedostoon

**Kun toteutetaan:**
1. ✅ ESP32 luo WiFi-verkon "LoRa_Roboter_9"
2. ✅ Yhdistä WiFi:llä, salasana: roboter123
3. ✅ Avaa selain: http://192.168.4.1
4. ✅ Näkyy websivu: asetukset, tilastot, graafit

**Testaus tulee myöhemmin!**

---

## FEATURE 4: Advanced Remote Commands 🎮

**Tarkoitus:** Laajempi komentopaletti LoRa-yhteydellä

### config.h -asetukset

```cpp
#define ENABLE_ADVANCED_COMMANDS true  // ← Muuta tämä
```

### Testausvaiheet

#### Testi 1: Perustoiminta
1. ✅ Lataa koodi **molempiin** laitteisiin
2. ✅ Odota Serial outputissa:
   ```
   ✓ Advanced commands enabled
     Available commands:
       CMD:STATUS, CMD:RESET_STATS, CMD:PING
       CMD:SET_POWER:X, CMD:SET_SF:X
       CMD:LED_ON, CMD:LED_OFF, CMD:LED_BLINK:X
       CMD:GET_RSSI, CMD:GET_BATTERY
   ```

#### Testi 2: PING-testi
1. ✅ Sender: Lähetä payload: `CMD:PING`
2. ✅ Receiver Serial:
   ```
   📡 Remote command received: PING
   → Responding to PING with PONG
   ```
3. ✅ Sender vastaanottaa: `PONG`

#### Testi 3: STATUS-kysely
1. ✅ Sender: Lähetä `CMD:STATUS`
2. ✅ Receiver Serial:
   ```
   📡 Remote command received: STATUS
   → Sending status report
   ```
3. ✅ Sender vastaanottaa:
   ```
   STATUS,UPTIME:123s,HEAP:145KB,RSSI:-85,SNR:7,
   LOSS:2.5%,STATE:OK,TX:45,RX:45
   ```

#### Testi 4: LED-ohjaus
1. ✅ Sender: Lähetä `CMD:LED_ON`
2. ✅ Receiver: LED syttyy, Serial:
   ```
   📡 Remote command received: LED_ON
   ✓ LED turned ON
   ```
3. ✅ Sender: Lähetä `CMD:LED_OFF`
4. ✅ Receiver: LED sammuu
5. ✅ Sender: Lähetä `CMD:LED_BLINK:5`
6. ✅ Receiver: LED vilkkuu 5 kertaa

#### Testi 5: LoRa-asetusten muutos
1. ✅ Sender: Lähetä `CMD:SET_POWER:10`
2. ✅ Receiver Serial:
   ```
   📡 Remote command received: SET_POWER:10
   ✓ TX power set to 10 dBm
   ```
3. ✅ Seuraavat viestit lähetetään 10 dBm teholla

#### Testi 6: Tilastojen nollaus
1. ✅ Sender: Lähetä `CMD:RESET_STATS`
2. ✅ Receiver Serial:
   ```
   ✓ Statistics reset
   ```
3. ✅ Packet loss palaa 0%:iin

#### Testi 7: RSSI-kysely
1. ✅ Sender: Lähetä `CMD:GET_RSSI`
2. ✅ Receiver vastaa: `RSSI:-85,SNR:7`
3. ✅ Sender näkee etälaitteen signaalin laadun

#### Testi 8: Akun kysely (jos Battery Monitor päällä)
1. ✅ Aseta `ENABLE_BATTERY_MONITOR true`
2. ✅ Sender: Lähetä `CMD:GET_BATTERY`
3. ✅ Receiver vastaa: `BATTERY:3.85V`

#### Testi 9: Komentostatistiikat
1. ✅ Lähetä useita komentoja
2. ✅ Receiver: Katso muuttujaa `cmdStats`
3. ✅ Tai lisää koodi:
   ```cpp
   extern void printCommandStats();
   printCommandStats();  // Kutsu loop():ssa
   ```

### Testattavat komennot

| Komento | Odotus | Testi |
|---------|--------|-------|
| `CMD:PING` | PONG palautuu | ✅ |
| `CMD:STATUS` | Full status | ✅ |
| `CMD:RESET_STATS` | Nollaa laskurit | ✅ |
| `CMD:LED_ON` | LED päälle | ✅ |
| `CMD:LED_OFF` | LED pois | ✅ |
| `CMD:LED_BLINK:3` | Vilkkuu 3× | ✅ |
| `CMD:SET_POWER:10` | Teho 10 dBm | ✅ |
| `CMD:SET_SF:10` | SF muuttuu | ✅ |
| `CMD:GET_RSSI` | RSSI palautuu | ✅ |
| `CMD:GET_BATTERY` | Jännite (jos enabled) | ✅ |

### Mahdolliset ongelmat

| Ongelma | Syy | Ratkaisu |
|---------|-----|----------|
| Ei reagoi | Feature disabled | config.h tarkistus |
| Komento ei toimi | Payload liian pitkä | Lyhennä komentoa |
| Ei vastausta | ACK timeout | Tarkista bi-directional |

---

## FEATURE 5: Performance Monitoring 📊

**Tarkoitus:** Seuraa järjestelmän suorituskykyä ja muistia

### config.h -asetukset

```cpp
#define ENABLE_PERFORMANCE_MONITOR true  // ← Muuta tämä
#define PERF_REPORT_INTERVAL 60000       // 60 sekuntia
```

### Testausvaiheet

#### Testi 1: Perustoiminta
1. ✅ Lataa koodi
2. ✅ Odota Serial outputissa:
   ```
   ✓ Performance monitor initialized
     Initial free heap: 245 KB
     Report interval: 60 seconds
   ```

#### Testi 2: Ensimmäinen raportti (60s)
1. ✅ Odota 60 sekuntia
2. ✅ Serial näyttää:
   ```
   ╔═══════════════ PERFORMANCE REPORT ═══════════════╗
   ║ Report #1
   ║ Uptime:        1 min 0 sec
   ║ Loop freq:     450 Hz ✓ Good
   ║ Total loops:   27000
   ║ Free heap:     243 KB ✓
   ║ Min heap:      238 KB
   ║ Initial heap:  245 KB
   ║ Memory used:   2 KB (0%)
   ╚══════════════════════════════════════════════════╝
   ```

#### Testi 3: Loop-taajuus
- ✅ **450 Hz** → ✓ Good
- ✅ **>1000 Hz** → ✓ Excellent
- ✅ **<10 Hz** → ⚠️ SLOW! (ongelma)

#### Testi 4: Muistivuototesti
1. ✅ Anna laitteen pyöriä 10-30 minuuttia
2. ✅ Tarkista raportit:
   - `Min heap` pysyy vakaana → ✅ OK
   - `Min heap` laskee jatkuvasti → ⚠️ Memory leak!
3. ✅ Jos leak havaitaan:
   ```
   ⚠️ POSSIBLE MEMORY LEAK DETECTED!
      Min heap dropped from 238 KB to 228 KB
   ```

#### Testi 5: Matalan muistin varoitus
1. ✅ Simuloi (väliaikainen testi):
   ```cpp
   // performance_monitor.h, rivi ~23:
   #define MEMORY_WARNING_THRESHOLD 300  // Korkea raja testiin
   ```
2. ✅ Serial näyttää:
   ```
   ⚠️ LOW MEMORY WARNING!
      Free heap: 243 KB
   ```

#### Testi 6: Kuormitustesti
1. ✅ Lisää raskasta koodia loop():een:
   ```cpp
   // Väliaikainen testi
   for (int i = 0; i < 1000000; i++) {
     volatile int x = i * 2;
   }
   ```
2. ✅ Tarkista: Loop freq laskee
3. ✅ Poista testaus

### Tulkinta

**Loop Frequency:**
- >1000 Hz → Loistava, paljon aikaa muulle
- 100-1000 Hz → Hyvä, normaali
- 10-100 Hz → OK, mutta lähellä rajaa
- <10 Hz → ⚠️ Hidas! Jotain vialla

**Free Heap:**
- >200 KB → Paljon tilaa
- 100-200 KB → Normaali
- 50-100 KB → Vähän tilaa
- <50 KB → Matala, varoitus!

**Memory Leak:**
- Min heap vakio → OK
- Min heap -1 KB/tunti → Pieni vuoto
- Min heap -10 KB/tunti → Vakava vuoto!

### Mahdolliset ongelmat

| Ongelma | Syy | Ratkaisu |
|---------|-----|----------|
| Loop freq < 10 Hz | Delay() liikaa | Poista/vähennä delayja |
| Memory leak | String-käyttö väärä | Tarkista koodi |
| Low memory | Liikaa muuttujia | Optimoi, käytä PROGMEM |

---

## FEATURE 6: Watchdog Timer 🐕

**Tarkoitus:** Automaattinen uudelleenkäynnistys jos järjestelmä jumittuu

### config.h -asetukset

```cpp
#define ENABLE_WATCHDOG true  // ← Muuta tämä
#define WATCHDOG_TIMEOUT_S 10  // 10 sekuntia
```

### ⚠️ HUOM: Vaatii implementoinnin!

Tämä ominaisuus on valmisteltu, mutta vaatii:
1. `#include <esp_task_wdt.h>` main .ino:ssa
2. `esp_task_wdt_init()` setup():issa
3. `esp_task_wdt_reset()` loop():issa

**Kun toteutetaan:**
1. ✅ Watchdog käynnistyy
2. ✅ Jos loop() ei kutsuttu 10 sekuntiin → reboot
3. ✅ Serial: "Brownout detector was triggered" TAI watchdog-viesti

**Testaus tulee myöhemmin!**

---

## FEATURE 7-10: Muut ominaisuudet 🚀

### FEATURE 7: Encryption
- XOR-salaus payloadille
- Tarvitsee implementoinnin
- Testaus: Lähetä/vastaanota salattua dataa

### FEATURE 8: Extended Telemetry
- Lisää uptime, heap, temp payloadiin
- Tarvitsee implementoinnin
- Testaus: CSV näyttää lisätiedot

### FEATURE 9: Adaptive SF
- Automaattinen SF-säätö RSSI:n mukaan
- Tarvitsee implementoinnin
- Testaus: SF muuttuu etäisyyden mukaan

### FEATURE 10: Packet Statistics
- Yksityiskohtaiset pakettitilastot
- Tarvitsee implementoinnin
- Testaus: Tilastoraportit

---

## 📋 Yhteenveto: Testausmatriisi

| Feature | Status | Laitteisto tarvitaan? | Testausaika | Prioriteetti |
|---------|--------|------------------------|-------------|--------------|
| #1 Battery Monitor | ✅ Valmis | Kyllä (voltage divider) | 15 min | ⭐⭐⭐ Korkea |
| #2 Runtime Config | ✅ Valmis | Ei | 10 min | ⭐⭐⭐ Korkea |
| #3 WiFi AP | 🔲 Ei toteutettu | Ei | - | ⭐⭐ Keskitaso |
| #4 Advanced Commands | ✅ Valmis | Kyllä (2 laitetta) | 20 min | ⭐⭐⭐ Korkea |
| #5 Performance Monitor | ✅ Valmis | Ei | 5 min | ⭐⭐ Keskitaso |
| #6 Watchdog | 🔲 Ei toteutettu | Ei | - | ⭐ Matala |
| #7 Encryption | 🔲 Ei toteutettu | Kyllä (2 laitetta) | - | ⭐ Matala |
| #8 Extended Telemetry | 🔲 Ei toteutettu | Ei | - | ⭐ Matala |
| #9 Adaptive SF | 🔲 Ei toteutettu | Kyllä | - | ⭐⭐ Keskitaso |
| #10 Packet Stats | 🔲 Ei toteutettu | Ei | - | ⭐ Matala |

---

## 🎯 Testausjärjestys (suositus)

**Ilman lisälaitteistoa (testaa ensin):**
1. ✅ **Feature #5: Performance Monitor** (5 min)
   - Helpoin, ei vaadi mitään lisää
   - Katsotaan että järjestelmä toimii

2. ✅ **Feature #2: Runtime Config** (10 min)
   - Serial-komennot
   - Testaa että voit muuttaa asetuksia

**Yhden laitteen kanssa:**
3. ✅ **Feature #1: Battery Monitor** (15 min)
   - Tarvitsee 2× 10kΩ vastukset
   - Tarvitsee akun/jännitelähteen

**Kahden laitteen kanssa:**
4. ✅ **Feature #4: Advanced Commands** (20 min)
   - Molemmissa laittissa sama koodi
   - Testaa etäkomennot

**Myöhemmin (kun toteutettu):**
- Feature #3: WiFi AP
- Feature #6: Watchdog
- Feature #7-10: Muut

---

## 💡 Vinkkejä

**Debuggaukseen:**
- Lisää Serial.println()-viestejä
- Käytä `#if ENABLE_XXX` -lohkoja
- Tarkista että feature on `true` config.h:ssa

**Ongelmatilanteissa:**
- Disabloi kaikki featuret
- Testaa yksi kerrallaan
- Tarkista Serial output virheviestien varalta

**Tallenna tulokset:**
- Käytä HARDWARE_TESTING_CHECKLIST.md
- Kirjaa ylös kaikki havainnot
- Ota screenshotteja Serial outputista

---

**Onnea testauksiin! 🚀**

Jos jokin ei toimi, katsothan:
1. `config.h` - Onko feature `true`?
2. Serial output - Mitä virheilmoituksia?
3. Laitteistokytkennät - Ovatko oikein?
