# RYLR896 LoRa - Yksinkertainen Testi

## 🎯 Tarkoitus
Hyvin yksinkertainen koodi, jolla testataan että viesti kulkee ESP32:n ja RYLR896 LoRa-moduulin kautta toiselta laitteelta toiselle.

## 🔌 Kytkennät

```
RYLR896    ->   ESP32
--------------------------
TX         ->   GPIO 25 (RX)
RX         ->   GPIO 26 (TX)
VCC        ->   3.3V
GND        ->   GND
```

## ⚙️ Käyttöönotto

### 1. ENSIMMÄINEN LAITE (Lähettäjä)

Avaa `RYLR896_simple.ino` ja aseta:

```cpp
bool LAHETYS_TILA = true;   // LÄHETTÄJÄ
#define LORA_ADDRESS 1      // Tämän laitteen osoite
#define TARGET_ADDRESS 2    // Kohde (vastaanottaja)
```

### 2. TOINEN LAITE (Vastaanottaja)

Avaa `RYLR896_simple.ino` ja aseta:

```cpp
bool LAHETYS_TILA = false;  // VASTAANOTTAJA
#define LORA_ADDRESS 2      // Tämän laitteen osoite
#define TARGET_ADDRESS 1    // (ei käytetä vastaanottajassa)
```

### 3. Lataa koodi molempiin ESP32:iin

### 4. Avaa Serial Monitor (115200 baud)

## 📡 Mitä pitäisi tapahtua

**LÄHETTÄJÄ näyttää:**
```
=================================
RYLR896 LoRa Testi
TILA: LÄHETTÄJÄ
Lähettää viestejä 5 sekunnin välein
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
RYLR896 LoRa Testi
TILA: VASTAANOTTAJA
Kuuntelee viestejä...
=================================

=== VIESTI VASTAANOTETTU ===
+RCV=1,6,Testi 1,-45,10
Lähettäjä: 1
Viesti: Testi 1
Signaali (RSSI): -45 dBm
SNR: 10
============================
```

## 🔧 Tärkein muuttuja

**`LAHETYS_TILA`** - Ainoa muuttuja jota tarvitsee muuttaa!

```cpp
bool LAHETYS_TILA = true;   // Lähettäjä
bool LAHETYS_TILA = false;  // Vastaanottaja
```

## 📝 Muut asetukset

- **LORA_ADDRESS**: Tämän laitteen osoite (1 tai 2)
- **LORA_NETWORK**: Verkko-ID, pidä sama molemmissa (oletus: 6)
- **TARGET_ADDRESS**: Minne lähetetään (vain lähettäjällä)

## 🐛 Vianetsintä

1. **Ei yhteyttä RYLR896:een**
   - Tarkista kytkennät (TX/RX oikein päin!)
   - Tarkista 3.3V jännite
   - Tarkista BAUD rate (115200)

2. **Ei vastaanota viestejä**
   - Tarkista että LORA_NETWORK on sama molemmissa
   - Tarkista että LORA_ADDRESS on eri molemmissa
   - Varmista että toinen on lähettäjä, toinen vastaanottaja

3. **Heikko signaali**
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
