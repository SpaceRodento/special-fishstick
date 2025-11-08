/*=====================================================================
  light_detector.h - Smoke Alarm Light Detection

  FEATURE 12: Light Detection (Palovaroittimen valotarkkailu)

  Detects smoke alarm visual indicator (flashing red LED).
  Useful for remote monitoring when audio is not reliable.

  Use Case:
  - Noisy environment where audio detection difficult
  - Redundant detection (audio + light = more reliable)
  - Deaf/hearing impaired monitoring
  - Visual confirmation of alarm

  How it Works:
  1. Read RGB + Lux values from TCS34725 sensor
  2. Detect red color (high R, low G/B ratio)
  3. Detect flashing pattern (1 Hz typical)
  4. Send LoRa alert when red flash detected

  Recommended Sensor:

  TCS34725 RGB Color Sensor (BEST CHOICE)
  - I2C interface (easy to use)
  - Measures Lux + RGB values
  - Integrated IR filter
  - Adjustable integration time and gain
  - Price: ~8-12€
  - Library: Adafruit_TCS34725

  Installation:
  Arduino IDE: Tools → Manage Libraries → "Adafruit TCS34725"
  Or: https://github.com/adafruit/Adafruit_TCS34725

  Connections:
  TCS34725 → ESP32
  VIN → 3.3V (or 5V if using level shifter)
  GND → GND
  SDA → GPIO 21 (I2C SDA)
  SCL → GPIO 22 (I2C SCL)
  LED → 3.3V (optional, sensor's white LED for illumination)
  INT → Not used (interrupt pin, optional)

  Alternative: Simple Phototransistor + Red Filter
  - Cheaper (~1-2€)
  - Analog output → GPIO 36 (ADC1_CH0) or GPIO 39 (ADC1_CH3)
  - Note: GPIO 35 already used by battery monitor
  - Less accurate, no color discrimination
  - Good enough for basic detection

  Smoke Alarm Visual Characteristics:
  - Color: Red (wavelength ~620-750 nm)
  - Pattern: Flashing 1 Hz (1 flash per second)
  - Some alarms: Steady red during alarm
  - Brightness: Visible in daylight

  Calibration:
  1. Measure ambient red level (no alarm)
  2. Set RED_THRESHOLD 2-3× ambient
  3. Test with actual alarm LED
  4. Check flash detection pattern
  5. Adjust thresholds if needed

  Red Detection Logic:
  - R > threshold (e.g., R > 100)
  - R > G and R > B (red dominant)
  - Ratio R/G > 2.0 and R/B > 2.0

  Flash Detection:
  - ON/OFF transitions within 0.5-2 seconds
  - Minimum 2 flashes to confirm
  - Ignore ambient light changes (slow)

  Testing:
  1. Set ENABLE_LIGHT_DETECTION true in config.h
  2. Upload code
  3. Shine red LED flashlight
  4. Or: Use phone flashlight with red filter
  5. Should detect and send alert

  Alert Format:
  When red flash detected: "ALERT:FIRE_LIGHT,RED:255,FLASHES:5"

  Performance:
  - Update rate: 10 Hz (100ms)
  - Detection latency: 1-3 seconds
  - CPU usage: Low (~2%)
  - Memory: ~100 bytes

  Tips:
  - Mount sensor facing smoke alarm LED
  - Distance: 0.5-3 meters optimal
  - Avoid direct sunlight (can interfere)
  - Use cardboard tube to focus on LED only
=======================================================================*/

#ifndef LIGHT_DETECTOR_H
#define LIGHT_DETECTOR_H

#include <Arduino.h>
#include "config.h"

// Check if TCS34725 library is available
// Note: Requires Adafruit_TCS34725 library
// Install: Arduino Library Manager → "Adafruit TCS34725"
#if ENABLE_LIGHT_DETECTION
  // Library will be included in main .ino if feature enabled
  // #include <Adafruit_TCS34725.h>
  // TCS34725 sensor object will be created in main code
#endif

// Light detection configuration
#define LIGHT_UPDATE_INTERVAL 100       // Update every 100ms (10 Hz)
#ifndef RED_THRESHOLD
  #define RED_THRESHOLD 100             // Minimum red value for detection (default)
