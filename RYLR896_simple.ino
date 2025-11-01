/*
 * RYLR896 LoRa Moduuli - Automaattinen Lähetin/Vastaanotin
 *
 * ESP32 kytkennät:
 * - RYLR896 TX -> ESP32 GPIO 25 (RX)
 * - RYLR896 RX -> ESP32 GPIO 26 (TX)
 * - RYLR896 VCC -> 3.3V
 * - RYLR896 GND -> GND
 *
 * ROOLIN MÄÄRITYS (HYPPYLANKA):
 * - GPIO 4 -> GND = LÄHETTÄJÄ (Address 1)
 * - GPIO 4 -> IRTI = VASTAANOTTAJA (Address 2)
 *
 * KOODI ON IDENTTINEN MOLEMMISSA LAITTEISSA!
 * Rooli määräytyy automaattisesti hyppylangalla.
 */

#include <HardwareSerial.h>

// ============================================
// ROOLIN MÄÄRITYS HYPPYLANGALLA
// ============================================
#define ROLE_PIN 4  // GPIO 4 määrittää roolin
// Pinni GND:ssä (LOW) = LÄHETTÄJÄ (Address 1)
// Pinni irti (HIGH)   = VASTAANOTTAJA (Address 2)
// ============================================

// Sarjaportin asetukset RYLR896:lle
#define RX_PIN 25  // ESP32 RX <- RYLR896 TX
#define TX_PIN 26  // ESP32 TX -> RYLR896 RX
#define BAUD_RATE 115200

// LoRa asetukset
#define LORA_NETWORK 6      // Verkko-ID (sama molemmissa laitteissa)

// Nämä määräytyvät automaattisesti roolin mukaan
bool LAHETYS_TILA;          // true = lähettäjä, false = vastaanottaja
int LORA_ADDRESS;           // Tämän laitteen osoite (1 tai 2)
int TARGET_ADDRESS;         // Kohteen osoite (2 tai 1)

HardwareSerial LoRaSerial(1);  // Käytetään Serial1 porttia

// Laskuri viesteille
int viestiLaskuri = 0;

void setup() {
  // Debug-sarjaportti USB:n kautta
  Serial.begin(115200);
  delay(1000);

  // ============================================
  // TUNNISTA ROOLI HYPPYLANGASTA
  // ============================================
  // Aseta ROLE_PIN input-tilaan sisäisellä pull-up vastuksella
  pinMode(ROLE_PIN, INPUT_PULLUP);
  delay(100);  // Anna pinnin asettua

  // Lue pinnin tila
  bool pinTila = digitalRead(ROLE_PIN);

  if (pinTila == LOW) {
    // Pinni on kytketty GND:hen -> LÄHETTÄJÄ
    LAHETYS_TILA = true;
    LORA_ADDRESS = 1;
    TARGET_ADDRESS = 2;
  } else {
    // Pinni on irti (pull-up nostaa HIGH) -> VASTAANOTTAJA
    LAHETYS_TILA = false;
    LORA_ADDRESS = 2;
    TARGET_ADDRESS = 1;
  }

  // ============================================
  // NÄYTÄ ROOLI JA ASETUKSET
  // ============================================
  Serial.println("\n\n=================================");
  Serial.println("RYLR896 LoRa - Automaattinen");
  Serial.println("=================================");
  Serial.print("GPIO ");
  Serial.print(ROLE_PIN);
  Serial.print(" tila: ");
  Serial.println(pinTila == LOW ? "LOW (GND)" : "HIGH (irti)");
  Serial.println("---------------------------------");

  if (LAHETYS_TILA) {
    Serial.println("ROOLI: LÄHETTÄJÄ");
    Serial.println("Toiminta: Lähettää viestejä 5s välein");
    Serial.print("Oma osoite: ");
    Serial.println(LORA_ADDRESS);
    Serial.print("Kohde: ");
    Serial.println(TARGET_ADDRESS);
  } else {
    Serial.println("ROOLI: VASTAANOTTAJA");
    Serial.println("Toiminta: Kuuntelee viestejä");
    Serial.print("Oma osoite: ");
    Serial.println(LORA_ADDRESS);
  }
  Serial.println("=================================\n");

  // Käynnistä LoRa sarjaportti
  LoRaSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(500);

  // Alusta RYLR896 moduuli
  alustaLoRa();
}

void loop() {
  if (LAHETYS_TILA) {
    // LÄHETTÄJÄ: Lähetä viesti 5 sekunnin välein
    lahetaViesti();
    delay(5000);
  } else {
    // VASTAANOTTAJA: Kuuntele jatkuvasti viestejä
    vastaanotaViesti();
  }
}

