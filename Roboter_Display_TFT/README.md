# Roboter Gruppe 9 - Display Station

**UART-pohjainen näyttölaite** ESP32-2432S022:lle (Landscape-tila).

Yksinkertainen, joustava ja helposti integroitavissa mihin tahansa ESP32-projektiin!

---

## 🎯 Ominaisuudet

✅ **Ei LoRa-riippuvuuksia** - Vain Serial-yhteys!
✅ **2 johtoa** - TX→RX ja GND→GND
✅ **Landscape-näyttö** - 320x240 vaakataso
✅ **Helppokäyttöinen API** - `display.set("LED", "ON")`
✅ **Automaattinen päivitys** - Näyttö päivittyy välittömästi
✅ **Joustava protokolla** - CSV-muotoinen data
✅ **Useita esimerkkejä** - Copy-paste valmis!
✅ **Värikoodattu UI** - Vihreä/Oranssi/Punainen

---

## 📦 Laitteisto

**Display-laite (ESP32-2432S022):**
- ESP32-WROOM-32
- 2.4" ST7789 TFT (240x320 pikseliä)
- 8-bit parallel interface
- Integroitu - ei vaadi kytkentöjä!

**Pää-ESP32:**
- Mikä tahansa ESP32 (ESP32, ESP32-S2, ESP32-C3, jne.)
- 1× vapaa GPIO (TX)

---

## 🔌 Kytkentä

### VAIN 2 JOHTOA!

```
┌─────────────────┐           ┌──────────────────┐
│   Pää-ESP32     │           │  Display-ESP32   │
│                 │           │  (2432S022)      │
│                 │           │                  │
│  GPIO 17 (TX) ──┼──────────►│─ GPIO 18 (RX)   │
│  GND          ──┼───────────│─ GND             │
│                 │           │                  │
└─────────────────┘           └──────────────────┘
```

**Huom:** GPIO 17 voi olla mikä tahansa vapaa GPIO päälaitteessa!

---

## 🚀 Pikaohje (5 minuuttia käyttöön!)

### Vaihe 1: Lataa display-laite

```bash
# Arduino IDE
File → Open → Roboter_Display_TFT/Roboter_Display_TFT.ino
Tools → Board → ESP32 Dev Module
Tools → Port → (valitse ESP32-2432S022)
Tools → Upload
```

### Vaihe 2: Kopioi kirjasto päälaitteeseen

Kopioi `DisplayClient.h` oman projektisi kansioon:
```
MinunProjekti/
├── MinunProjekti.ino
└── DisplayClient.h       ← Kopioi tämä!
```

### Vaihe 3: Lisää koodiin

```cpp
#include "DisplayClient.h"

DisplayClient display(17);  // TX pin 17

void setup() {
  Serial.begin(115200);
  display.begin();  // ← Lisää tämä
}

void loop() {
  display.set("LED", "ON");
  display.set("Temp", 42);
  display.send();

  delay(1000);
}
```

### Vaihe 4: Lataa ja testaa!

Kytke johdot, lataa koodi, ja näyttö alkaa päivittyä! 🎉

---

## 📖 API-dokumentaatio

### Perustoiminnot

```cpp
DisplayClient display(17);  // Luo client (TX pin)

display.begin();            // Alusta yhteys

display.set("key", "value"); // Lisää kenttä
display.send();              // Lähetä kaikki

display.update("key", 42);   // Päivitä yksi kenttä heti

display.alert("Fire!");      // Näytä hälytys
display.clearAlert();        // Poista hälytys

display.clearDisplay();      // Tyhjennä kaikki
```

### Esimerkkejä

#### Esimerkki 1: Yksinkertaisin

```cpp
void loop() {
  display.update("Counter", counter++);
  delay(1000);
}
```

#### Esimerkki 2: Useampi kenttä

```cpp
void loop() {
  display.clear();
  display.set("Temp", 22.5);
  display.set("Humidity", 65);
  display.set("Status", "OK");
  display.send();

  delay(2000);
}
```

