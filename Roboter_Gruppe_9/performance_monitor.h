/*=====================================================================
  performance_monitor.h - System Performance Monitoring

  FEATURE 5: Performance Monitoring

  Tracks system health and performance metrics:
  - CPU usage estimation (loop frequency)
  - Memory usage (free heap, minimum heap)
  - Uptime
  - Loop iterations
  - Memory leak detection
  - Performance warnings

  Prints detailed report every 60 seconds (configurable).

  Testing:
  1. Set ENABLE_PERFORMANCE_MONITOR true in config.h
  2. Upload code
  3. Check serial output every 60 seconds
  4. Should see performance report with all metrics

  Interpretation:
  - Loop frequency: Should be >100 Hz normally
  - Free heap: Should stay stable (not decreasing)
  - Min heap: If decreasing over time → memory leak!
=======================================================================*/

#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include <Arduino.h>
#include "config.h"

// Performance metrics
struct PerformanceMetrics {
  // Time
  unsigned long uptimeSeconds;
  unsigned long startTime;

  // Memory
  int freeHeapKB;
  int minFreeHeapKB;
  int initialHeapKB;

  // CPU
  int loopFrequency;          // Loops per second
  unsigned long loopCount;    // Total loop iterations
  unsigned long lastLoopTime; // Last frequency calculation
  unsigned long loopCountSnapshot; // For frequency calculation

  // Reporting
  unsigned long lastReport;
  int reportCount;

  // Warnings
  bool lowMemoryWarning;
  bool memoryLeakWarning;
};

PerformanceMetrics perf = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, false};

// Memory warning threshold (KB)
#define MEMORY_WARNING_THRESHOLD 50

// Initialize performance monitoring
void initPerformanceMonitor() {
  #if ENABLE_PERFORMANCE_MONITOR
    perf.startTime = millis();
    perf.initialHeapKB = ESP.getFreeHeap() / 1024;
    perf.freeHeapKB = perf.initialHeapKB;
    perf.minFreeHeapKB = perf.initialHeapKB;
    perf.lastLoopTime = millis();
    perf.lastReport = millis();

    Serial.println("✓ Performance monitor initialized");
    Serial.print("  Initial free heap: ");
    Serial.print(perf.initialHeapKB);
    Serial.println(" KB");
    Serial.print("  Report interval: ");
    Serial.print(PERF_REPORT_INTERVAL / 1000);
    Serial.println(" seconds");
  #endif
}

// Update performance metrics (call every loop)
void updatePerformanceMetrics() {
  #if ENABLE_PERFORMANCE_MONITOR
    unsigned long now = millis();

    // Update loop counter
    perf.loopCount++;

    // Update uptime
    perf.uptimeSeconds = (now - perf.startTime) / 1000;

    // Update memory stats
    int currentHeap = ESP.getFreeHeap();
    perf.freeHeapKB = currentHeap / 1024;
    int minHeap = ESP.getMinFreeHeap();
    perf.minFreeHeapKB = minHeap / 1024;

    // Check memory warnings
    if (perf.freeHeapKB < MEMORY_WARNING_THRESHOLD) {
      if (!perf.lowMemoryWarning) {
        Serial.println("⚠️ LOW MEMORY WARNING!");
        Serial.print("   Free heap: ");
        Serial.print(perf.freeHeapKB);
        Serial.println(" KB");
        perf.lowMemoryWarning = true;
      }
    } else {
      perf.lowMemoryWarning = false;
    }

    // Check for memory leak (min heap decreasing)
    static int previousMinHeap = perf.minFreeHeapKB;
    if (perf.minFreeHeapKB < previousMinHeap - 5) {  // 5 KB drop
      if (!perf.memoryLeakWarning) {
        Serial.println("⚠️ POSSIBLE MEMORY LEAK DETECTED!");
        Serial.print("   Min heap dropped from ");
        Serial.print(previousMinHeap);
        Serial.print(" KB to ");
        Serial.print(perf.minFreeHeapKB);
        Serial.println(" KB");
        perf.memoryLeakWarning = true;
      }
      previousMinHeap = perf.minFreeHeapKB;
    }

    // Calculate loop frequency (every second)
    if (now - perf.lastLoopTime >= 1000) {
      perf.loopFrequency = perf.loopCount - perf.loopCountSnapshot;
      perf.loopCountSnapshot = perf.loopCount;
      perf.lastLoopTime = now;
    }
  #endif
}

