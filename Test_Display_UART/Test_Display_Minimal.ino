/*
  MINIMAALINEN TESTI - ESP32-2432S022

  Testaa vain että koodi käynnistyy ja Serial toimii.
  EI mitään muuta.

  Jos tämä ei toimi, Arduino IDE asetukset väärin!
*/

void setup() {
  Serial.begin(115200);
  delay(2000);  // Anna aikaa Serial Monitorin avautua

  Serial.println("\n\n===================");
  Serial.println("MINIMAL TEST START");
  Serial.println("===================");
  Serial.println("If you see this, Serial works!");
  Serial.println();
}

int counter = 0;

void loop() {
  counter++;

  Serial.print("Counter: ");
  Serial.println(counter);

  delay(1000);
}
