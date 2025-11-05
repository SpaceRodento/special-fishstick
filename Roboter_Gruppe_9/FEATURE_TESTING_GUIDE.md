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
#define ENABLE_WATCHDOG true   // ← Muuta tämä
#define WATCHDOG_TIMEOUT_S 10  // 10 sekuntia
```

### Testausvaiheet

#### Testi 1: Perustoiminta
1. ✅ Lataa koodi
2. ✅ Odota Serial outputissa:
   ```
   🐕 Initializing watchdog timer (10s timeout)...
   ✓ Watchdog timer enabled
     System will auto-reboot if loop() hangs
     Timeout: 10 seconds
     ⚠️  IMPORTANT: loop() must run smoothly!
   ```

#### Testi 2: Normaali toiminta
1. ✅ Anna laitteen pyöriä 5-10 minuuttia
2. ✅ Ei pitäisi reboot:ata
3. ✅ Watchdog resetoidaan joka loop-kierroksella

#### Testi 3: Jumittumistesti (VAROITUS: Tämä rebootaa!)
1. ✅ Lisää väliaikaisesti koodiin (loop():iin):
   ```cpp
   #if ENABLE_WATCHDOG
     extern void testWatchdogTimeout();
     testWatchdogTimeout();  // Kutsuu vain kerran
   #endif
   ```
2. ✅ Lataa koodi
3. ✅ Serial näyttää:
   ```
   ⚠️⚠️⚠️ WATCHDOG TEST MODE ⚠️⚠️⚠️
   Simulating system hang...
   ESP32 will reboot in 10 seconds
   This is a TEST - do not use in production!
   ..........
   ```
4. ✅ 10 sekunnin kuluttua: ESP32 rebootaa
5. ✅ Serial: "rst:0x8 (TG1WDT_SYS_RESET),boot:0x..."
6. ✅ Poista testifunktion kutsu!

#### Testi 4: Tilastot
1. ✅ Tulosta watchdog-tilastot:
   ```cpp
   extern void printWatchdogStats();
   printWatchdogStats();  // Kutsu esim. 1× minuutissa
   ```
2. ✅ Serial näyttää:
   ```
   ╔═══════ WATCHDOG STATISTICS ═══════╗
   ║ Status:          ENABLED ✓
   ║ Timeout:         10 seconds
   ║ Total resets:    12345
   ║ Last reset:      0 s ago
   ║ Max interval:    85 ms
   ║ Max usage:       0.8% of timeout ✓
   ║ Safety margin:   9915 ms
   ╚═══════════════════════════════════╝
   ```

#### Testi 5: Varoitukset
1. ✅ Jos loop() hidastuu (max interval > 8 seconds):
   ```
   ⚠️  Watchdog: Long interval (8500 ms, timeout in 1500 ms)
   ```

### Mahdolliset ongelmat

| Ongelma | Syy | Ratkaisu |
|---------|-----|----------|
| Ei rebootaa jumissa | Feature disabled | Tarkista config.h |
| Rebootaa heti | Timeout liian lyhyt | Kasvata WATCHDOG_TIMEOUT_S |
| False triggers | Loop() liian hidas | Poista delay(), optimoi |

---

## FEATURE 7: Encryption (XOR) 🔒

**Tarkoitus:** Salaa LoRa-viestit yksinkertaisella XOR-salauksella

### config.h -asetukset

```cpp
#define ENABLE_ENCRYPTION true  // ← Muuta tämä
#define ENCRYPTION_KEY 0xA5     // Salausk avain (0x00-0xFF)
```

### ⚠️ TÄRKEÄ TURVALLISUUSHUOMIO

**XOR EI OLE kryptografisesti turvallinen!**
- Sopii vain perus-obfuskaatioon
- ÄLÄ käytä arkaluonteisiin tietoihin (salasanat, henkilötiedot)
- Voidaan murtaa helposti taajuusanalyysillä
- Oikeaan turvallisuuteen: AES-128/256 (ei toteutettu)

### Testausvaiheet

#### Testi 1: Perustoiminta
1. ✅ Aseta **molemmissa** laitteissa sama avain:
   ```cpp
   #define ENABLE_ENCRYPTION true
   #define ENCRYPTION_KEY 0xA5
   ```
2. ✅ Lataa koodi molempiin
3. ✅ Serial näyttää:
   ```
   🔒 Encryption enabled
     Algorithm: XOR cipher
     Key: 0xA5
     ⚠️  WARNING: XOR is NOT cryptographically secure!
     Use for basic obfuscation only
     Both devices MUST use same key!
   ```

#### Testi 2: Salauksen testaus
1. ✅ Kutsu testiä setup():issa:
   ```cpp
   #if ENABLE_ENCRYPTION
     extern void testEncryption();
     testEncryption();
   #endif
   ```
2. ✅ Serial näyttää:
   ```
   🔒 Testing encryption...
   Original:  LED:1,TEMP:25.5
   Hex:       4C 45 44 3A 31 2C 54 45 4D 50 ...
   Encrypted: E9 E0 E1 9F 94 89 F1 E0 E8 F5 ...
   Decrypted: LED:1,TEMP:25.5
   ✓ Encryption test PASSED
   ```

#### Testi 3: Viestintä salatulla yhteydellä
1. ✅ Lähetä viestejä normaalisti
2. ✅ Viestit toimivat (salataan lähettäessä, puretaan vastaanotettaessa)
3. ✅ **Ilma-aaltojen yli:** Viestit ovat salattuja
4. ✅ **Serial outputissa:** Näkyy selväkielisenä (purettu)

#### Testi 4: Väärä avain
1. ✅ Aseta laitteisiin **ERI avaimet:**
   - Laite 1: `ENCRYPTION_KEY 0xA5`
   - Laite 2: `ENCRYPTION_KEY 0x5A`
2. ✅ Viestit eivät parse:oidu oikein
3. ✅ Serial: Roskapayload

#### Testi 5: Salauksen/purkamisen debug
1. ✅ Käytä debug-funktioita:
   ```cpp
   String encrypted = encryptWithDebug("LED:1");
   String decrypted = decryptWithDebug(encrypted);
   ```
2. ✅ Serial näyttää yksityiskohtaiset hex-dumpit

### Suorituskyky

- Nopeus: <1ms tyypilliselle payloadille
- Ei havaittavaa viivettä
- Toimii kaikilla SF-arvoilla

---

## FEATURE 8: Extended Telemetry 📊

**Tarkoitus:** Lisää ylimääräisiä diagnostiikkatietoja payload:iin

### config.h -asetukset

```cpp
#define ENABLE_EXTENDED_TELEMETRY true  // ← Muuta tämä
```

### Mitä dataa lisätään?

Payload muuttuu:
```
Vanha: SEQ:123,LED:1,TOUCH:0
Uusi:  SEQ:123,LED:1,TOUCH:0,UP:3600,HEAP:245,MHEAP:238,TEMP:42,LOOP:450
```

Lisätyt kentät:
- **UP:** Uptime (seconds)
- **HEAP:** Free heap (KB)
- **MHEAP:** Min free heap (KB) - muistivuototunnistus
- **TEMP:** Sisälämpötila (°C) - tarkkuus ±5°C
- **LOOP:** Loop-taajuus (Hz) - jos Performance Monitor päällä

### Testausvaiheet

#### Testi 1: Perustoiminta
1. ✅ Lataa koodi
2. ✅ Serial näyttää:
   ```
   📊 Extended telemetry enabled
     Monitoring:
       - System uptime
       - Free heap memory
       - Internal temperature
       - Loop frequency
     ⚠️  Payload size increased by ~35 bytes
   ```

#### Testi 2: Payloadin tarkastelu
1. ✅ Tarkista Serial output
2. ✅ Pitäisi nähdä lisäkenttiä:
   ```
   →RCV: SEQ:45,LED:1,TOUCH:0,UP:125,HEAP:243,MHEAP:238,TEMP:42.5,LOOP:450
   ```

#### Testi 3: Telemetrian tulostus
1. ✅ Kutsu raportointia:
   ```cpp
   #if ENABLE_EXTENDED_TELEMETRY
     extern void printTelemetry();
     printTelemetry();  // 1× minuutissa
   #endif
   ```
2. ✅ Serial näyttää:
   ```
   ╔════════ EXTENDED TELEMETRY ════════╗
   ║ Uptime:          2 min 5 sec
   ║ Free heap:       243 KB
   ║ Min heap:        238 KB
   ║ Temperature:     42.3 °C
   ║ Loop freq:       450 Hz
   ║ Updates:         125
   ╚════════════════════════════════════╝
   ```

#### Testi 4: Muistivuodon havaitseminen
1. ✅ Anna pyöriä 30-60 minuuttia
2. ✅ Tarkista MHEAP:
   - Vakio → ✅ OK
   - Laskee jatkuvasti → ⚠️ Memory leak!
3. ✅ Serial varoitus:
   ```
   ║ ⚠️  Memory leak detected!
   ```

#### Testi 5: Lämpötilaseuranta
1. ✅ Katso TEMP-arvo
2. ✅ Normaali: 35-50°C
3. ✅ Korkea: >80°C → Serial varoitus
4. ✅ Tarkkuus ±5°C (vain trendeille!)

### Python-skriptien päivitys

Jos käytät PC-loggausta, päivitä parserit:
```python
# data_logger.py, lisää kentät:
cursor.execute('''
    CREATE TABLE IF NOT EXISTS lora_messages (
        ...,
        uptime INTEGER,
        free_heap INTEGER,
        min_heap INTEGER,
        temperature REAL,
        loop_freq INTEGER
    )
''')
```

---

## FEATURE 9: Adaptive Spreading Factor 📡

**Tarkoitus:** Automaattisesti säädä SF signaalin laadun mukaan

### config.h -asetukset

```cpp
#define ENABLE_ADAPTIVE_SF true          // ← Muuta tämä
#define ADAPTIVE_SF_RSSI_GOOD -80        // Laske SF tämän yläpuolella
#define ADAPTIVE_SF_RSSI_WEAK -105       // Nosta SF tämän alapuolella
```

### Miten toimii?

1. Seuraa RSSI:tä jatkuvasti
2. Jos RSSI > -80 dBm → Laske SF (nopeampi siirto)
3. Jos RSSI < -105 dBm → Nosta SF (parempi kantama)
4. Odota vakiintumista ennen seuraavaa muutosta
5. Molemmat laitteet synkronoivat SF:n

### SF-taulukko

| SF | Nopeus | Kantama | Ilma-aika | Herkkyys |
|----|--------|---------|-----------|----------|
| 7 | 5.5 kbps | 2 km | 41 ms | -123 dBm |
| 10 | 1.0 kbps | 5 km | 288 ms | -132 dBm |
| 12 | 0.3 kbps | 10 km | 991 ms | -137 dBm |

### Testausvaiheet

#### Testi 1: Perustoiminta
1. ✅ Aseta **molemmissa** laitteissa:
   ```cpp
   #define ENABLE_ADAPTIVE_SF true
   ```
2. ✅ Lataa molempiin
3. ✅ Serial näyttää:
   ```
   📡 Adaptive Spreading Factor enabled
     Initial SF: SF12
     Good RSSI threshold: -80 dBm
     Weak RSSI threshold: -105 dBm
     Cooldown: 30 seconds
     ⚠️  Both devices must have this enabled!
   ```

#### Testi 2: SF-muutos (hyvä signaali)
1. ✅ Aloita lähietäisyydeltä (<10m)
2. ✅ RSSI pitäisi olla > -80 dBm
3. ✅ 30-60 sekunnin kuluttua Serial:
   ```
   ╔════ ADAPTIVE SF ════╗
   ║ Current SF:  SF12
   ║ Avg RSSI:    -65.2 dBm
   ║ Target SF:   SF11
   ║ Reason:      Strong signal → Faster speed
   ╚═════════════════════╝
   📡 Applying SF11...
   ✓ SF changed to SF11
   ```
4. ✅ SF laskee asteittain: SF12 → SF11 → SF10 → ...

#### Testi 3: SF-muutos (heikko signaali)
1. ✅ Siirrä laitteet kauas toisistaan (100m+)
2. ✅ RSSI laskee < -105 dBm
3. ✅ SF nousee: SF7 → SF8 → SF9 → ... → SF12
4. ✅ Serial:
   ```
   ╔════ ADAPTIVE SF ════╗
   ║ Current SF:  SF7
   ║ Avg RSSI:    -110.5 dBm
   ║ Target SF:   SF8
   ║ Reason:      Weak signal → Better range
   ╚═════════════════════╝
   ```

#### Testi 4: SF-synkronointi
1. ✅ Tarkista että molemmat laitteet käyttävät samaa SF:ää
2. ✅ Vastaanottaja Serial:
   ```
   📡 Remote requests SF change to SF10
   ✓ SF changed to SF10
   → Sending ACK: CMD:SF_ACK:10
   ```

#### Testi 5: SF-tilanne
1. ✅ Tulosta status:
   ```cpp
   extern void printAdaptiveSFStatus();
   printAdaptiveSFStatus();
   ```
2. ✅ Serial:
   ```
   ╔═══════ ADAPTIVE SF STATUS ═══════╗
   ║ Current SF:      SF10
   ║ Avg RSSI:        -92.3 dBm
   ║ Changes:         5
   ║ Time since last: 45 s
   ║ Samples:         10 / 10
   ║ Status:          ✓ STABLE
   ╚══════════════════════════════════╝
   ```

#### Testi 6: Pakota SF (debugging)
1. ✅ Pakota SF:
   ```cpp
   forceSpreadingFactor(8);  // Pakota SF8
   ```
2. ✅ Ohittaa adaptiivisen logiikan

### Ongelmat

| Ongelma | Syy | Ratkaisu |
|---------|-----|----------|
| Jatkuvat SF-muutokset | Epävakaa RSSI | Kasvata cooldown-aikaa |
| Ei synkronoi | Vain toisessa päällä | Molemmat ENABLE_ADAPTIVE_SF true |
| Packet loss | SF-transition | Normaalia, menee ohi |

---

## FEATURE 10: Packet Statistics 📈

**Tarkoitus:** Yksityiskohtaiset tilastot pakettiliikenteestä

### config.h -asetukset

```cpp
#define ENABLE_PACKET_STATS true        // ← Muuta tämä
#define PACKET_STATS_INTERVAL 30000     // 30 sekuntia
```

### Mitä seurataan?

- Vastaanotetut/menetetyt paketit
- Duplikaatit ja järjestyksestä poikkeavat
- RSSI/SNR min/max/avg
- Pakettien väli ja jitter
- Häviöputket (loss streaks)
- Recovery-onnistumisprosentti

### Testausvaiheet

#### Testi 1: Perustoiminta
1. ✅ Lataa koodi
2. ✅ Serial näyttää:
   ```
   📈 Packet statistics enabled
     Report interval: 30 seconds
     Tracking:
       - Duplicates, out-of-order packets
       - RSSI/SNR min/max/avg
       - Packet timing and jitter
       - Loss streaks and recovery
   ```

#### Testi 2: Ensimmäinen raportti (30s)
1. ✅ Odota 30 sekuntia
2. ✅ Serial näyttää:
   ```
   ╔═══════════════ PACKET STATISTICS ═══════════════╗
   ║ Report #1
   ║
   ║ RECEPTION:
   ║   Packets received:    145
   ║   Packets lost:        3 (2.03%)
   ║   Duplicates:          0
   ║   Out-of-order:        1
   ║
   ║ RSSI (dBm):
   ║   Average:             -85.3
   ║   Min:                 -95
   ║   Max:                 -78
   ║   Range:               17
   ║
   ║ SNR (dB):
   ║   Average:             7.2
   ║   Min:                 4
   ║   Max:                 10
   ║
   ║ TIMING:
   ║   Avg interval:        2050 ms
   ║   Min interval:        1985 ms
   ║   Max interval:        2150 ms
   ║   Jitter:              25.3 ms
   ║
   ║ LOSS STREAKS:
   ║   Current streak:      0
   ║   Max streak:          2
   ║   Total streaks:       2
   ╚════════════════════════════════════════════════╝
   ```

#### Testi 3: Duplikaattien havaitseminen
1. ✅ Jos duplikaatteja:
   ```
   📋 Duplicate packet: SEQ:42
   ║   Duplicates:          1
   ```

#### Testi 4: Järjestyksestä poikkeavat
1. ✅ Jos OOO-paketteja:
   ```
   🔀 Out-of-order packet: Expected SEQ:50, Got:52
   ║   Out-of-order:        1
   ```

#### Testi 5: Loss streaks
1. ✅ Simuloi häviö: sammuta sender 10 sekunniksi
2. ✅ Receiver:
   ```
   ║   Current streak:      5
   ║   Max streak:          5
   ```
3. ✅ Käynnistä sender uudelleen → streak nollautuu

#### Testi 6: Nollaa tilastot
1. ✅ Nollaa:
   ```cpp
   extern void resetPacketStats();
   resetPacketStats();
   ```
2. ✅ Serial:
   ```
   🔄 Resetting packet statistics...
   ✓ Statistics reset
   ```

### CSV-output

Lisää CSV:hen yksityiskohtaiset tilastot:
```
...,RX:145,LOST:3,LOSS%:2.03,RSSI_AVG:-85.3,JITTER:25.3
```

---

## FEATURE 11: Audio Detection (Palovaroittimen äänitarkkailu) 🔊

**Tarkoitus:** Havaitsee palovaroittimen hälytysäänen ja lähettää hälytyksen LoRa-verkossa

### Laitteistokytkentä

```
MAX4466 Microphone Amplifier → ESP32
───────────────────────────────────
VCC   →  3.3V
GND   →  GND
OUT   →  GPIO 34 (ADC1_CH6)
GAIN  →  (adjustable potentiometer - säädä herkkyyttä)
```

**Laitteiston hankinta:**
- MAX4466 Electret Microphone Amplifier
- Hinta: ~3-5€
- Tilaus: AliExpress, Amazon, elektroniikkakaupat
- Suositus: Osta säädettävällä gainilla (potentiometri)

**Miksi MAX4466?**
- Säädettävä gain (25-125×)
- Vähäkohinainen
- Analog output (helppo ESP32:lle)
- Rail-to-rail output (0-3.3V)

### config.h -asetukset

```cpp
#define ENABLE_AUDIO_DETECTION true      // ← Muuta tämä
#define AUDIO_PIN 34                     // ADC1_CH6
#define AUDIO_SAMPLES 100                // RMS-laskentaan
#define AUDIO_THRESHOLD 200              // RMS kynnys
#define AUDIO_PEAK_MIN 3                 // Min peaks/sekunti
#define AUDIO_PEAK_MAX 5                 // Max peaks/sekunti
#define AUDIO_COOLDOWN 5000              // 5s välein hälytykset
```

### Palovaroittimen ääniominaisuudet

**Tyypillinen palovaroitin:**
- Taajuus: ~3 kHz (korkea ääni)
- Voimakkuus: 85 dB @ 3 metriä
- Kuvio: 3-4 piippauksia sekunnissa
- Kesto: Jatkuva kunnes sammutetaan

### Testausvaiheet

#### Testi 1: Perustoiminta ja kalibrointi
1. ✅ Kytke MAX4466 GPIO 34:ään
2. ✅ Lataa koodi
3. ✅ Serial näyttää:
   ```
   🔊 Audio detection initialized
     Pin: GPIO 34 (ADC1_CH6)
     Sample rate: 100 samples/update
     RMS threshold: 200
     Pattern: 3-5 peaks per second
     Cooldown: 5000 ms
   ```
4. ✅ Kalibroi ympäristön melutaso:
   ```
   📡 Calibrating audio baseline...
      Ensure quiet environment for calibration
      Measuring for 3 seconds...
   .........
   ✓ Ambient noise level: 45 RMS
     Recommended threshold: 95
   ```

**Huom:** Jos RMS > 200 normaalisti → säädä `AUDIO_THRESHOLD` korkeammaksi!

#### Testi 2: Äänitason seuranta
1. ✅ Odota normaali päivitys (200ms)
2. ✅ Serial näyttää hiljaisessa tilassa:
   ```
   🔊 Audio update: RMS:42, Peaks:0, Alarm:NO
   ```
3. ✅ Taputa mikrofonia:
   ```
   🔊 Audio update: RMS:385, Peaks:0, Alarm:NO
   ```
4. ✅ Tarkista että RMS reagoi ääniin

#### Testi 3: Hälytyskuvion tunnistus (ilman palovaroitinta)
1. ✅ **Vaihtoehto A:** Käytä älypuhelimen äänigeneraattoria
   - Lataa sovellus: "Tone Generator" tai "Signal Generator"
   - Aseta: 3000 Hz (3 kHz)
   - Soita toistuvaa ääntä 3-4× sekunnissa

2. ✅ **Vaihtoehto B:** Käytä tietokoneohjelma
   - Avaa: https://www.szynalski.com/tone-generator/
   - Aseta 3000 Hz
   - Paina play/pause rytmisesti

3. ✅ Serial näyttää kuvion tunnistuksen:
   ```
   💡 Peak detected! Count: 1
   💡 Peak detected! Count: 2
   💡 Peak detected! Count: 3
   🚨🚨🚨 FIRE ALARM AUDIO DETECTED! 🚨🚨🚨
     RMS value: 450
     Peaks detected: 3
     Sending LoRa alert...
   ```

#### Testi 4: LoRa-hälytyksen lähetys
1. ✅ Kun hälytys havaitaan → Serial:
   ```
   → Sending: ALERT:FIRE_AUDIO,RMS:450,PEAKS:3
   +OK
   ```
2. ✅ Toisella laitteella pitäisi näkyä:
   ```
   +RCV=2,14,ALERT:FIRE_AUDIO,RMS:450,PEAKS:3,-85,12
   🚨 FIRE ALERT RECEIVED FROM AUDIO DETECTOR!
   ```

#### Testi 5: Todellinen palovaroitin
1. ✅ **VAROITUS:** Tee tämä ulkona tai turvallisessa paikassa!
2. ✅ Aktivoi palovaroitin (paina testipainiketta)
3. ✅ Serial pitäisi näyttää tunnistus 1-3 sekunnissa
4. ✅ Testaa eri etäisyyksillä:
   - 0.5 metriä: RMS > 400 (todennäköisesti)
   - 1 metri: RMS 250-400
   - 2 metriä: RMS 150-250
   - 3 metriä: RMS 100-150

**Jos ei tunnista:**
- Säädä MAX4466:n gain-potentiometria (käännä myötäpäivään)
- Laske `AUDIO_THRESHOLD` arvoa (esim. 150)
- Tarkista ettei ympäristö ole liian meluisa

#### Testi 6: Väärien hälytysten esto
1. ✅ Testaa normaalilla puheella → Ei hälytystä
2. ✅ Testaa musiikilla → Ei hälytystä
3. ✅ Testaa muilla äänillä → Ei hälytystä
4. ✅ Vain oikea kuvio (3-4 piippaukset/s) → Hälytys

**Jos false positives:**
```
⚠️ False alarm detected! Not smoke alarm pattern.
  Peak count outside range: 8 (expected 3-5)
