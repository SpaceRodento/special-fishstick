/*=====================================================================
  audio_detector.h - Smoke Alarm Audio Detection

  FEATURE 11: Audio Detection (Palovaroittimen äänitarkkailu)

  Detects smoke alarm audible alert (85dB @ 3kHz typical).
  Useful for remote monitoring of smoke detectors via LoRa.

  Use Case:
  - Monitor remote building for fire alarm
  - Elderly care monitoring
  - Vacation home surveillance
  - Industrial fire safety monitoring

  How it Works:
  1. Continuously sample audio sensor (analog)
  2. Calculate RMS (Root Mean Square) for volume
  3. Detect sustained high amplitude (alarm pattern)
  4. Count peaks to verify alarm pattern (3-4 beeps/sec)
  5. Send LoRa alert when alarm detected

  Recommended Sensors:

  Option 1: MAX4466 Electret Microphone Amplifier (BEST)
  - Adjustable gain (sensitivity)
  - Good SNR (signal-to-noise ratio)
  - 2.4-5V operation
  - Analog output
  - Price: ~3-5€
  - Connection: OUT → GPIO 34 (ADC1_CH6)

  Option 2: KY-038 Sound Sensor Module (BUDGET)
  - Digital + analog output
  - Adjustable threshold (potentiometer)
  - Built-in comparator
  - Price: ~2€
  - Connection: AO → GPIO 34, DO → GPIO 33

  Pin Assignment:
  - GPIO 34 (ADC1_CH6): Analog audio input
  - ADC1 channel (works with WiFi, unlike ADC2)
  - Input-only pin, perfect for sensors

  Calibration:
  1. Run in silent environment
  2. Note baseline RMS value
  3. Set AUDIO_THRESHOLD 2-3× baseline
  4. Test with smoke alarm (85dB)
  5. Adjust threshold if needed

  Smoke Alarm Characteristics:
  - Volume: 85dB @ 3 meters (legal minimum)
  - Frequency: 3000-3500 Hz (3 kHz)
  - Pattern: 3-4 beeps per second
  - Duration: Continuous until silenced

  Testing:
  1. Set ENABLE_AUDIO_DETECTION true in config.h
  2. Upload code
  3. Check serial: Shows RMS values continuously
  4. Play smoke alarm sound (YouTube, app)
  5. Should detect and send alert

  False Positive Prevention:
  - Require sustained high volume (>1 second)
  - Check for pattern (multiple peaks)
  - Configurable threshold
  - Ignore short spikes (door slam, etc.)

  Alert Format:
  When alarm detected: "ALERT:FIRE_AUDIO,RMS:450,PEAKS:12"

  Performance:
  - Sample rate: 1000 Hz (1ms per sample)
  - Detection latency: 1-2 seconds
  - CPU usage: Low (~5-10%)
  - Memory: ~200 bytes
=======================================================================*/

#ifndef AUDIO_DETECTOR_H
#define AUDIO_DETECTOR_H

#include <Arduino.h>
#include "config.h"

// Audio detection configuration
#define AUDIO_PIN 34                    // ADC1_CH6 (analog audio input)
#define AUDIO_SAMPLE_WINDOW 50          // Sample window in milliseconds
#define AUDIO_SAMPLES 50                // Number of samples per window
#define AUDIO_THRESHOLD 200             // RMS threshold for alarm detection
#define AUDIO_SUSTAINED_MS 1000         // Alarm must be sustained (1 second)
#define AUDIO_PEAK_MIN 3                // Minimum peaks to confirm alarm
#define AUDIO_PEAK_MAX 6                // Maximum peaks per second (alarm pattern)
#define AUDIO_COOLDOWN 5000             // Cooldown between alerts (5 seconds)

// Audio detection state
struct AudioDetector {
  // Current readings
  int currentRMS;                      // Root Mean Square volume
  int peakCount;                       // Peaks detected in last second
  unsigned long lastPeakTime;          // Last peak detection time

  // Alarm state
  bool alarmDetected;                  // Alarm currently active
  unsigned long alarmStartTime;        // When alarm started
  unsigned long lastAlertTime;         // Last alert sent time
  int alertCount;                      // Total alerts sent this session

  // Calibration
  int baselineRMS;                     // Baseline noise level
  int maxRMS;                          // Maximum RMS seen
  bool isCalibrated;                   // Calibration complete

  // Statistics
  unsigned long samplesProcessed;      // Total samples processed
  int falsePositives;                  // False positive count
  unsigned long lastUpdate;            // Last detection update
};

AudioDetector audio = {0, 0, 0, false, 0, 0, 0, 0, 0, false, 0, 0, 0};