#### Esimerkki 3: Hälytys

```cpp
if (temperature > 30.0) {
  display.alert("High temperature!");
} else {
  display.clearAlert();
}
```

#### Esimerkki 4: Monipuolinen

```cpp
display.clear();
display.set("LED", digitalRead(LED_PIN) ? "ON" : "OFF");
display.set("Temp", String(temp, 1) + "C");
display.set("RSSI", String(rssi) + " dBm");
display.set("Uptime", String(millis()/1000) + "s");
display.send();
```

---

## 🔧 Konfigurointi

### Vaihda TX pin

```cpp
DisplayClient display(25);  // Käytä GPIO 25
```

### Vaihda baudrate

```cpp
DisplayClient display(17, -1, 9600);  // 9600 baud
```

### Full-duplex (TX + RX)

```cpp
DisplayClient display(17, 16);  // TX=17, RX=16
```

### Vaihda näytön kirkkautta

Display-laitteessa (`Universal_Display_TFT.ino`):
```cpp
#define BACKLIGHT_BRIGHTNESS 200  // 0-255
```

---

## 📊 Protokolla

### Formaatti

CSV-muotoinen data:
```
KEY:VALUE,KEY2:VALUE2,KEY3:VALUE3,...
```

### Esimerkkejä

```
LED:ON,Temp:22.5,Status:OK
SEQ:123,LED:1,TOUCH:0,RSSI:-78
Counter:42,Voltage:3.85,Heap:245
```

### Erikoiskomennot

| Komento | Kuvaus | Esimerkki |
|---------|--------|-----------|
| `ALERT:message` | Näytä hälytys | `ALERT:Fire detected!` |
| `CLEAR_ALERT` | Poista hälytys | `CLEAR_ALERT` |
| `CLEAR` | Tyhjennä kaikki | `CLEAR` |

### Rajoitukset

- Maksimi 20 kenttää
- Maksimi 256 merkkiä per viesti
- Kenttänimet max ~20 merkkiä
- Arvot max ~30 merkkiä (pidempi teksti katkeaa)

---

## 💡 Esimerkkiprojektit

### 1. Perus LED-blinkkaus

```cpp
#include "DisplayClient.h"
DisplayClient display(17);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  display.begin();
}

void loop() {
  bool led = !digitalRead(LED_PIN);
  digitalWrite(LED_PIN, led);

  display.update("LED", led ? "ON" : "OFF");
  delay(500);
}
```

### 2. DHT22 Lämpötila/Kosteus

```cpp
#include "DisplayClient.h"
#include "DHT.h"

DisplayClient display(17);
DHT dht(DHT_PIN, DHT22);

void setup() {
  dht.begin();
  display.begin();
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  display.clear();
  display.set("Temp", String(t, 1) + "C");
  display.set("Humidity", String(h, 0) + "%");
  display.send();

  if (t > 30) display.alert("Too hot!");
  else display.clearAlert();

  delay(2000);
}
```

### 3. WiFi Signaalin voimakkuus

```cpp
#include "DisplayClient.h"
#include <WiFi.h>

DisplayClient display(17);

void setup() {
  WiFi.begin("SSID", "password");
  display.begin();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();

    display.clear();
    display.set("WiFi", "Connected");
    display.set("RSSI", String(rssi) + " dBm");
    display.set("IP", WiFi.localIP().toString());
    display.send();
  } else {
    display.alert("WiFi disconnected!");
  }

  delay(1000);
}
```

### 4. Akun jännite

```cpp
#include "DisplayClient.h"

DisplayClient display(17);
#define BAT_PIN 35

void setup() {
  pinMode(BAT_PIN, INPUT);
  display.begin();
}

void loop() {
  int raw = analogRead(BAT_PIN);
  float voltage = (raw / 4095.0) * 3.3 * 2.0;  // Voltage divider

  int percent = map(voltage * 100, 300, 420, 0, 100);
  percent = constrain(percent, 0, 100);

  display.clear();
  display.set("Battery", String(voltage, 2) + "V");
  display.set("Level", String(percent) + "%");
  display.send();

  if (percent < 20) {
    display.alert("Low battery!");
  }

  delay(5000);
}
```

