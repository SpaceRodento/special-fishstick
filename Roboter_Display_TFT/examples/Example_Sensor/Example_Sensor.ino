/*=====================================================================
  Example_Sensor.ino - Sensoridata näytölle

  Näyttää miten lähettää sensoriarvoja näytölle.
  Esimerkki: DHT22 lämpötila/kosteus, LDR valoarvo
=======================================================================*/

#include "DisplayClient.h"

// Luo display-client
DisplayClient display(17);

// Simuloidut sensorit (vaihda oikeiksi sensoreiksi!)
#define TEMP_SENSOR_PIN 34
#define LIGHT_SENSOR_PIN 35

float temperature = 0;
float humidity = 0;
int lightLevel = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Sensor Display Example ===\n");

  // Alusta sensorit
  pinMode(TEMP_SENSOR_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);

  // Alusta näyttö
  display.begin();

  // Lähetä aloitusviesti
  display.alert("Sensor system online");
  delay(2000);
  display.clearAlert();
}

void loop() {
  // Lue sensorit (tässä simuloitu)
  temperature = readTemperature();
  humidity = readHumidity();
  lightLevel = readLightLevel();

  // Lähetä kaikki arvot kerralla
  display.clear();
  display.set("Temp", String(temperature, 1) + "C");
  display.set("Humidity", String(humidity, 0) + "%");
  display.set("Light", lightLevel);
  display.set("Status", getStatus());
  display.send();

  // Tarkista hälytysrajat
  if (temperature > 30.0) {
    display.alert("High temperature!");
  } else if (temperature < 15.0) {
    display.alert("Low temperature!");
  } else {
    display.clearAlert();
  }

  delay(2000);  // Päivitä 2 sekunnin välein
}

// Simuloi lämpötilan luku (vaihda oikeaksi!)
float readTemperature() {
  // Esim. DHT22:
  // return dht.readTemperature();

  // Simulaatio:
  return 20.0 + random(-50, 100) / 10.0;
}

// Simuloi kosteuden luku
float readHumidity() {
  // Esim. DHT22:
  // return dht.readHumidity();

  // Simulaatio:
  return 50.0 + random(-20, 20);
}

// Lue valoisuus (LDR-sensor)
int readLightLevel() {
  int raw = analogRead(LIGHT_SENSOR_PIN);
  return map(raw, 0, 4095, 0, 100);  // 0-100%
}

// Päätä tila arvojen perusteella
String getStatus() {
  if (temperature > 28 || temperature < 16) {
    return "WARNING";
  }
  if (humidity > 70 || humidity < 30) {
    return "CAUTION";
  }
  return "OK";
}
