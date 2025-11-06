/*=====================================================================
  test_display_uart.ino - UART Display Connection Tester

  Yksinkertainen testi joka lähettää dataa näytölle ilman muuta logiikkaa.
  Käytä tätä varmistamaan että UART-yhteys toimii.

  Kytkennät:
  - Päälaitteen GPIO 5 (TX) → Näytön GPIO 18 (RX)
  - Päälaitteen GND → Näytön GND  ← PAKOLLINEN!

  Testaus:
  1. Lataa tämä koodi päälaitteeseen
  2. Avaa Serial Monitor (115200 baud)
  3. Näytön pitäisi näyttää "Test: 1, 2, 3..." ja PKT kasvaa
  4. Serial Monitor näyttää mitä lähetetään

  Jos näyttö ei saa dataa:
  - Tarkista GND-yhteys multimittarilla (jatkuvuus)
  - Tarkista että GPIO 5 on kytketty näytön RX:ään (GPIO 18)
  - Mittaa GPIO 5:n jännite kun data lähetetään (pitäisi heilua)
=======================================================================*/

#define DISPLAY_TX_PIN 5    // GPIO 5 → Display RX (GPIO 18)
#define DISPLAY_BAUDRATE 115200

HardwareSerial DisplaySerial(1);  // UART1
int testCounter = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n╔═══════════════════════════════╗");
  Serial.println("║  UART DISPLAY TESTER          ║");
  Serial.println("╚═══════════════════════════════╝\n");

  // Initialize UART for display (TX only)
  DisplaySerial.begin(DISPLAY_BAUDRATE, SERIAL_8N1, -1, DISPLAY_TX_PIN);

  Serial.println("✅ UART initialized:");
  Serial.print("   TX Pin: GPIO ");
  Serial.println(DISPLAY_TX_PIN);
  Serial.print("   Baudrate: ");
  Serial.println(DISPLAY_BAUDRATE);
  Serial.println("   Display RX: GPIO 18");
  Serial.println();

  Serial.println("🔍 Connection checklist:");
  Serial.println("   ☐ GPIO 5 → Display GPIO 18 (RX)");
  Serial.println("   ☐ GND → Display GND");
  Serial.println("   ☐ Display has separate USB power");
  Serial.println();

  delay(1000);

  // Send initial test message
  Serial.println("📤 Sending initial test...");
  DisplaySerial.println("ALERT:UART Test Started");
  delay(1000);

  Serial.println("🚀 Starting continuous test...");
  Serial.println("   Watch display for 'Test: X' and PKT counter");
  Serial.println();
}

void loop() {
  testCounter++;

  // Build simple test message
  String message = "Test:" + String(testCounter) + ",Status:OK";

  // Send to display via UART
  DisplaySerial.println(message);

  // Print to Serial Monitor
  Serial.print("[");
  Serial.print(millis() / 1000);
  Serial.print("s] TX → ");
  Serial.print(message);
  Serial.print(" (");
  Serial.print(message.length());
  Serial.println(" bytes)");

  // Also send a simple alert every 10 messages
  if (testCounter % 10 == 0) {
    String alert = "ALERT:Test " + String(testCounter);
    DisplaySerial.println(alert);
    Serial.print("   🚨 ");
    Serial.println(alert);
  }

  // Send clear command every 20 messages to reset
  if (testCounter % 20 == 0) {
    DisplaySerial.println("CLEAR");
    Serial.println("   🗑️  CLEAR sent");
  }

  delay(1000);  // Send every second
}
