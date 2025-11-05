/*=====================================================================
  Example_Basic.ino - Perusesimerkki näytön käytöstä

  Yksinkertaisin tapa käyttää näyttöä.
  Lähettää muutaman arvon ja päivittää niitä.
=======================================================================*/

#include "DisplayClient.h"

// Luo display-client (TX pin 17)
DisplayClient display(17);

int counter = 0;
float temperature = 22.5;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Basic Display Example ===\n");

  // Alusta näyttöyhteys
  display.begin();

  delay(1000);
}

void loop() {
  // Vaihtoehto 1: Lähetä yksi kenttä kerrallaan
  display.update("Counter", counter);
  delay(100);

  display.update("Temp", temperature);
  delay(100);

  // Vaihtoehto 2: Lähetä useampi kenttä kerralla
  display.clear();
  display.set("Counter", counter);
  display.set("Temp", temperature);
  display.set("Status", "Running");
  display.send();

  // Päivitä arvoja
  counter++;
  temperature += 0.1;

  if (counter > 100) {
    counter = 0;
  }

  if (temperature > 30.0) {
    temperature = 20.0;
  }

  delay(1000);  // Päivitä kerran sekunnissa
}
