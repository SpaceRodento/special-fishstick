/*
  RYLR896 Direct Test
  
  Testaa RYLR896-moduulia suoraan ilman LoRa-lähetystä
*/

#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== RYLR896 Direct Test ===\n");
  
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(500);
  
  Serial.println("Type AT commands (e.g. 'AT' or 'AT+VER?')");
  Serial.println("Everything you type will be sent to RYLR896");
  Serial.println("Everything RYLR896 sends will be shown here\n");
}

void loop() {
  // Computer → RYLR896
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    Serial.print(">>> ");
    Serial.println(cmd);
    Serial2.println(cmd);
  }
  
  // RYLR896 → Computer
  if (Serial2.available()) {
    String response = Serial2.readStringUntil('\n');
    Serial.print("<<< ");
    Serial.println(response);
  }
}