```

### Vianmääritys

| Ongelma | Syy | Ratkaisu |
|---------|-----|----------|
| RMS aina 0 | Ei kytkentää | Tarkista OUT → GPIO 34 |
| RMS aina 2048 | Väärä referenssi | Tarkista VCC ja GND |
| Liian herkkä | Gain liian korkea | Säädä potentiometria vastapäivään |
| Ei tunnista | Gain liian matala | Säädä potentiometria myötäpäivään |
| Jatkuvasti peaks | Meluisa ympäristö | Suorita uudelleen kalibrointi |

### CSV-output

```
...,AUDIO_RMS:42,AUDIO_ALARM:0,AUDIO_ALERTS:0
```

Hälytyksen aikana:
```
...,AUDIO_RMS:450,AUDIO_ALARM:1,AUDIO_ALERTS:3
```

---

## FEATURE 12: Light Detection (Palovaroittimen valotarkkailu) 💡

**Tarkoitus:** Havaitsee palovaroittimen vilkkuvan punaisen LEDin ja lähettää hälytyksen

### Laitteistokytkentä

```
TCS34725 RGB Color Sensor → ESP32
──────────────────────────────────
VIN   →  3.3V (tai 5V jos level shifter)
GND   →  GND
SDA   →  GPIO 21 (I2C SDA)
SCL   →  GPIO 22 (I2C SCL)
LED   →  3.3V (valinnainen, sensorin valoitus)
INT   →  (ei käytetä)
```

**Laitteiston hankinta:**
- TCS34725 RGB Color Sensor with IR filter
- Hinta: ~8-12€
- Tilaus: Adafruit, AliExpress, Amazon
- Tarvitaan: **Adafruit_TCS34725** Arduino-kirjasto

**Miksi TCS34725?**
- I2C-käyttöliittymä (helppo)
- Mittaa RGB + Lux yhdessä
- Integroitu IR-filtteri
- Säädettävä gain ja integration time
- Hyvä tarkkuus värien erotteluun

### Kirjaston asennus

**Arduino IDE:**
1. Tools → Manage Libraries
2. Etsi: "Adafruit TCS34725"
3. Asenna: "Adafruit TCS34725" (+ riippuvuudet)

**Tai manuaalisesti:**
```bash
git clone https://github.com/adafruit/Adafruit_TCS34725
```

### config.h -asetukset

```cpp
#define ENABLE_LIGHT_DETECTION true      // ← Muuta tämä
// I2C pinnit kiinteät: SDA=21, SCL=22
```

**Main .ino -tiedostoon lisättävä:**
```cpp
#if ENABLE_LIGHT_DETECTION
  #include <Wire.h>
  #include <Adafruit_TCS34725.h>
  #include "light_detector.h"

  // Luo sensor-olio (integration time, gain)
  Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
