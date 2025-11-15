/*=====================================================================
  Sensors.ino - Sensor Data Display Example

  Shows how to send sensor data to the TFT display.
  Example: Temperature, humidity, light level

  This example uses simulated sensors. Replace with real sensor code!

  Hardware:
  - Sender ESP32 with sensors
  - Display ESP32-2432S022 running DisplayDevice.ino

  Wiring:
  - Sender TX (GPIO 17) → Display RX (GPIO 3)
  - Sender GND → Display GND
  - Connect your sensors to appropriate pins

  Copy DisplayClient.h to the same folder as this sketch!
=======================================================================*/

#include "DisplayClient.h"

// Create display client
DisplayClient display(17);

// Simulated sensor pins (replace with your actual sensors!)
#define TEMP_SENSOR_PIN 34
#define LIGHT_SENSOR_PIN 35

float temperature = 0;
float humidity = 0;
int lightLevel = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Sensor Display Example ===\n");

  // Initialize sensor pins
  pinMode(TEMP_SENSOR_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);

  // Initialize display
  display.begin();

  // Send startup message
  display.alert("Sensor system online");
  delay(2000);
  display.clearAlert();
}

void loop() {
  // Read sensors (currently simulated)
  temperature = readTemperature();
  humidity = readHumidity();
  lightLevel = readLightLevel();

  // Send all values at once
  display.clear();
  display.set("Temp", String(temperature, 1) + "C");
  display.set("Humidity", String(humidity, 0) + "%");
  display.set("Light", String(lightLevel) + "%");
  display.set("Status", getStatus());
  display.send();

  // Check alert conditions
  if (temperature > 30.0) {
    display.alert("High temperature!");
  } else if (temperature < 15.0) {
    display.alert("Low temperature!");
  } else {
    display.clearAlert();
  }

  delay(2000);  // Update every 2 seconds
}

// =============== SENSOR READING FUNCTIONS ================================
// Replace these with your actual sensor code!

// Simulate temperature reading
float readTemperature() {
  // Example for DHT22:
  // return dht.readTemperature();

  // Simulation: random value between 15-30°C
  return 20.0 + random(-50, 100) / 10.0;
}

// Simulate humidity reading
float readHumidity() {
  // Example for DHT22:
  // return dht.readHumidity();

  // Simulation: random value between 30-70%
  return 50.0 + random(-20, 20);
}

// Read light level (LDR sensor)
int readLightLevel() {
  // Real implementation:
  int raw = analogRead(LIGHT_SENSOR_PIN);
  return map(raw, 0, 4095, 0, 100);  // Convert to 0-100%
}

// Determine status based on sensor values
String getStatus() {
  if (temperature > 28 || temperature < 16) {
    return "WARNING";
  }
  if (humidity > 70 || humidity < 30) {
    return "CAUTION";
  }
  return "OK";
}