#endif
#define RED_RATIO_THRESHOLD 2.0         // R must be 2× larger than G and B
#define FLASH_MIN_INTERVAL 300          // Min time between flashes (ms)
#define FLASH_MAX_INTERVAL 2000         // Max time between flashes (ms)
#define FLASH_CONFIRM_COUNT 2           // Flashes needed to confirm alarm
#define LIGHT_COOLDOWN 5000             // Cooldown between alerts (5s)

// Runtime threshold (can be changed without recompiling)
int redThreshold = RED_THRESHOLD;

// Light detection state
struct LightDetector {
  // Current readings
  uint16_t red;                        // Current red value (0-65535)
  uint16_t green;                      // Current green value
  uint16_t blue;                       // Current blue value
  uint16_t clear;                      // Current clear/lux value
  uint16_t lux;                        // Calculated lux

  // Red detection
  bool redDetected;                    // Red light currently detected
  unsigned long redStartTime;          // When red detected
  unsigned long lastRedTime;           // Last red detection time

  // Flash detection
  int flashCount;                      // Flashes in current sequence
  unsigned long lastFlashTime;         // Last flash time
  bool flashSequenceActive;            // Currently detecting flash sequence

  // Alarm state
  bool alarmDetected;                  // Alarm confirmed
  unsigned long alarmStartTime;        // When alarm started
  unsigned long lastAlertTime;         // Last alert sent
  int alertCount;                      // Alerts sent this session

  // Calibration
  uint16_t ambientRed;                 // Ambient red level (no alarm)
  uint16_t maxRed;                     // Maximum red seen
  bool isCalibrated;                   // Calibration complete

  // Statistics
  unsigned long samplesProcessed;      // Total samples
  int falsePositives;                  // False positive count
  unsigned long lastUpdate;            // Last update time
  bool sensorAvailable;                // TCS34725 sensor detected
};

LightDetector light = {0, 0, 0, 0, 0, false, 0, 0, 0, 0, false,
                       false, 0, 0, 0, 0, 0, false, 0, 0, 0, false};

// Initialize light detector
// Note: Actual TCS34725 initialization done in main code
void initLightDetector() {
  #if ENABLE_LIGHT_DETECTION
    light.lastUpdate = millis();
    light.sensorAvailable = false;  // Will be set by main code

    Serial.println("💡 Light detection initialized");
    Serial.println("  Sensor: TCS34725 RGB Color Sensor");
    Serial.println("  I2C: SDA=GPIO21, SCL=GPIO22");
    Serial.print("  Red threshold: ");
    Serial.println(redThreshold);
    Serial.print("  Ratio threshold: ");
    Serial.println(RED_RATIO_THRESHOLD);
    Serial.println("  🚨 Smoke alarm LED monitoring active");
    Serial.println("  ⚠️  Requires Adafruit_TCS34725 library!");
  #endif
}

// Update readings from TCS34725 sensor
// This function will be called from main code with sensor data
void updateLightReadings(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
  #if ENABLE_LIGHT_DETECTION
    light.red = r;
    light.green = g;
    light.blue = b;
    light.clear = c;

    // Simple lux calculation (approximate)
    light.lux = c;

    // Track maximum red
    if (r > light.maxRed) {
      light.maxRed = r;
    }

    light.samplesProcessed++;
    light.sensorAvailable = true;
  #endif
}

// Check if red color is dominant
bool isRedDominant() {
  #if ENABLE_LIGHT_DETECTION
    // Red must be above threshold
    if (light.red < redThreshold) {
      return false;
    }

    // Red must be significantly higher than green and blue
    // Avoid division by zero
    if (light.green == 0 || light.blue == 0) {
      return (light.red > redThreshold);
    }

    float redGreenRatio = (float)light.red / (float)light.green;
    float redBlueRatio = (float)light.red / (float)light.blue;

    return (redGreenRatio > RED_RATIO_THRESHOLD &&
            redBlueRatio > RED_RATIO_THRESHOLD);
  #else
    return false;
  #endif
}