#endif
```

### Palovaroittimen valo-ominaisuudet

**Tyypillinen palovaroitin:**
- Väri: Punainen (λ ~620-750 nm)
- Kuvio: Vilkkuu 1 Hz (1 välähdys/sekunti)
- Joissakin: Jatkuva punainen valo hälytyksen aikana
- Kirkkaus: Näkyy päivänvalossa

### Testausvaiheet

#### Testi 1: I2C-sensorin tunnistus
1. ✅ Kytke TCS34725 I2C-väylään
2. ✅ Lataa koodi
3. ✅ Serial näyttää:
   ```
   💡 Light detection initialized
     Sensor: TCS34725 RGB Color Sensor
     I2C: SDA=GPIO21, SCL=GPIO22
     Red threshold: 100
     Ratio threshold: 2.0
     🚨 Smoke alarm LED monitoring active
     ⚠️  Requires Adafruit_TCS34725 library!
   ```
4. ✅ Jos sensoria ei löydy:
   ```
   ❌ TCS34725 sensor not found!
      Check:
      - TCS34725 connected?
      - I2C wiring correct?
      - Library installed?
   ```

**I2C-vianetsintä:**
```cpp
// Main .ino -tiedostossa setup():
Wire.begin();
Wire.beginTransmission(0x29);  // TCS34725 I2C address
if (Wire.endTransmission() == 0) {
  Serial.println("✓ TCS34725 found at 0x29");
} else {
  Serial.println("❌ TCS34725 NOT found!");
}
```

#### Testi 2: RGB-arvojen lukeminen
1. ✅ Lisää loop():iin sensor-lukeminen:
   ```cpp
   #if ENABLE_LIGHT_DETECTION
     uint16_t r, g, b, c;
     tcs.getRawData(&r, &g, &b, &c);
     updateLightReadings(r, g, b, c);
     updateLightDetection();
   #endif
   ```
2. ✅ Serial näyttää:
   ```
   🔊 Light update: R:45, G:52, B:48, Lux:145
   ```
3. ✅ Testaa eri väreillä:
   - Valkoinen paperi: R≈G≈B
   - Punainen esine: R > G ja R > B
   - Sininen esine: B > R ja B > G

#### Testi 3: Punaisen valon tunnistus
1. ✅ Käytä punaista LED-taskulamppua tai älypuhelin
2. ✅ Osoita suoraan sensoriin
3. ✅ Serial pitäisi näyttää:
   ```
   🔴 Red light detected! R:255, G:45, B:30
   ```
4. ✅ Tarkista ratio:
   - R/G ≈ 255/45 ≈ 5.7 (> 2.0 ✓)
   - R/B ≈ 255/30 ≈ 8.5 (> 2.0 ✓)

#### Testi 4: Vilkkumiskuvion tunnistus
1. ✅ Vilkuta punaista valoa 1× sekunnissa
2. ✅ Serial näyttää:
   ```
   💡 Flash detected! Count: 1
   💡 Flash detected! Count: 2
   🚨🚨🚨 SMOKE ALARM LIGHT DETECTED! 🚨🚨🚨
     Red value: 255
     Flashes: 2
     Sending LoRa alert...
   ```
3. ✅ Vähintään 2 välähdystä tarvitaan vahvistukseen

#### Testi 5: Todellinen palovaroitin
1. ✅ Asenna sensori noin 0.5-3 metrin päähän palovaroittimesta
2. ✅ Kohdista sensori LEDiin
3. ✅ Aktivoi palovaroitin (testipainike)
4. ✅ Serial näyttää tunnistuksen 1-3 sekunnissa

**Asennus-vinkkejä:**
- Käytä pahviputkea fokusoimaan vain LED (estää häikäisy)
- Vältä suoraa auringonvaloa (voi häiritä)
- Optimaalinen etäisyys: 0.5-3 metriä
- Kohdista tarkasti LEDiin

#### Testi 6: LoRa-hälytyksen lähetys
1. ✅ Kun vilkkuva punainen havaitaan → Serial:
   ```
   → Sending: ALERT:FIRE_LIGHT,RED:255,FLASHES:5
   +OK
   ```
2. ✅ Toisella laitteella:
   ```
   +RCV=2,14,ALERT:FIRE_LIGHT,RED:255,FLASHES:5,-85,12
   🚨 FIRE ALERT FROM LIGHT DETECTOR!
   ```

#### Testi 7: Kalibrointi ja baseline
1. ✅ Suorita baseline-kalibrointi:
   ```cpp
   calibrateLightBaseline();
   ```
2. ✅ Serial näyttää:
   ```
   💡 Calibrating light baseline...
      Ensure normal lighting, no alarm LED
      Measuring for 3 seconds...
   .........
   ✓ Ambient red level: 35
     Recommended threshold: 85
   ```
3. ✅ Päivitä config.h jos tarpeen

### Testausfunktiot

**Testaa sensoria (main .ino):**
```cpp
#if ENABLE_LIGHT_DETECTION
  testLightDetector();  // 10 sekunnin testi
