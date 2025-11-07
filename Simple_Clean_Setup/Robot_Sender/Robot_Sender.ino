/*
  SIMPLE ROBOT SENDER

  Minimaalinen lähettäjä joka lähettää dataa näytölle
  - Lähettää CSV-muotoista dataa UART:lla
  - Simuloi robotin dataa
  - Toimii heti ilman konfigurointia
*/

#include <Arduino.h>

// UART Configuration
#define UART_TX_PIN 23         // Robot TX → Display RX
#define UART_BAUDRATE 115200

HardwareSerial DisplaySerial(2);

// Demo data
int counter = 0;
bool ledState = false;
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Setup UART for display
  DisplaySerial.begin(UART_BAUDRATE, SERIAL_8N1, -1, UART_TX_PIN);

  Serial.println("Robot Sender Ready!");
  Serial.println("Sending data to display every 2 seconds...");
}

void loop() {
  if (millis() - lastSend >= 2000) {
    lastSend = millis();

    // Toggle LED for demo
    ledState = !ledState;
    counter++;

    // Build CSV message: KEY:VALUE,KEY2:VALUE2,...
    String msg = "";
    msg += "SEQ:" + String(counter);
    msg += ",LED:" + String(ledState ? "ON" : "OFF");
    msg += ",TEMP:" + String(random(20, 30));
    msg += ",RSSI:" + String(random(-90, -60));
    msg += ",SNR:" + String(random(5, 12));
    msg += "\n";

    // Send to display
    DisplaySerial.print(msg);

    // Debug to serial
    Serial.print("TX [");
    Serial.print(counter);
    Serial.print("]: ");
    Serial.print(msg);
  }

  delay(10);
}
