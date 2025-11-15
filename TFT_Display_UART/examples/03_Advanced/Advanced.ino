/*=====================================================================
  Advanced.ino - Advanced TFT Display Example

  Demonstrates more advanced features:
  - Multiple data types (integers, floats, strings)
  - System metrics (uptime, heap memory)
  - LED status indication
  - Alert conditions

  This is a more complete example showing various use cases.

  Hardware:
  - Sender ESP32 (any model)
  - Display ESP32-2432S022 running DisplayDevice.ino
  - Optional: LED on GPIO 2

  Wiring:
  - Sender TX (GPIO 17) → Display RX (GPIO 3)
  - Sender GND → Display GND

  Copy DisplayClient.h to the same folder as this sketch!
=======================================================================*/

#include "DisplayClient.h"

// Create display client
DisplayClient display(17);

// LED pin (built-in LED on most ESP32 boards)
#define LED_PIN 2

// Tracking variables
int messageCount = 0;
unsigned long startTime = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Advanced Display Example ===\n");

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize display
  display.begin();

  startTime = millis();

  // Show startup alert
  display.alert("System starting...");
  delay(2000);
  display.clearAlert();

  Serial.println("System ready!");
}

void loop() {
  // Toggle LED
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);

  // Calculate uptime
  unsigned long uptimeSeconds = (millis() - startTime) / 1000;
  int hours = uptimeSeconds / 3600;
  int minutes = (uptimeSeconds % 3600) / 60;
  int seconds = uptimeSeconds % 60;

  String uptimeStr = String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";

  // Get system metrics
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t heapPercent = (freeHeap * 100) / ESP.getHeapSize();

  // Simulate some sensor data
  float voltage = 3.3 + (random(-10, 10) / 100.0);  // 3.2-3.4V
  int rssi = random(-90, -50);  // Simulated WiFi signal

  // Build and send message
  display.clear();
  display.set("Count", messageCount);
  display.set("LED", ledState ? "ON" : "OFF");
  display.set("Uptime", uptimeStr);
  display.set("Heap", String(freeHeap / 1024) + "KB");
  display.set("HeapPct", String(heapPercent) + "%");
  display.set("Voltage", String(voltage, 2) + "V");
  display.set("RSSI", String(rssi) + "dBm");
  display.set("Status", getSystemStatus(heapPercent, voltage));
  display.send();

  // Check alert conditions
  if (heapPercent < 20) {
    display.alert("Low memory warning!");
  } else if (voltage < 3.2) {
    display.alert("Low voltage!");
  } else {
    display.clearAlert();
  }

  // Increment message counter
  messageCount++;

  delay(1000);  // Update once per second
}

// =============== HELPER FUNCTIONS ================================

/**
 * Determine system status based on metrics
 */
String getSystemStatus(int heapPercent, float voltage) {
  if (heapPercent < 20 || voltage < 3.2) {
    return "WARNING";
  }
  if (heapPercent < 40 || voltage < 3.3) {
    return "CAUTION";
  }
  return "GOOD";
}
