# Roboter Display TFT - LoRa Display Station

**Erillinen TFT-näyttölaite** ESP32-2432S022:lle joka näyttää reaaliaikaista dataa pää-ESP32:lta LoRa-verkon yli.

---

## 📦 Laitteisto

**ESP32-2432S022:**
- ESP32-WROOM-32
- 2.4" ST7789 TFT (240x320 pikseliä)
- 8-bit parallel interface
- CST820 kosketusnäyttö (I2C) - ei käytössä tässä projektissa
- USB-C virtalähde

**RYLR896 LoRa-moduuli:**
- 868 MHz LoRa transceiver
- UART-käyttöliittymä

---

## 🔌 Kytkennät

### LoRa-moduuli → ESP32-2432S022

```
RYLR896         ESP32-2432S022
────────────────────────────────
VCC       →     3.3V
GND       →     GND
TX        →     GPIO 18 (RX1)
RX        →     GPIO 26 (TX1)
```

### TFT-näyttö (jo valmiiksi kytketty)

Näyttö on integroitu ESP32-2432S022 board:iin - ei tarvitse kytkentöjä!

---

## 📚 Kirjastot

### Arduino IDE:

1. **LovyanGFX** (TFT-grafiikka)
   ```
   Tools → Manage Libraries
   Etsi: "LovyanGFX"
   Asenna: "LovyanGFX" by lovyan03
   ```

2. **ESP32 Board Support:**
   ```
   File → Preferences → Additional Boards Manager URLs:
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

   Tools → Board → Boards Manager
   Etsi: "esp32"
   Asenna: "esp32" by Espressif Systems
   ```

### Vaihtoehtoinen tapa:

Käytä mukana tullutta `Libraries/LovyanGFX-master` kansiota:
1. Kopioi kansio: `Example_For_2in_Screen/Libraries/LovyanGFX-master`
2. Arduino Libraries -kansioon: `~/Documents/Arduino/libraries/`

---

## ⚙️ Konfigurointi

### 1. LoRa-osoitteet

Päälaitteen ja näyttölaitteen LoRa-osoitteet:

| Laite | Osoite | Rooli |
|-------|--------|-------|
| Pää-ESP32 (sender) | 2 | Lähettää dataa |
| Display-ESP32 | 3 | Vastaanottaa ja näyttää |
| Receiver-ESP32 | 1 | Vastaanottaa |

**Tärkeää:** Network ID:n pitää olla sama (6) kaikissa laitteissa!

### 2. Pää-ESP32 muutokset

Lisää `Roboter_Gruppe_9.ino`:hon funktio joka lähettää dataa displaylle:

```cpp
void sendDisplayUpdate() {
  String payload = "SEQ:" + String(sequence) +
                   ",LED:" + String(digitalRead(LED_PIN)) +
                   ",TOUCH:" + String(touchDetected ? 1 : 0);

  // Jos extended telemetry käytössä:
  #if ENABLE_EXTENDED_TELEMETRY
    payload += ",UP:" + String(millis() / 1000);
    payload += ",HEAP:" + String(ESP.getFreeHeap() / 1024);
    // jne...
  #endif

  // Lähetä displaylle (address 3)
  sendLoRaMessage(payload, 3);
}
```

Kutsu `sendDisplayUpdate()` esim. 2 sekunnin välein.

### 3. Näytön kirkkaus

Säädä `Roboter_Display_TFT.ino`:ssa:

```cpp
#define BACKLIGHT_BRIGHTNESS 200  // 0-255 (0 = pimeä, 255 = kirkkain)
```

---

## 🖥️ Näytön Layout

```
╔════════════════════════════════╗
║       ROBOTER 9                ║ ← Header (sininen)
║       CONNECTED    2h15m       ║
╠════════════════════════════════╣
║ STATUS:                        ║
║   Seq: 1234                    ║
║   LED: ON                      ║
║   Touch: NO                    ║
║   RSSI: -78 dBm                ║
║   Battery: 3.85V (85%)         ║
╠════════════════════════════════╣
║ TELEMETRY:                     ║
║   Uptime: 2h15m                ║
║   Heap: 245 KB                 ║
║   Temp: 42 C                   ║
║   Loop: 450 Hz                 ║
╠════════════════════════════════╣
║ ALERTS:                        ║
║   No alerts                    ║ ← Muuttuu punaiseksi jos hälytys!
╠════════════════════════════════╣
║ Packets: 1234                  ║ ← Footer
╚════════════════════════════════╝
```

---

## 🚀 Käyttö

### 1. Lataa koodi ESP32-2432S022:een

Arduino IDE:
```
Tools → Board → ESP32 Arduino → ESP32 Dev Module
Tools → Upload Speed → 115200
Tools → Port → (valitse oikea portti)
Sketch → Upload
```

### 2. Avaa Serial Monitor

```
Tools → Serial Monitor
Baud rate: 115200
```

Pitäisi näkyä:
```
╔════════════════════════════════════════╗
║  ROBOTER GRUPPE 9 - DISPLAY STATION   ║
║  ESP32-2432S022 + LoRa Display        ║
╚════════════════════════════════════════╝

📺 Initializing TFT display...
📡 Initializing LoRa...
  Configuring RYLR896...
  +OK
✓ LoRa initialized
  Address: 3
  Network ID: 6
✓ Display station ready!

Waiting for LoRa data...
```

### 3. Käynnistä pää-ESP32

Kun pää-ESP32 alkaa lähettää, näytössä näkyy:
```
📡 RX: SEQ:1,LED:1,TOUCH:0 [RSSI:-78 SNR:10]
📡 RX: SEQ:2,LED:0,TOUCH:0 [RSSI:-79 SNR:10]
...
```