#endif
```

Serial output:
```
💡 Testing light detector...
   Flash red LED at sensor!
   Monitoring for 10 seconds...

R:45 G:52 B:48
R:255 G:45 B:30  🔴 RED!
R:255 G:42 B:28  🔴 RED!
R:50 G:55 B:51
...

✓ Test complete
╔══════ LIGHT DETECTION ══════╗
║ Red:            255
║ Green:          45
║ Blue:           30
║ Red dominant:   YES 🔴
║ Alarm active:   🚨 YES!
║ Flash count:    5
╚═════════════════════════════╝
```

### Vianmääritys

| Ongelma | Syy | Ratkaisu |
|---------|-----|----------|
| Sensor ei löydy | I2C-virhe | Tarkista SDA/SCL kytkentä |
| R=G=B=0 | Ei virta | Tarkista VCC ja GND |
| Ei tunnista punaista | Threshold väärä | Laske RED_THRESHOLD arvoa |
| Tunnistaa kaiken punaiseksi | Liian herkkä | Nosta RED_THRESHOLD |
| Ei flash detection | Liian hidas | Tarkista FLASH_MIN/MAX_INTERVAL |

### CSV-output

```
...,LIGHT_R:45,LIGHT_ALARM:0,LIGHT_ALERTS:0
```

Hälytyksen aikana:
```
...,LIGHT_R:255,LIGHT_ALARM:1,LIGHT_ALERTS:3
```

---

## FEATURE 13: Current Monitoring (Virrankulutuksen mittaus) ⚡

**Tarkoitus:** Mittaa akun virrankulutusta, tehoa ja kokonaisenergiaa INA219-anturilla

### Miksi tämä on hyödyllinen?

- 📊 Näe reaaliaikainen virrankulutus (mA)
- 🔋 Seuraa akkuun jäävä kapasiteetti (mAh)
- ⏱️ Laske jäljellä oleva käyttöaika
- 📈 Havaitse virrankulutuspiikit (esim. LoRa TX)
- 🐛 Debuggaa tehonkulutusongelmia

### Laitteistokytkentä

```
                        ┌─────────────┐
