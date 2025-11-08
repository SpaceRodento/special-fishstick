# LoRa-robotti - Ryhmä 9

ESP32-pohjainen robottijärjestelmä LoRa-viestinnällä ja TFT-näytöllä.

## Projektin osat

### Display_Device
Näyttölaite ESP32-2432S022 -laitteelle (TFT-näyttö + kosketuspaneeli).
- Vastaanottaa dataa UART-väylän kautta (GPIO 18)
- Näyttää tiedot värillisellä TFT-näytöllä (320x240)
- Tukee yksinkertaista CSV-muotoista dataa (KEY:VALUE parit)

### Robot_Sender
Lähettäjälaite mille tahansa ESP32:lle.
- Lähettää dataa UART-väylän kautta (GPIO 23)
- Demototeutus - korvaa omalla robottilogiikalla
- Toimii esimerkkinä Display_Device -integraatioon

## Pika-aloitus

### 1. Asenna kirjastot
Display_Device vaatii LovyanGFX-kirjaston:
```
Arduino IDE → Library Manager → "LovyanGFX" → Install
```

Robot_Sender ei vaadi lisäkirjastoja.

### 2. Lataa koodi laitteille
```
Display_Device/Display_Device.ino → ESP32-2432S022
Robot_Sender/Robot_Sender.ino    → Robot ESP32
```

### 3. Kytkennät
```
Robot GPIO 23 (TX) ─────► Display GPIO 18 (RX)
Robot GND          ─────  Display GND
```

## Dokumentaatio

**Yksityiskohtainen dokumentaatio löytyy:**
- `Roboter_Gruppe_9/QUICK_START.md` - Pika-aloitus täydellä robotilla
- `Roboter_Gruppe_9/TESTING_CHECKLIST.md` - Testausohje
- `Roboter_Display_TFT/README.md` - Näyttökirjaston dokumentaatio
- `LIBRARIES.txt` - Tarvittavat kirjastot

## Projektirakenne

```
.
├── README.md                    ← Tämä tiedosto
├── Display_Device/              ← Näyttölaitteen koodi
│   └── Display_Device.ino
├── Robot_Sender/                ← Lähetin-esimerkkikoodi
│   └── Robot_Sender.ino
├── Roboter_Gruppe_9/            ← Täydellinen robottiprojekti
│   ├── Display_Test_2in/        ← Ohjain 2" TFT-näytölle
│   └── Robot_Control/           ← Robotin ohjauslogiikka
├── Roboter_Display_TFT/         ← Näyttökirjasto (valinnaisesti)
│   ├── DisplayClient.h          ← Helppo integraatio
│   └── DisplayServer.h
└── LIBRARIES.txt                ← Asennusohjeet kirjastoille
```

## Tekniset tiedot

**Laitteet:**
- Display: ESP32-2432S022 (320x240 TFT + kosketuspaneeli)
- Robot: Mikä tahansa ESP32

**Kommunikaatio:**
- UART 115200 baud
- Yksinkertainen CSV-muoto: `KEY1:VAL1,KEY2:VAL2\n`
- 2 johtoa: TX→RX + GND

**Riippuvuudet:**
- LovyanGFX (vain Display_Device)
- Arduino ESP32 Core

## Integrointi omaan projektiin

### Vaihtoehto 1: Suora UART (yksinkertaisin)
Katso `Robot_Sender.ino` esimerkki - lähetä dataa suoraan UART:lla.

### Vaihtoehto 2: DisplayClient-kirjasto (suositeltu)
Katso `Roboter_Display_TFT/README.md` - helpompi integraatio.

```cpp
#include "DisplayClient.h"
DisplayClient display(23);  // TX pin

void setup() {
  display.begin();
}

void loop() {
  display.set("TEMP", temperature);
  display.set("SPEED", motorSpeed);
  display.send();
}
```

## Lisenssi

Koulutyö - Ryhmä 9

---

**Tarvitsetko apua?** Lue ensin `Roboter_Gruppe_9/QUICK_START.md` - siellä on yksityiskohtaiset ohjeet.