TFT-näyttö päivittyy automaattisesti!

---

## 🐛 Vianetsintä

### Näyttö ei käynnisty
- Tarkista USB-C virtalähde (tarvitaan vähintään 500mA)
- Tarkista että kirjastot on asennettu oikein
- Tarkista board-valinta (ESP32 Dev Module)

### LoRa ei yhdistä
- Tarkista kytkennät (TX ↔ RX ristiin!)
- Tarkista osoitteet (Display = 3, Sender = 2)
- Tarkista Network ID (6 molemmissa)
- Tarkista LoRa-moduulin virtalähde (3.3V!)

### Näyttö on liian kirkas/tumma
- Säädä `BACKLIGHT_BRIGHTNESS` (0-255)

### "NO SIGNAL" näkyy näytössä
- Pää-ESP32 ei lähetä
- Väärät LoRa-osoitteet
- LoRa-moduuli ei vastaa (tarkista kytkennät)

### Näyttö päivittyy hitaasti
- Normaalia! Päivitysväli on 500ms
- Voit muuttaa: `#define DISPLAY_UPDATE_INTERVAL 500`

---

## 📊 Datan Formaatti

Pää-ESP32 lähettää CSV-muotoista dataa:

**Perus payload:**
```
SEQ:123,LED:1,TOUCH:0
```

**Extended telemetry:**
```
SEQ:123,LED:1,TOUCH:0,UP:3600,HEAP:245,TEMP:42,LOOP:450
```

**Akku:**
```
SEQ:123,LED:1,TOUCH:0,BAT:3.85
```

**Hälytykset:**
```
ALERT:FIRE_AUDIO,RMS:450,PEAKS:3
ALERT:FIRE_LIGHT,RED:255,FLASHES:5
```

Display parsii automaattisesti kaikki kentät!

---

## ✅ Testaus

### 1. Testaa TFT ilman LoRaa

Kommentoi pois LoRa-initialisointi ja aseta test-dataa:
```cpp
void setup() {
  // ...
  // initLoRa();  // Kommentoi pois

  // Test data
  displayData.sequence = 1234;
  displayData.ledState = true;
  displayData.rssi = -78;
  displayData.batteryVoltage = 3.85;
  displayData.loraConnected = true;
}
```

### 2. Testaa LoRa ilman pää-ESP:tä

Lähetä manuaalisesti Serial Monitorista pää-ESP:ltä:
```
AT+SEND=3,15,SEQ:1,LED:1
```

Displayn pitäisi vastaanottaa viesti!

---

## 🔧 Customointi

### Muuta värejä

`Roboter_Display_TFT.ino`:ssa:
```cpp
#define COLOR_BG 0x0000          // Tausta (musta)
#define COLOR_HEADER 0x001F      // Header (sininen)
#define COLOR_TEXT 0xFFFF        // Teksti (valkoinen)
#define COLOR_GOOD 0x07E0        // Hyvä (vihreä)
#define COLOR_WARN 0xFD20        // Varoitus (oranssi)
#define COLOR_BAD 0xF800         // Huono (punainen)
```

Värikoodi: RGB565 (16-bit)
- R: 5 bittiä
- G: 6 bittiä
- B: 5 bittiä

### Muuta layout:ia

Muuta region-korkeuksia:
```cpp
#define HEADER_H 40      // Header korkeus
#define STATUS_H 100     // Status-osion korkeus
#define TELEMETRY_H 100  // Telemetry-osion korkeus
#define ALERTS_H 60      // Alert-osion korkeus
```

### Lisää uusi data-kenttä

1. Lisää `DisplayData` struct:iin:
```cpp
struct DisplayData {
  // ...
  int myNewField;
};
```

2. Parsenna `parseLoRaMessage()`:ssa:
```cpp
int myIdx = message.indexOf("MYNEW:");
if (myIdx >= 0) {
  displayData.myNewField = message.substring(myIdx + 6, ...).toInt();
}
```

3. Näytä `drawStatus()` tai `drawTelemetry()`:ssä:
```cpp
String myStr = "My Field: " + String(displayData.myNewField);
tft.drawString(myStr, 20, y);
```

---

## 🎯 Seuraavat Askeleet

### Vaihtoehto 1: Kosketusnäyttö

Lisää CST820 touch-tuki (ei vielä toteutettu):
- Kalibroi/nollaa painike
- Hiljennä hälytys -painike
- Vaihda näkymää (status/graph/history)

### Vaihtoehto 2: Graafinen käyrä

Lisää RSSI/Battery history-käyrä:
- Tallennetaan viimeiset 100 arvoa
- Piirretään line chart
- Zoom-toiminto

### Vaihtoehto 3: Useampi sivu

Lisää sivunavigaatio:
- Sivu 1: Status
- Sivu 2: Telemetry
- Sivu 3: Alerts
- Sivu 4: Statistics

---

## 📄 Tiedostot

- `Roboter_Display_TFT.ino` - Pääohjelma (display + LoRa)
- `display_config.h` - ESP32-2432S022 TFT-konfiguraatio
- `README.md` - Tämä dokumentti

---

**Valmis käyttöön! 🚀**

Jos ongelmia, tarkista:
1. Kirjastot asennettu
2. LoRa-kytkennät oikein (TX ↔ RX ristiin!)
3. LoRa-osoitteet oikein (Display = 3)
4. Network ID sama (6)
5. Pää-ESP32 lähettää osoitteeseen 3