Akku (+) ───────────────┤ VIN+        │
                        │   INA219    │
ESP32 VIN ──────────────┤ VIN-    SDA ├───── GPIO 21 (I2C Data)
                        │         SCL ├───── GPIO 22 (I2C Clock)
GND ─────────────────┬──┤ GND     VCC ├───── 3.3V
                     │  └─────────────┘
                     └─────────────────────── GND

```

**Tärkeää:**
- INA219 on SARJASSA akun ja ESP32:n välissä!
- Mittaa akusta ESP32:lle kulkeva virta
- Käyttää samaa I2C-väylää kuin TCS34725 (light detector)

### config.h -asetukset

```cpp
#define ENABLE_CURRENT_MONITOR true     // ← Muuta tämä
#define CURRENT_MONITOR_I2C_ADDR 0x40   // I2C-osoite (oletus)
#define CURRENT_CHECK_INTERVAL 10000    // Tarkista 10s välein
#define CURRENT_HIGH_THRESHOLD 200      // Varoitus >200mA
#define CURRENT_MAX_THRESHOLD 500       // Kriittinen >500mA
```

### Kirjasto

**Asenna Arduino IDE:ssä:**
1. Avaa Library Manager (Tools → Manage Libraries...)
2. Etsi "Adafruit INA219"
3. Asenna (asentaa automaattisesti myös "Adafruit BusIO")

### Testausvaiheet

#### Testi 1: Perustoiminta

1. ✅ Kytke INA219 ylläolevan kaavion mukaan
2. ✅ Asenna kirjasto (katso yllä)
3. ✅ Lataa koodi
4. ✅ Odota Serial outputissa:
   ```
   === Initializing Current Monitor ===
   ✓ INA219 current monitor initialized
     I2C Address: 0x40
     Calibration: 32V, 2A range
     Check interval: 10 seconds
     High current warning: >200 mA
     Overload warning: >500 mA
   ```

5. ❌ Jos näet virheen:
   ```
   ❌ Failed to find INA219 chip!
      Check wiring:
      - SDA → GPIO 21
      - SCL → GPIO 22
      - VCC → 3.3V
      - GND → GND
      Current monitoring DISABLED
   ```
   → Tarkista kytkennät ja I2C-osoite!

#### Testi 2: Virranmittaus

1. ✅ Odota 10 sekuntia
2. ✅ Serial näyttää:
   ```
   ⚡ Current #1: 85.2 mA, 3.78 V, 322 mW ✓
   ```
3. ✅ Tarkista arvot:
   - Current: 80-100 mA (tyypillinen WiFi/LoRa idle)
   - Voltage: Akun jännite (~3.7V)
   - Power: V × I (esim. 3.78V × 0.085A = 321mW)

#### Testi 3: Virrankulutuspiikit

1. ✅ Lähetä LoRa-viesti (Sender-moodissa automaattista)
2. ✅ Katso hetkellinen piikki:
   ```
   ⚡ Current #5: 142.8 mA, 3.76 V, 537 mW ✓
                  ↑↑↑ LoRa TX-piikki!
   ```
3. ✅ Tyypilliset virrat:
   - Deep sleep: 0.01-0.15 mA
   - CPU idle: 20-50 mA
   - WiFi active: 80-170 mA
   - LoRa TX: 120-140 mA (riippuu tehosta)

#### Testi 4: Tilastot ja energianseuranta

1. ✅ Anna laitteen olla päällä vähintään 100 sekuntia
2. ✅ Serial näyttää 10 mittauksen välein:
   ```
     --- Current Statistics ---
     Average: 89.3 mA
     Range: 82.1 - 145.6 mA
     Peak power: 548 mW
     Energy used: 2.5 mAh (0.009 Wh)
     Est. runtime (2000mAh): 22.4 hours
     Uptime: 104 seconds
   ```
3. ✅ Tarkista:
   - Average = keskimääräinen virrankulutus
   - Energy used = kulutettu energia alusta alkaen
   - Est. runtime = arvioitu käyttöaika 2000mAh akulla

#### Testi 5: Display-integraatio

Jos `ENABLE_DISPLAY_OUTPUT true`:

1. ✅ TFT-näytöllä näkyy:
   ```
   Current: 85 mA
   Power: 322 mW
   Energy: 2.5 mAh
   Voltage: 3.78 V  (jos ei BATTERY_MONITOR)
   ```

#### Testi 6: I2C-osoitteen vaihto (jos konflikti)

Jos käytät useampaa INA219:a tai on osoitekonflikti:

1. ✅ INA219-modulissa A0/A1 jumpperit
2. ✅ Solder A0 → osoite 0x41
3. ✅ Solder A1 → osoite 0x44
4. ✅ Solder A0+A1 → osoite 0x45
5. ✅ Päivitä config.h:
   ```cpp
   #define CURRENT_MONITOR_I2C_ADDR 0x41
   ```

### Testausfunktiot

INA219 tarjoaa suoran lukemisen:

```cpp
#if ENABLE_CURRENT_MONITOR
  Serial.print("Current: ");
  Serial.print(current.current_mA, 1);
  Serial.println(" mA");

  Serial.print("Average: ");
  Serial.print(current.currentAvg, 1);
  Serial.println(" mA");

  Serial.print("Energy used: ");
  Serial.print(current.energyUsed_mAh, 1);
  Serial.println(" mAh");