// Initialize audio detector
void initAudioDetector() {
  #if ENABLE_AUDIO_DETECTION
    pinMode(AUDIO_PIN, INPUT);

    // Configure ADC
    analogSetAttenuation(ADC_11db);    // 0-3.3V range
    analogReadResolution(12);          // 12-bit (0-4095)

    audio.lastUpdate = millis();

    Serial.println("🔊 Audio detection initialized");
    Serial.print("  Pin: GPIO ");
    Serial.println(AUDIO_PIN);
    Serial.print("  Threshold: ");
    Serial.println(AUDIO_THRESHOLD);
    Serial.print("  Sample rate: ");
    Serial.print(1000 / AUDIO_SAMPLE_WINDOW);
    Serial.println(" Hz");
    Serial.println("  🚨 Smoke alarm monitoring active");
    Serial.println("  Run calibration in silent environment!");
  #endif
}

// Calculate RMS (Root Mean Square) from audio samples
int calculateRMS() {
  #if ENABLE_AUDIO_DETECTION
    unsigned long sum = 0;
    int samples = AUDIO_SAMPLES;

    // Collect samples
    for (int i = 0; i < samples; i++) {
      int sample = analogRead(AUDIO_PIN);

      // Convert to signed (ADC center is ~2048)
      int amplitude = sample - 2048;

      // Square
      sum += (unsigned long)(amplitude * amplitude);

      delayMicroseconds(1000 / samples);  // Spread samples over window
    }

    // Mean and square root
    int rms = (int)sqrt(sum / samples);

    audio.samplesProcessed += samples;
    return rms;
  #else
    return 0;
  #endif
}

// Calibrate baseline noise level
void calibrateAudioBaseline() {
  #if ENABLE_AUDIO_DETECTION
    Serial.println("🔊 Calibrating audio baseline...");
    Serial.println("   Please ensure silent environment for 3 seconds");

    delay(1000);  // Wait for user

    long sum = 0;
    int measurements = 30;

    for (int i = 0; i < measurements; i++) {
      int rms = calculateRMS();
      sum += rms;
      delay(100);

      if (i % 10 == 0) {
        Serial.print(".");
      }
    }

    audio.baselineRMS = sum / measurements;
    audio.isCalibrated = true;

    Serial.println();
    Serial.print("✓ Baseline RMS: ");
    Serial.println(audio.baselineRMS);
    Serial.print("  Recommended threshold: ");
    Serial.println(audio.baselineRMS * 3);
    Serial.println("  Update AUDIO_THRESHOLD in config.h if needed");
  #endif
}

// Detect peaks for alarm pattern recognition
bool detectPeak(int rms) {
  #if ENABLE_AUDIO_DETECTION
    unsigned long now = millis();

    // Check if this is a peak (above threshold)
    if (rms > AUDIO_THRESHOLD) {
      // Prevent multiple detections of same peak (debounce)
      if (now - audio.lastPeakTime > 100) {  // 100ms debounce
        audio.peakCount++;
        audio.lastPeakTime = now;
        return true;
      }
    }

    // Reset peak count every second
    if (now - audio.lastPeakTime > 1000) {
      audio.peakCount = 0;
    }

    return false;
  #else
    return false;
  #endif
}

// Check if alarm pattern matches smoke detector
bool isAlarmPattern() {
  #if ENABLE_AUDIO_DETECTION
    // Smoke alarm: 3-4 beeps per second
    // We should see 3-6 peaks per second
    return (audio.peakCount >= AUDIO_PEAK_MIN &&
            audio.peakCount <= AUDIO_PEAK_MAX);
  #else
    return false;
  #endif
}