// Print performance report
void printPerformanceReport() {
  #if ENABLE_PERFORMANCE_MONITOR
    unsigned long now = millis();

    // Check if it's time to report
    if (now - perf.lastReport < PERF_REPORT_INTERVAL) {
      return;
    }

    perf.lastReport = now;
    perf.reportCount++;

    // Print report
    Serial.println("\n╔═══════════════ PERFORMANCE REPORT ═══════════════╗");
    Serial.print("║ Report #");
    Serial.println(perf.reportCount);

    // Uptime
    Serial.print("║ Uptime:        ");
    if (perf.uptimeSeconds < 60) {
      Serial.print(perf.uptimeSeconds);
      Serial.println(" seconds");
    } else if (perf.uptimeSeconds < 3600) {
      Serial.print(perf.uptimeSeconds / 60);
      Serial.print(" min ");
      Serial.print(perf.uptimeSeconds % 60);
      Serial.println(" sec");
    } else {
      Serial.print(perf.uptimeSeconds / 3600);
      Serial.print(" hours ");
      Serial.print((perf.uptimeSeconds % 3600) / 60);
      Serial.println(" min");
    }

    // Loop stats
    Serial.print("║ Loop freq:     ");
    Serial.print(perf.loopFrequency);
    Serial.print(" Hz");
    if (perf.loopFrequency < 10) {
      Serial.print(" ⚠️ SLOW!");
    } else if (perf.loopFrequency > 1000) {
      Serial.print(" ✓ Excellent");
    } else if (perf.loopFrequency > 100) {
      Serial.print(" ✓ Good");
    }
    Serial.println();

    Serial.print("║ Total loops:   ");
    Serial.println(perf.loopCount);

    // Memory stats
    Serial.print("║ Free heap:     ");
    Serial.print(perf.freeHeapKB);
    Serial.print(" KB");
    if (perf.freeHeapKB < 50) {
      Serial.print(" ⚠️ LOW!");
    } else {
      Serial.print(" ✓");
    }
    Serial.println();

    Serial.print("║ Min heap:      ");
    Serial.print(perf.minFreeHeapKB);
    Serial.println(" KB");

    Serial.print("║ Initial heap:  ");
    Serial.print(perf.initialHeapKB);
    Serial.println(" KB");

    // Memory usage
    int usedHeap = perf.initialHeapKB - perf.freeHeapKB;
    Serial.print("║ Memory used:   ");
    Serial.print(usedHeap);
    Serial.print(" KB (");
    Serial.print((usedHeap * 100) / perf.initialHeapKB);
    Serial.println("%)");

    // Warnings
    if (perf.lowMemoryWarning) {
      Serial.println("║ ⚠️ WARNING: Low memory!");
    }
    if (perf.memoryLeakWarning) {
      Serial.println("║ ⚠️ WARNING: Possible memory leak!");
    }

    Serial.println("╚══════════════════════════════════════════════════╝\n");
  #endif
}

// Get performance summary string (for CSV output)
String getPerformanceStatus() {
  #if ENABLE_PERFORMANCE_MONITOR
    return String(perf.loopFrequency) + "," + String(perf.freeHeapKB);
  #else
    return "0,0";
  #endif
}

// Check if performance is degraded
bool isPerformanceDegraded() {
  #if ENABLE_PERFORMANCE_MONITOR
    return (perf.loopFrequency < 10 || perf.freeHeapKB < MEMORY_WARNING_THRESHOLD);
  #else
    return false;
  #endif
}

// Get loop frequency (for monitoring)
int getLoopFrequency() {
  #if ENABLE_PERFORMANCE_MONITOR
    return perf.loopFrequency;
  #else
    return 0;
  #endif
}

// Get free heap (for monitoring)
int getFreeHeapKB() {
  #if ENABLE_PERFORMANCE_MONITOR
    return perf.freeHeapKB;
  #else
    return 0;
  #endif
}

#endif // PERFORMANCE_MONITOR_H