#endif
```

### Vianmääritys

| Ongelma | Syy | Ratkaisu |
|---------|-----|----------|
| "Failed to find INA219" | I2C-virhe | Tarkista SDA/SCL kytkentä |
| Current = 0 | Ei virta sensoria läpi | Tarkista VIN+/VIN- kytkentä |
| Liian suuri virta | Väärä kalibrointi | Käytä `setCalibration_16V_400mA()` |
| Negatiivinen virta | Väärä suunta | Vaihda VIN+ ↔ VIN- |
| I2C-konflikti TCS34725:n kanssa | Sama väylä | Normaali! Molemmat toimivat samalla väylällä |

### CSV-output

```
...,CURRENT:85.2,VOLTAGE:3.78,POWER:322,ENERGY:2.5
```

### Kalibrointivaihtoehdot

INA219 tukee eri mittausalueita:

```cpp
// current_monitor.h, muuta initCurrentMonitor():

ina219.setCalibration_32V_2A();    // Oletus: 0-32V, ±3.2A
ina219.setCalibration_32V_1A();    // Parempi resoluutio: ±1A
ina219.setCalibration_16V_400mA(); // Matala virta: ±400mA
```

**ESP32:lle suositus:** `32V_2A` (oletus) on riittävä.

### Edistynyt: Runtime-laskenta

```cpp
// Laske jäljellä oleva aika 2000mAh akulla:
float capacity = 2000.0;  // mAh
float runtime = getEstimatedRuntime(capacity);