### 5. LoRa RSSI/SNR (Roboter 9)

```cpp
#include "DisplayClient.h"

DisplayClient display(17);

void loop() {
  // ... LoRa-vastaanotto ...

  if (receiveLoRaMessage(remote, payload)) {
    display.clear();
    display.set("SEQ", remote.sequenceNumber);
    display.set("RSSI", String(remote.rssi) + " dBm");
    display.set("SNR", String(remote.snr) + " dB");
    display.set("Packets", remote.messageCount);
    display.send();
  }

  // ...
}
```

---

## 🎨 Customointi

### Muuta värejä

`Universal_Display_TFT.ino`:ssa:

```cpp
#define COLOR_BG 0x0000          // Tausta (musta)
#define COLOR_HEADER 0x001F      // Header (sininen)
#define COLOR_TEXT 0xFFFF        // Teksti (valkoinen)
#define COLOR_GOOD 0x07E0        // Hyvä (vihreä)
#define COLOR_WARN 0xFD20        // Varoitus (oranssi)
#define COLOR_BAD 0xF800         // Huono (punainen)
#define COLOR_ALERT 0xFFE0       // Hälytys (keltainen)
```

RGB565 värikoodit:
- Punainen: `0xF800`
- Vihreä: `0x07E0`
- Sininen: `0x001F`
- Valkoinen: `0xFFFF`
- Musta: `0x0000`
- Keltainen: `0xFFE0`
- Syaani: `0x07FF`
- Magenta: `0xF81F`

### Muuta fonttikokoa

```cpp
tft.setTextSize(2);  // 1=pieni, 2=normaali, 3=iso
```

### Muuta layoutia

```cpp
#define HEADER_H 40      // Header korkeus
#define DATA_H 220       // Data-osion korkeus
#define ALERT_H 40       // Alert-osion korkeus
```

---

## 🐛 Vianetsintä

### Näyttö ei reagoi

1. **Tarkista johdot:**
   - TX (päälaite) → RX (näyttö)
   - GND → GND
   - Johdot kunnossa?

2. **Tarkista baudrate:**
   - Molemmissa 115200?
   - `display.begin()` kutsuttu?

3. **Tarkista GPIO:**
   - TX pin oikein?
   - Pin vapaa (ei käytössä muualla)?

4. **Serial Monitor:**
   - Avaa päälaitteen Serial Monitor
   - Näkyykö "→ Display: ..." viestit?

### Teksti ei päivity

1. **Kutsu `send()`:**
   ```cpp
   display.set("LED", "ON");
   display.send();  // ← Tärkeä!
   ```

2. **Tarkista kentän nimi:**
   - Sama nimi ylikirjoittaa vanhan
   - Eri nimi luo uuden kentän

3. **Liian pitkä teksti:**
   - Max 30 merkkiä per arvo
   - Katkeaa automaattisesti

### "NO SIGNAL" näytössä

1. **Ei dataa 5 sekuntiin:**
   - Lähetätkö tarpeeksi usein?
   - `delay()` liian pitkä?

2. **Väärä RX pin:**
   - Näytössä GPIO 18
   - Tarkista `UART_RX_PIN`

### Näyttö flikkaa

1. **Päivitä harvemmin:**
   ```cpp
   delay(500);  // Vähintään 100ms välein
   ```

2. **Lähetä vain kun arvo muuttuu:**
   ```cpp
   static int lastValue = 0;
   if (value != lastValue) {
     display.update("Val", value);
     lastValue = value;
   }
   ```

---

## 📐 Tekniset tiedot

### Display-laite (ESP32-2432S022)

