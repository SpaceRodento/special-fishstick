# RYLR896 LoRa - ESP32 Yksinkertainen Testi

Hyvin yksinkertainen ja toimintavarma koodi ESP32 + RYLR896 LoRa-moduulille.

## 🚀 Ominaisuudet

- **Yksi muuttuja** vaihtaa lähettäjän ja vastaanottajan roolin
- Hyvin kommentoitu suomeksi
- Ei ylimääräisiä kirjastoja
- Toimintavarma ja yksinkertainen

## 📁 Tiedostot

- `RYLR896_simple.ino` - Pääohjelma (Arduino/PlatformIO)
- `KAYTTOOHJE.md` - Yksityiskohtaiset ohjeet
- `platformio.ini` - PlatformIO-konfiguraatio

## ⚡ Pika-aloitus

1. Kytke RYLR896 ESP32:een (TX->GPIO25, RX->GPIO26)
2. Avaa `RYLR896_simple.ino`
3. Vaihda `LAHETYS_TILA` (true=lähettäjä, false=vastaanottaja)
4. Lataa koodi ESP32:een
5. Avaa Serial Monitor (115200 baud)

Katso `KAYTTOOHJE.md` yksityiskohtaiset ohjeet!