// Detect flash events
void detectFlash() {
  #if ENABLE_LIGHT_DETECTION
    unsigned long now = millis();

    bool currentlyRed = isRedDominant();

    // Rising edge: Red detected
    if (currentlyRed && !light.redDetected) {
      light.redDetected = true;
      light.redStartTime = now;

      // Check if this is a flash (transition from OFF to ON)
      unsigned long timeSinceLastFlash = now - light.lastFlashTime;

      if (timeSinceLastFlash > FLASH_MIN_INTERVAL &&
          timeSinceLastFlash < FLASH_MAX_INTERVAL) {

        // Valid flash!
        light.flashCount++;
        light.lastFlashTime = now;
        light.flashSequenceActive = true;

        Serial.print("💡 Flash detected! Count: ");
        Serial.println(light.flashCount);

        // Check if we have enough flashes to confirm alarm
        if (light.flashCount >= FLASH_CONFIRM_COUNT && !light.alarmDetected) {
          light.alarmDetected = true;
          light.alarmStartTime = now;

          Serial.println("\n🚨🚨🚨 SMOKE ALARM LIGHT DETECTED! 🚨🚨🚨");
          Serial.print("  Red value: ");
          Serial.println(light.red);
          Serial.print("  Flashes: ");
          Serial.println(light.flashCount);
          Serial.println("  Sending LoRa alert...");

          // Send alert via LoRa
          #if defined(LORA_SENDER_ADDRESS)
            extern void sendLoRaMessage(String payload, int address);
            String alert = "ALERT:FIRE_LIGHT,RED:" + String(light.red) +
                          ",FLASHES:" + String(light.flashCount);
            sendLoRaMessage(alert, LORA_SENDER_ADDRESS);
          #endif

          light.alertCount++;
          light.lastAlertTime = now;
        }
      } else if (timeSinceLastFlash >= FLASH_MAX_INTERVAL) {
        // Too long since last flash, reset sequence
        light.flashCount = 1;
        light.lastFlashTime = now;
        light.flashSequenceActive = true;
      }
    }

    // Falling edge: Red no longer detected
    if (!currentlyRed && light.redDetected) {
      light.redDetected = false;
      light.lastRedTime = now;
    }

    // Reset flash sequence if too long since last flash
    if (light.flashSequenceActive &&
        now - light.lastFlashTime > FLASH_MAX_INTERVAL) {

      if (light.flashCount < FLASH_CONFIRM_COUNT) {
        // False positive (not enough flashes)
        light.falsePositives++;
      }

      light.flashSequenceActive = false;
      light.flashCount = 0;
    }

    // Send periodic alerts while alarm active
    if (light.alarmDetected &&
        now - light.lastAlertTime > LIGHT_COOLDOWN) {

      Serial.println("🚨 Red light alarm still active...");

      #if defined(LORA_SENDER_ADDRESS)
        extern void sendLoRaMessage(String payload, int address);
        String alert = "ALERT:FIRE_LIGHT,RED:" + String(light.red) +
                      ",DURATION:" + String((now - light.alarmStartTime) / 1000);
        sendLoRaMessage(alert, LORA_SENDER_ADDRESS);
      #endif

      light.lastAlertTime = now;
    }
  #endif
}

// Calibrate ambient red level
void calibrateLightBaseline() {
  #if ENABLE_LIGHT_DETECTION
    if (!light.sensorAvailable) {
      Serial.println("❌ TCS34725 sensor not available");
      return;
    }

    Serial.println("💡 Calibrating light baseline...");
    Serial.println("   Ensure normal lighting, no alarm LED");
    Serial.println("   Measuring for 3 seconds...");

    delay(1000);

    unsigned long sum = 0;
    int measurements = 30;

    for (int i = 0; i < measurements; i++) {
      // Note: Actual sensor read done in main loop
      // This just averages existing values
      sum += light.red;
      delay(100);

      if (i % 10 == 0) {
        Serial.print(".");
      }
    }

    light.ambientRed = sum / measurements;
    light.isCalibrated = true;

    Serial.println();
    Serial.print("✓ Ambient red level: ");
    Serial.println(light.ambientRed);
    Serial.print("  Recommended threshold: ");
    // Use 2× ambient or minimum 100, whichever is higher
    int recommendedThreshold = max((int)(light.ambientRed * 2), 100);
    Serial.println(recommendedThreshold);
  #endif
}