| Komponentti | Tiedot |
|-------------|--------|
| MCU | ESP32-WROOM-32 |
| Display | ST7789 2.4" 240x320 |
| Interface | 8-bit Parallel (MCU8080) |
| Touch | CST820 (I2C) - ei käytössä |
| UART | RX=GPIO18, TX=GPIO19 |
| Baudrate | 115200 (muutettavissa) |
| Virta | USB-C, min 500mA |

### Pin-varaukset

**Display (integroitu):**
- TFT Data: GPIO 12,13,14,15,25,27,32,33
- TFT Control: GPIO 2,4,16,17
- Touch I2C: GPIO 21,22 (valinnainen)
- Backlight: GPIO 0

**Vapaana:**
- GPIO 18 (UART RX) ← Käytetään!
- GPIO 19 (UART TX) - valinnainen
- GPIO 5,23,26,34-39

### Suorituskyky

- Päivitysnopeus: 10 Hz (100ms)
- Viive: <50ms
- Max kenttiä: 20
- Max viestipituus: 256 merkkiä
- RAM-käyttö: ~2KB

---

## 🔄 Päivityshistoria

### v2.0 (2025-01-05)
- ✅ Poistettu LoRa-riippuvuus
- ✅ UART-pohjainen yhteys
- ✅ DisplayClient-kirjasto
- ✅ Esimerkkiprojektit
- ✅ Kattava dokumentaatio

### v1.0 (2025-01-05)
- ✅ Alkuperäinen LoRa-versio

---

## 💾 Tiedostot

```
Roboter_Display_TFT/
├── Universal_Display_TFT.ino    Display-laitteen koodi
├── display_config.h             TFT-konfiguraatio
├── DisplayClient.h              Päälaitteen kirjasto
├── README_UNIVERSAL.md          Tämä dokumentti
└── examples/
    ├── Example_Basic/           Perusesimerkki
    ├── Example_Sensor/          Sensoriesimerkki
    └── Example_Roboter9/        Roboter 9 integraatio
```

---

## 🤝 Integrointi olemassa olevaan projektiin

### Roboter Gruppe 9

1. Kopioi `DisplayClient.h` → `Roboter_Gruppe_9/`

2. Lisää `Roboter_Gruppe_9.ino`:hon:
```cpp
#include "DisplayClient.h"
DisplayClient display(17);
```

3. Lisää `setup()`:iin:
```cpp
display.begin();
```

4. Lisää `loop()`:iin (sender):
```cpp
static unsigned long lastDisplay = 0;
if (millis() - lastDisplay >= 2000) {
  lastDisplay = millis();

  display.clear();
  display.set("SEQ", local.sequenceNumber);
  display.set("LED", local.ledState ? "ON" : "OFF");
  display.set("RSSI", String(remote.rssi) + " dBm");
  display.send();
}
```

### Muu projekti

1. Kopioi `DisplayClient.h` projektiisi
2. Include ja luo client
3. Kutsu `begin()` setup:issa
4. Lähetä dataa `set()` + `send()`

---

## 🎓 Oppimateriaali

### Video-tutoriaalit (tulossa)

- Peruskytkentä ja testaus
- Sensoridatan näyttäminen
- Roboter 9 integraatio
- Custom UI-suunnittelu

### Linkit

- [LovyanGFX kirjasto](https://github.com/lovyan03/LovyanGFX)
- [ESP32 UART dokumentaatio](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)
- [ESP32-2432S022 datasheet](https://github.com/makerfabs/ESP32-2432S022)

---

## 📞 Tuki

**Ongelma?**
1. Lue vianetsintä-osio
2. Tarkista esimerkit
3. Testaa Basic-esimerkki ensin
4. Tarkista Serial Monitor

**Vinkkejä:**
- Aloita yksinkertaisesta
- Testaa yksi asia kerrallaan
- Käytä Serial.println() debuggaukseen
- Tarkista johdot multimittarilla

---

**Valmis käyttöön!** 🚀

Kokeile ensin `Example_Basic` ja laajenna siitä eteenpäin.

Onnea projektiin! 🎉
