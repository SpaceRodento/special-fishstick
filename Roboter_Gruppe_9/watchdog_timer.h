/*=====================================================================
  watchdog_timer.h - Hardware Watchdog Timer

  FEATURE 6: Watchdog Timer

  Automatically reboots the ESP32 if the system hangs or freezes.
  Uses ESP32 Task Watchdog Timer (TWDT) to monitor loop() execution.

  How it works:
  1. Watchdog starts with timeout (default 10 seconds)
  2. loop() must call esp_task_wdt_reset() regularly
  3. If reset NOT called within timeout → ESP32 reboots
  4. Prevents infinite loops, deadlocks, and system hangs

  Use cases:
  - Production deployments (auto-recovery)
  - Long-term unattended operation
  - Debugging intermittent hangs
  - Safety-critical applications

  Configuration:
  - WATCHDOG_TIMEOUT_S: Timeout in seconds (5-30 recommended)
  - Too short: False triggers if loop() is legitimately slow
  - Too long: Long hang time before recovery

  Testing:
  1. Set ENABLE_WATCHDOG true in config.h
  2. Upload code
  3. Should see: "✓ Watchdog timer enabled (10s timeout)"
  4. Simulate hang: Add while(1) delay(1000); in loop()
  5. After 10s: ESP32 reboots automatically

  Warning Signs:
  - Serial: "E (12345) task_wdt: Task watchdog got triggered"
  - Serial: "abort() was called at PC 0x..."
  - ESP32 reboots unexpectedly → Check if watchdog triggered

  Notes:
  - Watchdog survives most hangs but not all (hard crashes)
  - Does NOT survive brownout (power issues)
  - Does NOT survive hardware issues (flash corruption)
=======================================================================*/

#ifndef WATCHDOG_TIMER_H
#define WATCHDOG_TIMER_H

#include <Arduino.h>
#include "config.h"

#if ENABLE_WATCHDOG
  #include <esp_task_wdt.h>
#endif

// Watchdog statistics
struct WatchdogStats {
  unsigned long lastReset;       // Last time watchdog was reset
  unsigned long resetCount;      // Total resets this session
  unsigned long maxInterval;     // Longest interval between resets (ms)
  bool isEnabled;                // Watchdog enabled flag
  int timeoutSeconds;            // Configured timeout
};

WatchdogStats wdtStats = {0, 0, 0, false, 0};

// Initialize hardware watchdog timer
void initWatchdog() {
  #if ENABLE_WATCHDOG
    Serial.print("🐕 Initializing watchdog timer (");
    Serial.print(WATCHDOG_TIMEOUT_S);
    Serial.println("s timeout)...");

    // Configure Task Watchdog Timer
    // Parameters:
    // - timeout_s: Timeout in seconds
    // - panic: true = reboot on timeout, false = just print error
    esp_err_t err = esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);

    if (err == ESP_OK) {
      // Add current task to watchdog monitoring
      err = esp_task_wdt_add(NULL);  // NULL = current task

      if (err == ESP_OK) {
        wdtStats.isEnabled = true;
        wdtStats.timeoutSeconds = WATCHDOG_TIMEOUT_S;
        wdtStats.lastReset = millis();

        Serial.println("✓ Watchdog timer enabled");
        Serial.println("  System will auto-reboot if loop() hangs");
        Serial.print("  Timeout: ");
        Serial.print(WATCHDOG_TIMEOUT_S);
        Serial.println(" seconds");
        Serial.println("  ⚠️  IMPORTANT: loop() must run smoothly!");
      } else {
        Serial.print("❌ Failed to add task to watchdog: ");
        Serial.println(esp_err_to_name(err));
      }
    } else {
      Serial.print("❌ Failed to initialize watchdog: ");
      Serial.println(esp_err_to_name(err));
    }
  #else
    // Watchdog disabled
    wdtStats.isEnabled = false;
  #endif
}

// Reset watchdog timer (call regularly in loop!)
void resetWatchdog() {
  #if ENABLE_WATCHDOG
    unsigned long now = millis();

    // Calculate interval since last reset
    unsigned long interval = now - wdtStats.lastReset;

    // Track maximum interval (longest time between resets)
    if (interval > wdtStats.maxInterval) {
      wdtStats.maxInterval = interval;

      // Warning if interval is getting close to timeout
      if (interval > (WATCHDOG_TIMEOUT_S * 1000 * 0.8)) {  // 80% of timeout
        Serial.print("⚠️  Watchdog: Long interval (");
        Serial.print(interval);
        Serial.print(" ms, timeout in ");
        Serial.print((WATCHDOG_TIMEOUT_S * 1000) - interval);
        Serial.println(" ms)");
      }
    }

    // Reset the watchdog
    esp_task_wdt_reset();

    // Update statistics
    wdtStats.lastReset = now;
    wdtStats.resetCount++;
  #endif
}