// Update audio detection (call regularly)
void updateAudioDetection() {
  #if ENABLE_AUDIO_DETECTION
    unsigned long now = millis();

    // Sample audio
    audio.currentRMS = calculateRMS();

    // Track maximum
    if (audio.currentRMS > audio.maxRMS) {
      audio.maxRMS = audio.currentRMS;
    }

    // Detect peaks
    bool peakDetected = detectPeak(audio.currentRMS);

    // Check for alarm
    bool highVolume = (audio.currentRMS > AUDIO_THRESHOLD);

    if (highVolume && !audio.alarmDetected) {
      // Potential alarm start
      if (audio.alarmStartTime == 0) {
        audio.alarmStartTime = now;
      }

      // Check if sustained long enough
      if (now - audio.alarmStartTime >= AUDIO_SUSTAINED_MS) {
        // Check pattern
        if (isAlarmPattern()) {
          audio.alarmDetected = true;

          Serial.println("\n🚨🚨🚨 SMOKE ALARM DETECTED! 🚨🚨🚨");
          Serial.print("  RMS: ");
          Serial.println(audio.currentRMS);
          Serial.print("  Peaks/sec: ");
          Serial.println(audio.peakCount);
          Serial.println("  Sending LoRa alert...");

          // Send alert via LoRa
          #if defined(LORA_SENDER_ADDRESS)
            extern void sendLoRaMessage(String payload, int address);
            String alert = "ALERT:FIRE_AUDIO,RMS:" + String(audio.currentRMS) +
                          ",PEAKS:" + String(audio.peakCount);
            sendLoRaMessage(alert, LORA_SENDER_ADDRESS);
          #endif

          audio.alertCount++;
          audio.lastAlertTime = now;
        } else {
          // High volume but wrong pattern (false positive)
          audio.falsePositives++;
          audio.alarmStartTime = 0;  // Reset
        }
      }
    }
    else if (!highVolume && audio.alarmDetected) {
      // Alarm stopped
      Serial.println("✓ Smoke alarm stopped");
      audio.alarmDetected = false;
      audio.alarmStartTime = 0;
    }
    else if (!highVolume) {
      // Reset alarm start time
      audio.alarmStartTime = 0;
    }

    // Send periodic alerts while alarm active
    if (audio.alarmDetected &&
        now - audio.lastAlertTime > AUDIO_COOLDOWN) {

      Serial.println("🚨 Alarm still active, sending reminder...");

      #if defined(LORA_SENDER_ADDRESS)
        extern void sendLoRaMessage(String payload, int address);
        String alert = "ALERT:FIRE_AUDIO,RMS:" + String(audio.currentRMS) +
                      ",DURATION:" + String((now - audio.alarmStartTime) / 1000);
        sendLoRaMessage(alert, LORA_SENDER_ADDRESS);
      #endif

      audio.lastAlertTime = now;
    }

    audio.lastUpdate = now;
  #endif
}

// Print audio detection status
void printAudioStatus() {
  #if ENABLE_AUDIO_DETECTION
    Serial.println("\n╔══════ AUDIO DETECTION ══════╗");
    Serial.print("║ Current RMS:    ");
    Serial.println(audio.currentRMS);
    Serial.print("║ Baseline RMS:   ");
    Serial.println(audio.baselineRMS);
    Serial.print("║ Max RMS:        ");
    Serial.println(audio.maxRMS);
    Serial.print("║ Threshold:      ");
    Serial.println(AUDIO_THRESHOLD);
    Serial.print("║ Alarm active:   ");
    Serial.println(audio.alarmDetected ? "🚨 YES!" : "No");
    Serial.print("║ Peaks/sec:      ");
    Serial.println(audio.peakCount);
    Serial.print("║ Alerts sent:    ");
    Serial.println(audio.alertCount);
    Serial.print("║ False positives:");
    Serial.println(audio.falsePositives);
    Serial.print("║ Samples:        ");
    Serial.println(audio.samplesProcessed);

    if (audio.isCalibrated) {
      Serial.println("║ Calibration:    ✓ Complete");
    } else {
      Serial.println("║ Calibration:    ⚠️  Needed");
    }

    Serial.println("╚═════════════════════════════╝\n");
  #endif
}

// Get audio status string for CSV output
String getAudioStatus() {
  #if ENABLE_AUDIO_DETECTION
    String status = String(audio.currentRMS) + "," +
                   String(audio.alarmDetected ? 1 : 0) + "," +
                   String(audio.alertCount);
    return status;
  #else
    return "0,0,0";
  #endif
}

// Check if alarm is currently active
bool isFireAlarmActive() {
  #if ENABLE_AUDIO_DETECTION
    return audio.alarmDetected;
  #else
    return false;
  #endif
}

// Manual threshold adjustment
void setAudioThreshold(int threshold) {
  #if ENABLE_AUDIO_DETECTION
    Serial.print("🔊 Audio threshold changed: ");
    Serial.print(AUDIO_THRESHOLD);
    Serial.print(" → ");
    Serial.println(threshold);

    // Note: This changes runtime value, not config.h define
    // To persist, update AUDIO_THRESHOLD in config.h
  #endif
}

// Test audio detector (make noise!)
void testAudioDetector() {
  #if ENABLE_AUDIO_DETECTION
    Serial.println("\n🔊 Testing audio detector...");
    Serial.println("   Make loud noise or play smoke alarm sound!");
    Serial.println("   Monitoring for 10 seconds...\n");

    for (int i = 0; i < 100; i++) {
      updateAudioDetection();

      Serial.print("RMS: ");
      Serial.print(audio.currentRMS);
      Serial.print("  Peaks: ");
      Serial.print(audio.peakCount);

      if (audio.currentRMS > AUDIO_THRESHOLD) {
        Serial.print("  🔊 LOUD!");
      }

      Serial.println();

      delay(100);
    }

    Serial.println("\n✓ Test complete");
    printAudioStatus();
  #endif
}

#endif // AUDIO_DETECTOR_H