// Alustaa RYLR896 moduulin
void alustaLoRa() {
  Serial.println("Alustetaan RYLR896...");

  // Nollaa moduuli
  lahetaKomento("AT+RESET");
  delay(1000);

  // Aseta osoite
  lahetaKomento("AT+ADDRESS=" + String(LORA_ADDRESS));
  delay(100);

  // Aseta verkko-ID
  lahetaKomento("AT+NETWORKID=" + String(LORA_NETWORK));
  delay(100);

  // Aseta parametrit (taajuus, spreading factor, bandwidth, coding rate, power)
  // AT+PARAMETER=SPREADING_FACTOR,BANDWIDTH,CODING_RATE,PREAMBLE
  // 12,7,1,4 = maksimi kantama, hitain
  lahetaKomento("AT+PARAMETER=12,7,1,4");
  delay(100);

  // Tarkista versio
  lahetaKomento("AT+VER?");
  delay(100);

  Serial.println("RYLR896 valmis!\n");
}

// Lähettää AT-komennon RYLR896:lle
void lahetaKomento(String komento) {
  Serial.print(">> Lähetetään: ");
  Serial.println(komento);

  LoRaSerial.println(komento);
  delay(200);

  // Lue vastaus
  unsigned long aloitusAika = millis();
  while (millis() - aloitusAika < 500) {
    if (LoRaSerial.available()) {
      String vastaus = LoRaSerial.readStringUntil('\n');
      Serial.print("<< Vastaus: ");
      Serial.println(vastaus);
    }
  }
}

// Lähettää viestin LoRa:n kautta
void lahetaViesti() {
  viestiLaskuri++;

  // Muodosta viesti
  String viesti = "Testi " + String(viestiLaskuri);

  // AT+SEND=osoite,pituus,data
  String komento = "AT+SEND=" + String(TARGET_ADDRESS) + "," +
                   String(viesti.length()) + "," + viesti;

  Serial.println("\n--- LÄHETETÄÄN VIESTI ---");
  Serial.print("Viesti #");
  Serial.print(viestiLaskuri);
  Serial.print(": ");
  Serial.println(viesti);

  LoRaSerial.println(komento);

  // Odota vahvistus
  unsigned long aloitusAika = millis();
  bool vahvistettu = false;

  while (millis() - aloitusAika < 2000) {
    if (LoRaSerial.available()) {
      String vastaus = LoRaSerial.readStringUntil('\n');
      Serial.print("LoRa: ");
      Serial.println(vastaus);

      if (vastaus.indexOf("+OK") >= 0) {
        vahvistettu = true;
        Serial.println("✓ Lähetys OK!");
      }
    }
  }

  if (!vahvistettu) {
    Serial.println("✗ Ei vahvistusta!");
  }
  Serial.println("------------------------\n");
}

// Vastaanottaa viestejä LoRa:sta
void vastaanotaViesti() {
  if (LoRaSerial.available()) {
    String vastaus = LoRaSerial.readStringUntil('\n');

    // RYLR896 lähettää vastaanotetut viestit muodossa:
    // +RCV=lähettäjä,pituus,data,RSSI,SNR
    if (vastaus.indexOf("+RCV=") >= 0) {
      Serial.println("\n=== VIESTI VASTAANOTETTU ===");
      Serial.println(vastaus);

      // Parsitaan viesti
      int alku = vastaus.indexOf("+RCV=") + 5;
      int pilkku1 = vastaus.indexOf(',', alku);
      int pilkku2 = vastaus.indexOf(',', pilkku1 + 1);
      int pilkku3 = vastaus.indexOf(',', pilkku2 + 1);
      int pilkku4 = vastaus.indexOf(',', pilkku3 + 1);

      if (pilkku1 > 0 && pilkku2 > 0) {
        String lahettaja = vastaus.substring(alku, pilkku1);
        String pituus = vastaus.substring(pilkku1 + 1, pilkku2);
        String data = vastaus.substring(pilkku2 + 1, pilkku3);
        String rssi = vastaus.substring(pilkku3 + 1, pilkku4);
        String snr = vastaus.substring(pilkku4 + 1);

        Serial.print("Lähettäjä: ");
        Serial.println(lahettaja);
        Serial.print("Viesti: ");
        Serial.println(data);
        Serial.print("Signaali (RSSI): ");
        Serial.print(rssi);
        Serial.println(" dBm");
        Serial.print("SNR: ");
        Serial.println(snr);
      }
      Serial.println("============================\n");
    } else {
      // Muut vastaukset (OK, ERROR, jne)
      Serial.print("LoRa: ");
      Serial.println(vastaus);
    }
  }
  delay(10);  // Pieni viive CPU:n helpottamiseksi
}
