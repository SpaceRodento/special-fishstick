/*=====================================================================
  DisplayClient.h - Universal Display Client Library

  Helppokäyttöinen kirjasto datan lähettämiseen ESP32-2432S022 näytölle.

  Käyttö pää-ESP32:ssa:

  #include "DisplayClient.h"
  DisplayClient display(17);  // TX pin

  void setup() {
    display.begin();
  }

  void loop() {
    display.set("LED", "ON");
    display.set("Temp", 42);
    display.set("RSSI", -78);
    display.send();  // Lähetä kaikki kerralla

    delay(1000);
  }

  Tämä on kirjasto pää-laitteelle - kopioi tämä omaan projektiisi!
=======================================================================*/

#ifndef DISPLAY_CLIENT_H
#define DISPLAY_CLIENT_H

#include <Arduino.h>

class DisplayClient {
private:
  HardwareSerial* serial;
  uint8_t txPin;
  uint8_t rxPin;
  unsigned long baudrate;
  String dataBuffer;
  bool firstField;

public:
  /**
   * Constructor
   * @param tx_pin TX pin number (connects to display RX)
   * @param rx_pin RX pin number (optional, -1 if not used)
   * @param baud Baudrate (default: 115200)
   */
  DisplayClient(uint8_t tx_pin, uint8_t rx_pin = -1, unsigned long baud = 115200) {
    txPin = tx_pin;
    rxPin = rx_pin;
    baudrate = baud;
    serial = &Serial2;  // Use UART2 (UART1 is used by LoRa!)
    firstField = true;
  }

  /**
   * Initialize serial connection to display
   */
  void begin() {
    // ✅ CRITICAL FIX: Set pinMode() BEFORE serial->begin()
    // Required when using &Serial2 with custom pins (GPIO 23)
    // Based on working Robot_Sender.ino implementation
    pinMode(txPin, OUTPUT);
    if (rxPin != -1) {
      pinMode(rxPin, INPUT);
    }

    // Initialize UART with custom pins
    if (rxPin == -1) {
      // TX only mode (most common)
      serial->begin(baudrate, SERIAL_8N1, -1, txPin);
    } else {
      // Full duplex mode
      serial->begin(baudrate, SERIAL_8N1, rxPin, txPin);
    }

    Serial.println("📺 Display client initialized");
    Serial.print("  TX: GPIO ");
    Serial.println(txPin);
    if (rxPin != -1) {
      Serial.print("  RX: GPIO ");
      Serial.println(rxPin);
    }
    Serial.print("  Baudrate: ");
    Serial.println(baudrate);

    // Send test message
    delay(500);
    serial->println("STATUS:Display connected");
  }

  /**
   * Start building a new message
   * Call this before adding fields
   */
  void clear() {
    dataBuffer = "";
    firstField = true;
  }

  /**
   * Add a key-value pair to message
   * @param key Field name (e.g., "LED", "Temp")
   * @param value Field value (any type)
   */
  template<typename T>
  void set(String key, T value) {
    if (!firstField) {
      dataBuffer += ",";
    }
    dataBuffer += key + ":" + String(value);
    firstField = false;
  }

  /**
   * Send the buffered message to display
   * Call this after adding all fields
   */
  void send() {
    if (dataBuffer.length() > 0) {
      serial->println(dataBuffer);

      // Debug
      Serial.print("→ Display: ");
      Serial.println(dataBuffer);

      clear();  // Reset for next message
    }
  }

  /**
   * Send a complete message in one call
   * Useful for simple messages
   */
  void sendRaw(String message) {
    serial->println(message);
    Serial.print("→ Display: ");
    Serial.println(message);
  }

  /**
   * Show an alert on display
   * @param message Alert message text
   */
  void alert(String message) {
    serial->println("ALERT:" + message);
    Serial.print("🚨 Alert: ");
    Serial.println(message);
  }

  /**
   * Clear alert from display
   */
  void clearAlert() {
    serial->println("CLEAR_ALERT");
  }

  /**
   * Clear all data from display
   */
  void clearDisplay() {
    serial->println("CLEAR");
    Serial.println("🗑️  Display cleared");
  }

  /**
   * Update single field and send immediately
   * Shortcut for simple updates
   */
  template<typename T>
  void update(String key, T value) {
    clear();
    set(key, value);
    send();
  }

  /**
   * Send multiple fields at once (variadic template)
   * Example: display.sendMultiple("LED", "ON", "Temp", 42);
   */
  template<typename... Args>
  void sendMultiple(Args... args) {
    clear();
    addMultiple(args...);
    send();
  }

private:
  // Helper for variadic template
  template<typename T, typename U, typename... Rest>
  void addMultiple(T key, U value, Rest... rest) {
    set(String(key), value);
    addMultiple(rest...);
  }

  template<typename T, typename U>
  void addMultiple(T key, U value) {
    set(String(key), value);
  }
};

#endif // DISPLAY_CLIENT_H