// Get time since last watchdog reset (ms)
unsigned long getTimeSinceWatchdogReset() {
  #if ENABLE_WATCHDOG
    return millis() - wdtStats.lastReset;
  #else
    return 0;
  #endif
}

// Check if watchdog is enabled
bool isWatchdogEnabled() {
  return wdtStats.isEnabled;
}

// Get watchdog timeout (ms)
unsigned long getWatchdogTimeout() {
  #if ENABLE_WATCHDOG
    return WATCHDOG_TIMEOUT_S * 1000;
  #else
    return 0;
  #endif
}

// Print watchdog statistics
void printWatchdogStats() {
  #if ENABLE_WATCHDOG
    Serial.println("\n╔═══════ WATCHDOG STATISTICS ═══════╗");
    Serial.print("║ Status:          ");
    Serial.println(wdtStats.isEnabled ? "ENABLED ✓" : "DISABLED");

    if (wdtStats.isEnabled) {
      Serial.print("║ Timeout:         ");
      Serial.print(wdtStats.timeoutSeconds);
      Serial.println(" seconds");

      Serial.print("║ Total resets:    ");
      Serial.println(wdtStats.resetCount);

      Serial.print("║ Last reset:      ");
      Serial.print((millis() - wdtStats.lastReset) / 1000);
      Serial.println(" s ago");

      Serial.print("║ Max interval:    ");
      Serial.print(wdtStats.maxInterval);
      Serial.println(" ms");

      // Warning indicator
      float usagePercent = (wdtStats.maxInterval / (float)(WATCHDOG_TIMEOUT_S * 1000)) * 100.0;
      Serial.print("║ Max usage:       ");
      Serial.print(usagePercent, 1);
      Serial.print("% of timeout");

      if (usagePercent > 80.0) {
        Serial.print(" ⚠️  HIGH!");
      } else if (usagePercent > 50.0) {
        Serial.print(" ⚠️");
      } else {
        Serial.print(" ✓");
      }
      Serial.println();

      // Safety margin
      Serial.print("║ Safety margin:   ");
      Serial.print(WATCHDOG_TIMEOUT_S * 1000 - wdtStats.maxInterval);
      Serial.println(" ms");
    }

    Serial.println("╚═══════════════════════════════════╝\n");
  #endif
}

// Temporarily disable watchdog (use with caution!)
// Useful for long blocking operations (e.g., firmware update)
void suspendWatchdog() {
  #if ENABLE_WATCHDOG
    Serial.println("⚠️  Suspending watchdog timer...");
    esp_task_wdt_delete(NULL);  // Remove current task from monitoring
    wdtStats.isEnabled = false;
  #endif
}

// Re-enable watchdog after suspension
void resumeWatchdog() {
  #if ENABLE_WATCHDOG
    Serial.println("✓ Resuming watchdog timer");
    esp_task_wdt_add(NULL);  // Add current task back
    wdtStats.isEnabled = true;
    wdtStats.lastReset = millis();
  #endif
}

// Get watchdog status string for CSV output
String getWatchdogStatus() {
  #if ENABLE_WATCHDOG
    if (wdtStats.isEnabled) {
      return String(millis() - wdtStats.lastReset);  // Time since last reset
    } else {
      return "DISABLED";
    }
  #else
    return "OFF";
  #endif
}

// Test function: Trigger watchdog timeout (for testing only!)
// WARNING: This will cause ESP32 to reboot!
void testWatchdogTimeout() {
  #if ENABLE_WATCHDOG
    Serial.println("\n⚠️⚠️⚠️ WATCHDOG TEST MODE ⚠️⚠️⚠️");
    Serial.println("Simulating system hang...");
    Serial.print("ESP32 will reboot in ");
    Serial.print(WATCHDOG_TIMEOUT_S);
    Serial.println(" seconds");
    Serial.println("This is a TEST - do not use in production!");
    Serial.flush();

    // Infinite loop without resetting watchdog → triggers reboot
    while(1) {
      delay(1000);
      Serial.print(".");
    }
    // ESP32 will reboot before reaching here
  #else
    Serial.println("⚠️  Watchdog is disabled, cannot test");
  #endif
}

#endif // WATCHDOG_TIMER_H
