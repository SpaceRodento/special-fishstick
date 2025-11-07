/*=====================================================================
  Test_Display_UART_Simple.ino - Yksinkertainen UART-testi

  Testaa UART-vastaanottoa ilman TFT-koodia.
  Tulostaa kaiken vastaanotetun datan Serial Monitor:iin.

  Kytkentä:
  - Robotti GPIO 23 (TX) → Display GPIO 18 (RX)
  - GND → GND

  Display syötetty OMASTA USB:sta!

  Testaus:
  1. Lataa tämä koodi display-ESP32:lle
  2. Avaa Serial Monitor (115200 baud)
  3. Lähetä dataa robotilta
  4. Pitäisi näkyä: "RX: data tässä"
=======================================================================*/

#define UART_RX_PIN 18
#define UART_TX_PIN 19  // Ei käytetä
#define UART_BAUDRATE 115200

HardwareSerial DataSerial(1);  // UART1

void setup() {
  // USB Serial (debug)
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  UART TEST - Display ESP32           ║");
  Serial.println("╚════════════════════════════════════════╝\n");

  // UART1 (data from robot)
  DataSerial.begin(UART_BAUDRATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  Serial.println("✅ UART initialized");
  Serial.print("  RX Pin: GPIO ");
  Serial.println(UART_RX_PIN);
  Serial.print("  Baudrate: ");
  Serial.println(UART_BAUDRATE);
  Serial.println("\n📡 Waiting for data...\n");
}

void loop() {
  // Read from UART1
  if (DataSerial.available()) {
    String message = DataSerial.readStringUntil('\n');
    message.trim();

    if (message.length() > 0) {
      Serial.print("📥 RX: ");
      Serial.println(message);
    }
  }

  delay(10);
}
