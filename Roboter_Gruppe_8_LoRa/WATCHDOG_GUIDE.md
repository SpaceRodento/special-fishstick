# Connection Watchdog & Health Monitoring Guide

## Overview

The Connection Watchdog & Health Monitoring system provides robust tracking and recovery capabilities for LoRa communication.

## Features

### 1. Connection State Machine
```
UNKNOWN -> CONNECTING -> CONNECTED <-> WEAK -> LOST
                              ↓                  ↓
                         (watchdog)      (auto-recovery)
```

**States:**
- **UNKNOWN** (`?`): Initial state before first message
- **CONNECTING** (`~`): LoRa module initializing
- **CONNECTED** (`*`): Good connection, messages flowing
- **WEAK** (`!`): Delayed messages or weak signal
- **LOST** (`X`): No messages for > 8 seconds

### 2. Health Metrics Tracked

#### RSSI Statistics
- **Minimum RSSI**: Weakest signal seen
- **Maximum RSSI**: Strongest signal seen
- **Average RSSI**: Running average over sliding window
- **Sample count**: Number of RSSI readings

#### Packet Tracking
- **Packets received**: Total successfully received
- **Packets lost**: Detected missing sequence numbers
- **Packet loss %**: Lost packets / total expected
- **Duplicate packets**: Same sequence number received twice

#### Connection Quality
- **Uptime**: Total time since device started
- **Connected time**: Duration in CONNECTED state
- **Recovery attempts**: Number of reconnection tries

### 3. Watchdog Thresholds

Default configuration (adjustable in `health_monitor.h`):

```cpp
WatchdogConfig watchdogCfg = {
  .weakTimeout = 3000,           // 3 seconds -> WEAK
  .lostTimeout = 8000,           // 8 seconds -> LOST
  .weakRssiThreshold = -100,     // dBm -> WEAK
  .criticalRssiThreshold = -110, // dBm -> CRITICAL
  .recoveryInterval = 15000,     // Try recovery every 15s
  .maxRecoveryAttempts = 3       // Give up after 3 attempts
};
```

**State Transitions:**
- **CONNECTED**: Messages received < 3s ago, RSSI > -100 dBm
- **WEAK**: Messages 3-8s old OR RSSI -100 to -110 dBm
- **LOST**: No messages for > 8s

### 4. Automatic Recovery

When connection is LOST:
1. Wait for recovery interval (15s by default)
2. Attempt to re-initialize LoRa module
3. If successful: Reset to CONNECTING state
4. If failed: Wait and retry (max 3 attempts)
5. After 3 failures: Manual intervention needed

## LCD Display Integration

### Version 1 (Wide Visual Bar)
```
*[██████████]45 ^  ← Connection status + signal bar + count
-52dB L:1 T:0   v  ← RSSI + local status
```

### Version 2 (Compact with Packet Loss)
```
*[███████] -52  ^  ← Connection status + bar + RSSI
S:8 L:2% 45     v  ← SNR + packet loss % + count
```

**Connection Status Icons:**
- `*` = CONNECTED (good)
- `!` = WEAK (warning)
- `X` = LOST (error)
- `~` = CONNECTING
- `?` = UNKNOWN

## Serial Monitor Output

### Health Report (every 30 seconds)
```
╔═══════════════════════════════════════╗
║        HEALTH MONITOR REPORT         ║
╠═══════════════════════════════════════╣
║ Status:     CONNECTED *
║ Uptime:     125 s
║ Connected:  98 s
╠═══════════════════════════════════════╣
║ RSSI Avg:   -58 dBm
║ RSSI Min:   -72 dBm
║ RSSI Max:   -45 dBm
║ Samples:    47
╠═══════════════════════════════════════╣
║ Packets RX: 49
║ Lost:       2 (3.9%)
║ Duplicate:  0
╚═══════════════════════════════════════╝
```

### State Change Notifications
```
╔════════ CONNECTION STATE CHANGE ════════╗
║ CONNECTED -> WEAK
║ Time since last message: 4.2 s
║ RSSI: -105 dBm
╚═══════════════════════════════════════╝
```

### Packet Loss Detection
```
⚠ Lost packets detected: 2
```

### Recovery Attempts
```
╔════════════════════════════════════╗
║ RECOVERY ATTEMPT #1
║ Re-initializing LoRa module...
╚════════════════════════════════════╝
✓ Recovery successful!
```

## Message Format

Messages now include sequence numbers for packet loss tracking:

**Sender payload:**
```
SEQ:42,LED:1,TOUCH:0,SPIN:2,COUNT:45
```

