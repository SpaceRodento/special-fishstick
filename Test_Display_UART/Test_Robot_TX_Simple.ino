/*=====================================================================
  Test_Robot_TX_Simple.ino - Yksinkertainen UART-lähetin

  Testaa UART-lähetystä ilman LoRa-koodia.
  Lähettää "HELLO X" viestin 2s välein.

  Kytkentä:
  - Robot GPIO 23 (TX) → Display GPIO 18 (RX)
  - GND → GND

  Testaus:
  1. Lataa tämä koodi robotti-ESP32:lle
  2. Avaa Serial Monitor (115200 baud)
  3. Pitäisi näkyä: "TX: HELLO 1", "TX: HELLO 2", jne.
  4. Display pitäisi vastaanottaa samat viestit
=======================================================================*/

#define DISPLAY_TX_PIN 23  // TX to display
#define DISPLAY_RX_PIN -1  // Not used (TX only)
#define UART_BAUDRATE 115200

HardwareSerial DisplaySerial(2);  // UART2

void setup() {
  // USB Serial (debug)
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  UART TEST - Robot ESP32              ║");
  Serial.println("╚════════════════════════════════════════╝\n");

  // Ensure TX pin is OUTPUT
  pinMode(DISPLAY_TX_PIN, OUTPUT);

  // UART2 (data to display)
  DisplaySerial.begin(UART_BAUDRATE, SERIAL_8N1, DISPLAY_RX_PIN, DISPLAY_TX_PIN);

  Serial.println("✅ UART initialized");
  Serial.print("  TX Pin: GPIO ");
  Serial.println(DISPLAY_TX_PIN);
  Serial.print("  Baudrate: ");
  Serial.println(UART_BAUDRATE);
  Serial.println("\n📡 Sending test messages every 2 seconds...\n");
}

int counter = 0;

void loop() {
  counter++;

  String message = "HELLO " + String(counter);

  // Send to display
  DisplaySerial.println(message);

  // Show in Serial Monitor
  Serial.print("📤 TX: ");
  Serial.println(message);

  delay(2000);
}