// Update light detection (call regularly)
void updateLightDetection() {
  #if ENABLE_LIGHT_DETECTION
    unsigned long now = millis();

    // Check update interval
    if (now - light.lastUpdate < LIGHT_UPDATE_INTERVAL) {
      return;
    }

    // Detect flashes
    detectFlash();

    light.lastUpdate = now;
  #endif
}

// Print light detection status
void printLightStatus() {
  #if ENABLE_LIGHT_DETECTION
    Serial.println("\n╔══════ LIGHT DETECTION ══════╗");

    if (!light.sensorAvailable) {
      Serial.println("║ Status:         ❌ Sensor not found!");
      Serial.println("║ Check:");
      Serial.println("║   - TCS34725 connected?");
      Serial.println("║   - I2C wiring correct?");
      Serial.println("║   - Library installed?");
      Serial.println("╚═════════════════════════════╝\n");
      return;
    }

    Serial.print("║ Red:            ");
    Serial.println(light.red);
    Serial.print("║ Green:          ");
    Serial.println(light.green);
    Serial.print("║ Blue:           ");
    Serial.println(light.blue);
    Serial.print("║ Lux:            ");
    Serial.println(light.lux);

    Serial.print("║ Red dominant:   ");
    Serial.println(isRedDominant() ? "YES 🔴" : "No");

    Serial.print("║ Alarm active:   ");
    Serial.println(light.alarmDetected ? "🚨 YES!" : "No");

    Serial.print("║ Flash count:    ");
    Serial.println(light.flashCount);

    Serial.print("║ Alerts sent:    ");
    Serial.println(light.alertCount);

    Serial.print("║ False positives:");
    Serial.println(light.falsePositives);

    Serial.print("║ Ambient red:    ");
    Serial.println(light.ambientRed);

    Serial.print("║ Max red:        ");
    Serial.println(light.maxRed);

    Serial.print("║ Samples:        ");
    Serial.println(light.samplesProcessed);

    if (light.isCalibrated) {
      Serial.println("║ Calibration:    ✓ Complete");
    } else {
      Serial.println("║ Calibration:    ⚠️  Needed");
    }

    Serial.println("╚═════════════════════════════╝\n");
  #endif
}

// Get light status string for CSV output
String getLightStatus() {
  #if ENABLE_LIGHT_DETECTION
    String status = String(light.red) + "," +
                   String(light.alarmDetected ? 1 : 0) + "," +
                   String(light.alertCount);
    return status;
  #else
    return "0,0,0";
  #endif
}

// Check if light alarm is currently active
bool isFireLightActive() {
  #if ENABLE_LIGHT_DETECTION
    return light.alarmDetected;
  #else
    return false;
  #endif
}

// Test light detector
void testLightDetector() {
  #if ENABLE_LIGHT_DETECTION
    if (!light.sensorAvailable) {
      Serial.println("❌ Cannot test: Sensor not available");
      return;
    }

    Serial.println("\n💡 Testing light detector...");
    Serial.println("   Flash red LED at sensor!");
    Serial.println("   Monitoring for 10 seconds...\n");

    for (int i = 0; i < 100; i++) {
      // Note: Sensor reading happens in main loop
      updateLightDetection();

      Serial.print("R:");
      Serial.print(light.red);
      Serial.print(" G:");
      Serial.print(light.green);
      Serial.print(" B:");
      Serial.print(light.blue);

      if (isRedDominant()) {
        Serial.print("  🔴 RED!");
      }

      Serial.println();

      delay(100);
    }

    Serial.println("\n✓ Test complete");
    printLightStatus();
  #endif
}

// Get RGB values for external use
void getLightRGB(uint16_t* r, uint16_t* g, uint16_t* b) {
  #if ENABLE_LIGHT_DETECTION
    *r = light.red;
    *g = light.green;
    *b = light.blue;
  #else
    *r = 0;
    *g = 0;
    *b = 0;
  #endif
}

// Manual red threshold adjustment (runtime only, not persisted)
void setRedThreshold(int threshold) {
  #if ENABLE_LIGHT_DETECTION
    int oldThreshold = redThreshold;
    redThreshold = threshold;

    Serial.print("💡 Red threshold changed: ");
    Serial.print(oldThreshold);
    Serial.print(" → ");
    Serial.println(redThreshold);
    Serial.println("  Note: Runtime only, not saved to config.h");
  #endif
}

#endif // LIGHT_DETECTOR_H