**Fields:**
- `SEQ`: Sequence number (increments with each message)
- `LED`: LED state (0/1)
- `TOUCH`: Touch sensor state (0/1)
- `SPIN`: Spinner animation index (0-3)
- `COUNT`: Total messages sent

## API Reference

### Initialization
```cpp
HealthMonitor health;
initHealthMonitor(health);
```

### Update Functions (Receiver Loop)
```cpp
// Update RSSI statistics
updateRSSI(health, remote.rssi);

// Track packet for loss detection
trackPacket(health, remote.sequenceNumber);

// Update connection state (watchdog)
updateConnectionState(health, remote);

// Attempt recovery if lost
if (health.state == CONN_LOST) {
  attemptRecovery(health, MY_LORA_ADDRESS, LORA_NETWORK_ID);
}
```

### Query Functions
```cpp
// Get connection state string
const char* state = getConnectionStateString(health.state);

// Get connection icon
char icon = getConnectionIcon(health.state);

// Get average RSSI
int avg = getRSSIAverage(health);

// Get packet loss percentage
float loss = getPacketLoss(health);

// Get uptime string
String uptime = getUptimeString(health);
```

### Reporting
```cpp
// Print full health report
printHealthReport(health, remote);
```

## Tuning the Watchdog

### Adjust Timeouts
Edit `watchdogCfg` in `health_monitor.h`:

```cpp
// More sensitive (faster detection):
watchdogCfg.weakTimeout = 2000;   // 2s -> WEAK
watchdogCfg.lostTimeout = 5000;   // 5s -> LOST

// More tolerant (slower detection):
watchdogCfg.weakTimeout = 5000;   // 5s -> WEAK
watchdogCfg.lostTimeout = 15000;  // 15s -> LOST
```

### Adjust RSSI Thresholds
```cpp
// More sensitive to weak signals:
watchdogCfg.weakRssiThreshold = -90;  // Warn earlier

// More tolerant:
watchdogCfg.weakRssiThreshold = -110; // Warn later
```

### Adjust Recovery
```cpp
// Try recovery more often:
watchdogCfg.recoveryInterval = 10000;  // Every 10s

// More attempts before giving up:
watchdogCfg.maxRecoveryAttempts = 5;
```

## Troubleshooting

### Frequent WEAK state
**Symptom:** State oscillates between CONNECTED and WEAK

**Solutions:**
1. Increase `weakTimeout` to 5000ms
2. Lower `weakRssiThreshold` to -110 dBm
3. Check antenna connections
4. Reduce distance between devices

### Constant LOST state
**Symptom:** Connection stays LOST, recovery fails

**Solutions:**
1. Check sender is powered and running
2. Verify both devices have same Network ID
3. Check LoRa module power supply (3.3V)
4. Inspect wiring (TX->GPIO25, RX->GPIO26)
5. Test with simple RYLR896_simple.ino first

### High packet loss
**Symptom:** Packet loss > 10%

**Solutions:**
1. Check RSSI values (should be > -100 dBm)
2. Reduce distance or remove obstacles
3. Check for interference (WiFi, etc.)
4. Consider different LoRa parameters (SF, BW)

### Recovery attempts exhausted
**Symptom:** "Max recovery attempts reached" message

**Solutions:**
1. Manually reset receiver ESP32
2. Check sender is still running
3. Power cycle both devices
4. Verify LoRa module hardware

## Performance Impact

Minimal overhead:
- **Memory**: ~100 bytes for HealthMonitor struct
- **CPU**: < 1% (only runs on packet receipt and periodic checks)
- **LoRa bandwidth**: +5 bytes per message (sequence number)

## Future Enhancements

Possible additions:
- [ ] Packet retransmission on detected loss
- [ ] ACK/NACK protocol for guaranteed delivery
- [ ] Link quality score (0-100%) combining RSSI, SNR, packet loss
- [ ] Historical graphs on LCD (if custom characters)
- [ ] Adaptive send rate (faster when CONNECTED, slower when WEAK)
- [ ] Bi-directional health monitoring (sender also tracks receiver)

## Summary

The Connection Watchdog provides:
- ✅ Automatic connection state tracking
- ✅ RSSI statistics (min/max/average)
- ✅ Packet loss detection with sequence numbers
- ✅ Automatic recovery from lost connections
- ✅ Comprehensive health reporting
- ✅ LCD integration with status icons
- ✅ Configurable thresholds and behavior

**Result:** Robust, reliable LoRa communication with visibility into connection quality and automatic error recovery.