Serial.print("Runtime left: ");
Serial.print(runtime, 1);
Serial.println(" hours");
```

### Yhteensopivuus

**Toimii yhdessä:**
- ✅ Battery Monitor (molemmat mittaavat jännitettä)
- ✅ Light Detection (sama I2C-väylä)
- ✅ Audio Detection (eri GPIO)
- ✅ Kaikki muut ominaisuudet

**Huom:**
- Jos `ENABLE_BATTERY_MONITOR` ja `ENABLE_CURRENT_MONITOR` molemmat päällä:
  - Battery monitor mittaa jännitteen ADC:llä (GPIO 35)
  - Current monitor mittaa jännitteen INA219:llä (tarkempi!)
  - Display näyttää Battery voltage, current monitor lisää virran/tehon

---

## 📋 Yhteenveto: Testausmatriisi

| Feature | Status | Laitteisto? | Testausaika | Prioriteetti |
|---------|--------|-------------|-------------|--------------|
| #1 Battery Monitor | ✅ Valmis | Kyllä (voltage divider) | 15 min | ⭐⭐⭐ Korkea |
| #2 Runtime Config | ✅ Valmis | Ei | 10 min | ⭐⭐⭐ Korkea |
| #3 WiFi AP | 🔲 Ei toteutettu | Ei | - | ⭐⭐ Keskitaso |
| #4 Advanced Commands | ✅ Valmis | Kyllä (2 laitetta) | 20 min | ⭐⭐⭐ Korkea |
| #5 Performance Monitor | ✅ Valmis | Ei | 5 min | ⭐⭐ Keskitaso |
| #6 Watchdog Timer | ✅ Valmis | Ei | 10 min | ⭐⭐ Keskitaso |
| #7 Encryption (XOR) | ✅ Valmis | Kyllä (2 laitetta) | 15 min | ⭐⭐ Keskitaso |
| #8 Extended Telemetry | ✅ Valmis | Ei | 10 min | ⭐⭐⭐ Korkea |
| #9 Adaptive SF | ✅ Valmis | Kyllä (2 laitetta) | 25 min | ⭐⭐⭐ Korkea |
| #10 Packet Statistics | ✅ Valmis | Ei | 10 min | ⭐⭐ Keskitaso |
| #11 Audio Detection | ✅ **UUSI!** 🔊 | Kyllä (MAX4466) | 20 min | ⭐⭐⭐⭐ Erittäin tärkeä |
| #12 Light Detection | ✅ **UUSI!** 💡 | Kyllä (TCS34725) | 20 min | ⭐⭐⭐⭐ Erittäin tärkeä |
| #13 Current Monitor | ✅ **UUSI!** ⚡ | Kyllä (INA219) | 15 min | ⭐⭐⭐ Korkea |

**Yhteensä:** 12 valmista ominaisuutta, 1 tulossa (#3 WiFi AP)

---

## 🎯 Testausjärjestys (suositus)

**VAIHE 1: Ilman lisälaitteistoa (testaa ensin - 40 min):**

1. ✅ **Feature #5: Performance Monitor** (5 min)
   - Helpoin, ei vaadi mitään lisää
   - Katsotaan että järjestelmä toimii

2. ✅ **Feature #2: Runtime Config** (10 min)
   - Serial-komennot
   - Testaa että voit muuttaa asetuksia

3. ✅ **Feature #6: Watchdog Timer** (10 min) 🆕
   - Turvallisuusominaisuus
   - Testaa jumittumissuojaus

4. ✅ **Feature #8: Extended Telemetry** (10 min) 🆕
   - Lisää dataa payloadiin
   - Muistivuototarkkailu

5. ✅ **Feature #10: Packet Statistics** (10 min) 🆕
   - Yksityiskohtaiset tilastot
   - Debuggaustyökalu

**VAIHE 2: Yhden laitteen kanssa (15 min):**

6. ✅ **Feature #1: Battery Monitor** (15 min)
   - Tarvitsee: 2× 10kΩ vastukset + akku
   - Mittaa akkujännite

**VAIHE 3: Kahden laitteen kanssa (80 min):**

7. ✅ **Feature #4: Advanced Commands** (20 min)
   - Molemmissa laittissa sama koodi
   - Testaa etäkomennot

8. ✅ **Feature #7: Encryption** (15 min) 🆕
   - Molemmissa sama avain
   - Salattu viestintä

9. ✅ **Feature #9: Adaptive SF** (25 min) 🆕
   - Molemmat laitteet synkronoivat
   - Testaa lähellä ja kaukana
   - Vaativain feature!

**VAIHE 4: Sensorit (kun laitteisto saatavilla - 40 min):**

10. ✅ **Feature #11: Audio Detection** (20 min) 🔊 **UUSI!**
    - Tarvitsee: MAX4466 microphone amplifier (~3-5€)
    - Havaitsee palovaroittimen äänen
    - Testaa äänigeneraattorilla tai oikealla palovaroittimella

11. ✅ **Feature #12: Light Detection** (20 min) 💡 **UUSI!**
    - Tarvitsee: TCS34725 RGB sensor (~8-12€)
    - Havaitsee vilkkuvan punaisen LEDin
    - Testaa punaisella LED-taskulampulla tai palovaroittimella

**VAIHE 5: Myöhemmin:**

- ⏳ Feature #3: WiFi AP (ei vielä toteutettu)

---

## 📊 Ominaisuuksien yhteensopivuus

**Voiko käyttää yhtä aikaa?**

| Feature | Yhteensopiva kaikkien kanssa? | Huomiot |
|---------|--------------------------------|---------|
| Battery Monitor | ✅ Kyllä | Ei riippuvuuksia |
| Runtime Config | ✅ Kyllä | Voi muuttaa muita asetuksia |
| Advanced Commands | ✅ Kyllä | Vaatii bi-directional |
| Performance Monitor | ✅ Kyllä | Suositeltu aina päälle |
| Watchdog Timer | ✅ Kyllä | Turvallisuusominaisuus |
| Encryption | ✅ Kyllä | Molemmat laitteet sama avain |
| Extended Telemetry | ⚠️ Payload kasvaa | Voi vaikuttaa kantamaan |
| Adaptive SF | ⚠️ Monimutkainen | Testaa ensin erikseen |
| Packet Statistics | ✅ Kyllä | Vähän muistia (~100 bytes) |
| Audio Detection | ✅ Kyllä | Vaatii GPIO 34 (ADC1_CH6) |
| Light Detection | ✅ Kyllä | Vaatii I2C (GPIO 21/22) |
| Current Monitor | ✅ Kyllä | Vaatii I2C (GPIO 21/22), sama väylä kuin Light Detection |

**Suositellut yhdistelmät:**

**Perus (tuotanto):**
- Performance Monitor
- Watchdog Timer
- Packet Statistics

**Kattava seuranta:**
- Performance Monitor
- Extended Telemetry
- Battery Monitor
- Current Monitor ⚡ **UUSI!**
- Packet Statistics

**Turvallisuus + diagnostiikka:**
- Watchdog Timer
- Advanced Commands
- Encryption
- Extended Telemetry

**Maksimisuorituskyky:**
- Adaptive SF
- Extended Telemetry
- Packet Statistics

**Palovaroittimen hälytys (smoke alarm monitoring):** 🚨 **UUSI!**
- Audio Detection (ääni)
- Light Detection (vilkkuva LED)
- Watchdog Timer (luotettavuus)
- Performance Monitor (diagnostiikka)
- Battery Monitor (jos akku)
- Current Monitor (tehonseuranta) ⚡

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
