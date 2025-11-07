# INCREMENTAL UART TEST - Debuggaustesti

**Tarkoitus:** Selvittää MISSÄ kohtaa koodi kaatuu kun UART lisätään.

---

## ✅ Mitä tiedämme JO:

1. ✅ Test_Display_Minimal.ino TOIMII (Serial OK)
2. ✅ Test_Display_TFT_Only.ino TOIMII (TFT näyttää tekstiä)
3. ❌ Test_Display_UART_Simple.ino EI TOIMI (valkoinen näyttö)

**Ongelma:** Kun UART-koodi lisätään, koodi kaatuu.

**Ratkaisu:** Tämä testi lisää UART:n VAIHEITTAIN ja tulostaa Serial.println() jokaisen vaiheen jälkeen.

---

## 🎯 Testaa TÄMÄ nyt:

### 1. Lataa koodi

```
File > Open > Test_Display_Incremental.ino
Board: ESP32 Dev Module
Upload
```

### 2. Avaa Serial Monitor (115200 baud)

### 3. Paina RESET

---

## 📊 Mitä pitäisi näkyä:

**Serial Monitor:**
```
=== INCREMENTAL UART TEST ===
Step 1: Starting...
Step 2: Backlight...
  OK
Step 3: TFT init...
  OK
Step 4: TFT setup...
  OK
Step 5: Draw text...
  OK
Step 6: pinMode for UART...
  OK
Step 7: UART begin...
  OK
Step 8: Waiting for UART data...
  OK

=== ALL STEPS COMPLETED ===
If you see this, UART init worked!
Now send data from robot...
```

**Display:**
```
UART TEST
Waiting...
```

---

## 🔍 Diagnoosi:

### A) Jos pysähtyy Step 6:een

```
Step 5: Draw text...
  OK
Step 6: pinMode for UART...
(ei enää mitään)
```

**Syy:** pinMode(18, INPUT) kaataa koodin
**Ratkaisu:** GPIO 18 on varattu jollekin muulle, vaihdetaan eri pinni

### B) Jos pysähtyy Step 7:ään

```
Step 6: pinMode for UART...
  OK
Step 7: UART begin...
(ei enää mitään)
```

**Syy:** HardwareSerial(1).begin() kaataa
**Ratkaisu:** UART1 on varattu, vaihdetaan UART2

### C) Jos kaikki stepit näkyvät

```
=== ALL STEPS COMPLETED ===
```

**Hienoa!** UART-alustus toimii!

**Seuraavaksi:**
- Lataa Test_Robot_TX_Simple.ino robotille
- Kytke kaapelit (GPIO 23 → GPIO 18, GND → GND)
- Pitäisi näkyä "RX: HELLO 1"

---

## 💬 Kerro tulokset:

**Mihin kohtaan pysähtyy Serial Monitor?**

```
[ ] Step 1
[ ] Step 2
[ ] Step 3
[ ] Step 4
[ ] Step 5
[ ] Step 6  ← Tässä = pinMode ongelma
[ ] Step 7  ← Tässä = UART begin ongelma
[ ] ALL STEPS COMPLETED ← UART toimii!
```

**Kopioi Serial Monitor output tähän:**
```
(liitä tähän)
```

---

**Tämä kertoo TARKALLEEN missä ongelma on!**
