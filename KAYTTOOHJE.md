# RYLR896 LoRa - Automaattinen Lähetin/Vastaanotin

## 🎯 Tarkoitus
Hyvin yksinkertainen koodi, jolla testataan että viesti kulkee ESP32:n ja RYLR896 LoRa-moduulin kautta toiselta laitteelta toiselle.

**✨ ERIKOISUUS: KOODI ON IDENTTINEN MOLEMMISSA LAITTEISSA!**
Rooli (lähettäjä/vastaanottaja) määräytyy automaattisesti hyppylangalla.

## 🔌 Kytkennät

### LoRa-moduuli:
```
RYLR896    ->   ESP32
--------------------------
TX         ->   GPIO 25 (RX)
RX         ->   GPIO 26 (TX)
VCC        ->   3.3V
GND        ->   GND
```

### Roolin määritys (HYPPYLANKA):
```
GPIO 4 -> GND        = LÄHETTÄJÄ (osoite 1)
GPIO 4 -> IRTI       = VASTAANOTTAJA (osoite 2)
```

## ⚙️ Käyttöönotto

### 1. ENSIMMÄINEN LAITE (Lähettäjä)

1. Kytke RYLR896 moduuli ESP32:een
2. **Kytke GPIO 4 -> GND hyppylangalla**
3. Lataa `RYLR896_simple.ino` ESP32:lle
4. Avaa Serial Monitor (115200 baud)

Laite tunnistaa automaattisesti:
- Rooli: LÄHETTÄJÄ
- Osoite: 1
- Kohde: 2

### 2. TOINEN LAITE (Vastaanottaja)

1. Kytke RYLR896 moduuli ESP32:een
2. **Jätä GPIO 4 irti (ei hyppylankaa)**
3. Lataa **SAMA** `RYLR896_simple.ino` ESP32:lle
4. Avaa Serial Monitor (115200 baud)

Laite tunnistaa automaattisesti:
- Rooli: VASTAANOTTAJA
- Osoite: 2
- Kuuntelee: Lähettäjältä 1

## 📡 Mitä pitäisi tapahtua

**LÄHETTÄJÄ näyttää:**
```
=================================
RYLR896 LoRa - Automaattinen
=================================
GPIO 4 tila: LOW (GND)
---------------------------------
ROOLI: LÄHETTÄJÄ
Toiminta: Lähettää viestejä 5s välein
Oma osoite: 1
Kohde: 2
=================================

--- LÄHETETÄÄN VIESTI ---
Viesti #1: Testi 1
LoRa: +OK
✓ Lähetys OK!
------------------------
```

**VASTAANOTTAJA näyttää:**
```
=================================
RYLR896 LoRa - Automaattinen
=================================
GPIO 4 tila: HIGH (irti)
---------------------------------
ROOLI: VASTAANOTTAJA
Toiminta: Kuuntelee viestejä
Oma osoite: 2
=================================

=== VIESTI VASTAANOTETTU ===
+RCV=1,6,Testi 1,-45,10
Lähettäjä: 1
Viesti: Testi 1
Signaali (RSSI): -45 dBm
SNR: 10
============================
```

## 🔧 Roolin vaihto

**EI TARVITSE MUUTTAA KOODIA!**

Rooli vaihtuu yksinkertaisesti:
- **GPIO 4 -> GND** = Lähettäjä
- **GPIO 4 -> IRTI** = Vastaanottaja

Koodi on identtinen molemmissa laitteissa!

## 📝 Asetukset

- **ROLE_PIN (GPIO 4)**: Määrittää roolin automaattisesti
- **LORA_NETWORK (6)**: Verkko-ID, pidä sama molemmissa
- **LORA_ADDRESS**: Määräytyy automaattisesti (1=lähettäjä, 2=vastaanottaja)
- **TARGET_ADDRESS**: Määräytyy automaattisesti vastakkaiseksi

## 🐛 Vianetsintä

1. **Väärä rooli tunnistetaan**
   - Tarkista GPIO 4 kytkentä
   - Lähettäjä: GPIO 4 pitää olla kytketty GND:hen
   - Vastaanottaja: GPIO 4 pitää olla irti (ei kytkettynä mihinkään)
   - Katso Serial Monitor, se näyttää GPIO 4 tilan käynnistyksen yhteydessä

2. **Ei yhteyttä RYLR896:een**
   - Tarkista kytkennät (TX/RX oikein päin!)
   - Tarkista 3.3V jännite
   - Tarkista BAUD rate (115200)

3. **Ei vastaanota viestejä**
   - Varmista että molemmat laitteet käyttävät samaa koodia
   - Tarkista että toisen GPIO 4 on GND:ssä ja toisen irti
   - Tarkista Serial Monitor molemmista - näkyykö oikeat roolit?

4. **Heikko signaali**
   - RSSI pitäisi olla -120 ja 0 välillä (lähempänä 0 = parempi)
   - Siirrä laitteita lähemmäksi
   - Varmista että antennit on kunnolla kiinni

## 📊 LoRa parametrit

Koodi käyttää maksimi kantamaan optimoituja asetuksia:
- **Spreading Factor**: 12 (hitain, pisin kantama)
- **Bandwidth**: 7 (125 kHz)
- **Coding Rate**: 1 (4/5)
- **Preamble**: 4

Jos haluat nopeamman mutta lyhyemmän kantaman, muuta:
```cpp
lahetaKomento("AT+PARAMETER=7,7,1,4");  // Nopea, lyhyt kantama
```

## ✅ Testattu

- ESP32 DevKit v1
- RYLR896 LoRa module
- Arduino IDE 2.x
- ESP32 board package 2.x
