/*=====================================================================
  Basic.ino - Simple TFT Display Example

  The simplest way to use the TFT display.
  Sends a counter and uptime to the display.

  Hardware:
  - Sender ESP32 (any model)
  - Display ESP32-2432S022 running DisplayDevice.ino

  Wiring:
  - Sender TX (GPIO 17) → Display RX (GPIO 3)
  - Sender GND → Display GND

  Copy DisplayClient.h to the same folder as this sketch!
=======================================================================*/

#include "DisplayClient.h"

// Create display client (TX pin 17)
DisplayClient display(17);

int counter = 0;
unsigned long startTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Basic TFT Display Example ===\n");

  // Initialize display connection
  display.begin();

  startTime = millis();

  delay(1000);
}

void loop() {
  // Calculate uptime
  unsigned long uptime = (millis() - startTime) / 1000;  // seconds
  int hours = uptime / 3600;
  int minutes = (uptime % 3600) / 60;
  int seconds = uptime % 60;

  String uptimeStr = String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";

  // Send multiple fields at once
  display.clear();
  display.set("Counter", counter);
  display.set("Uptime", uptimeStr);
  display.set("Status", "Running");
  display.send();

  // Increment counter
  counter++;

  // Optional: trigger alert at specific count
  if (counter == 50) {
    display.alert("Counter reached 50!");
  }

  if (counter == 60) {
    display.clearAlert();
  }

  // Reset counter at 100
  if (counter > 100) {
    counter = 0;
  }

  delay(1000);  // Update once per second
}
