# RYLR896 LoRa - ESP32 Automaattinen Lähetin/Vastaanotin

Hyvin yksinkertainen ja toimintavarma koodi ESP32 + RYLR896 LoRa-moduulille.

## ✨ Pääominaisuus

**KOODI ON IDENTTINEN MOLEMMISSA LAITTEISSA!**

Rooli (lähettäjä/vastaanottaja) määräytyy automaattisesti hyppylangalla:
- **GPIO 4 -> GND** = LÄHETTÄJÄ
- **GPIO 4 -> IRTI** = VASTAANOTTAJA

## 🚀 Ominaisuudet

- **Ei tarvitse muuttaa koodia** laitteiden välillä
- Rooli vaihtuu fyysisellä hyppylangalla (GPIO 4)
- Hyvin kommentoitu suomeksi
- Ei ylimääräisiä kirjastoja
- Toimintavarma ja yksinkertainen
- Automaattinen osoitteen ja kohteen määritys

## 📁 Tiedostot

- `RYLR896_simple.ino` - Pääohjelma (Arduino/PlatformIO)
- `KAYTTOOHJE.md` - Yksityiskohtaiset ohjeet
- `platformio.ini` - PlatformIO-konfiguraatio

## ⚡ Pika-aloitus

### Kytkennät:
```
RYLR896 -> ESP32
-----------------
TX  -> GPIO 25
RX  -> GPIO 26
VCC -> 3.3V
GND -> GND

Roolin määritys:
GPIO 4 -> GND  = Lähettäjä
GPIO 4 -> IRTI = Vastaanottaja
```

### Käyttö:
1. Lataa **SAMA** koodi molempiin ESP32:iin
2. Ensimmäisessä: Kytke GPIO 4 -> GND (lähettäjä)
3. Toisessa: Jätä GPIO 4 irti (vastaanottaja)
4. Avaa Serial Monitor (115200 baud) molemmista

Katso `KAYTTOOHJE.md` yksityiskohtaiset ohjeet!