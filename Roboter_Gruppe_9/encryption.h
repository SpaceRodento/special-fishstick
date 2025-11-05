/*=====================================================================
  encryption.h - Simple XOR Encryption

  FEATURE 7: Data Encryption

  Encrypts LoRa payload using XOR cipher for basic privacy.

  ⚠️  IMPORTANT SECURITY NOTICE:
  - XOR encryption is NOT cryptographically secure!
  - Suitable for basic obfuscation only
  - Do NOT use for sensitive data (passwords, personal info)
  - Can be broken easily with frequency analysis
  - For real security, use AES (more complex, not implemented here)

  How XOR Encryption Works:
  - Each byte is XOR'd with encryption key
  - Encryption: cipher = plaintext XOR key
  - Decryption: plaintext = cipher XOR key (same operation!)
  - Symmetric: Same key for encrypt and decrypt

  Example:
  - Plaintext: "LED:1" = 0x4C 0x45 0x44 0x3A 0x31
  - Key: 0xA5
  - Encrypted: 0xE9 0xE0 0xE1 0x9F 0x94
  - Decrypt with 0xA5 → back to "LED:1"

  Use Cases:
  - Prevent casual eavesdropping (neighbors with LoRa receiver)
  - Obfuscate commands (harder to reverse engineer)
  - Privacy for non-critical data
  - Quick and lightweight (no CPU overhead)

  Configuration:
  - ENCRYPTION_KEY: 0x00-0xFF (0xA5 default)
  - Both sender and receiver MUST use same key!
  - Change key periodically for better security

  Testing:
  1. Set ENABLE_ENCRYPTION true in config.h
  2. Set same ENCRYPTION_KEY on both devices
  3. Upload to both
  4. Messages still work (encrypted over-the-air)
  5. Check serial: Should show encrypted payload in hex

  Key Management:
  - Store key in config.h (not ideal for production)
  - For better security: Store in EEPROM/Flash
  - For best security: Use AES-128 or AES-256

  Performance:
  - Very fast (<1ms for typical payload)
  - No noticeable delay
  - Works at any spreading factor
=======================================================================*/

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <Arduino.h>
#include "config.h"

// Encryption statistics
struct EncryptionStats {
  unsigned long messagesEncrypted;
  unsigned long messagesDecrypted;
  unsigned long encryptionErrors;
  uint8_t currentKey;
  bool isEnabled;
};

EncryptionStats encStats = {0, 0, 0, 0, false};

// Initialize encryption system
void initEncryption() {
  #if ENABLE_ENCRYPTION
    encStats.isEnabled = true;
    encStats.currentKey = ENCRYPTION_KEY;

    Serial.println("🔒 Encryption enabled");
    Serial.print("  Algorithm: XOR cipher");
    Serial.println();
    Serial.print("  Key: 0x");
    Serial.println(ENCRYPTION_KEY, HEX);
    Serial.println("  ⚠️  WARNING: XOR is NOT cryptographically secure!");
    Serial.println("  Use for basic obfuscation only");
    Serial.println("  Both devices MUST use same key!");
  #else
    encStats.isEnabled = false;
  #endif
}

// Encrypt string using XOR cipher
String encryptXOR(String plaintext, uint8_t key) {
  #if ENABLE_ENCRYPTION
    String encrypted = "";
    encrypted.reserve(plaintext.length());  // Pre-allocate memory

    for (int i = 0; i < plaintext.length(); i++) {
      // XOR each character with key
      char encryptedChar = plaintext[i] ^ key;
      encrypted += encryptedChar;
    }

    encStats.messagesEncrypted++;
    return encrypted;
  #else
    // Encryption disabled, return plaintext
    return plaintext;
  #endif
}

// Decrypt string using XOR cipher (same as encrypt!)
String decryptXOR(String ciphertext, uint8_t key) {
  #if ENABLE_ENCRYPTION
    // XOR decryption is identical to encryption
    String decrypted = "";
    decrypted.reserve(ciphertext.length());

    for (int i = 0; i < ciphertext.length(); i++) {
      char decryptedChar = ciphertext[i] ^ key;
      decrypted += decryptedChar;
    }

    encStats.messagesDecrypted++;
    return decrypted;
  #else
    // Encryption disabled, return ciphertext as-is
    return ciphertext;
  #endif
}

// Encrypt payload before sending
String encryptPayload(String payload) {
  #if ENABLE_ENCRYPTION
    return encryptXOR(payload, ENCRYPTION_KEY);
  #else
    return payload;
  #endif
}

// Decrypt received payload
String decryptPayload(String payload) {
  #if ENABLE_ENCRYPTION
    return decryptXOR(payload, ENCRYPTION_KEY);
  #else
    return payload;
  #endif
}

// Convert string to hex for debugging
String toHex(String str) {
  String hex = "";
  for (int i = 0; i < str.length(); i++) {
    char buffer[4];
    sprintf(buffer, "%02X ", (unsigned char)str[i]);
    hex += buffer;
  }
  return hex;
}

// Print encryption statistics
void printEncryptionStats() {
  #if ENABLE_ENCRYPTION
    Serial.println("\n╔════════ ENCRYPTION STATISTICS ════════╗");
    Serial.print("║ Status:          ");
    Serial.println(encStats.isEnabled ? "ENABLED 🔒" : "DISABLED");

    if (encStats.isEnabled) {
      Serial.print("║ Algorithm:       XOR cipher");
      Serial.println();
      Serial.print("║ Key:             0x");
      Serial.println(encStats.currentKey, HEX);
      Serial.print("║ Encrypted:       ");
      Serial.println(encStats.messagesEncrypted);
      Serial.print("║ Decrypted:       ");
      Serial.println(encStats.messagesDecrypted);
      Serial.print("║ Errors:          ");
      Serial.println(encStats.encryptionErrors);
    }

    Serial.println("╚═══════════════════════════════════════╝\n");
  #endif
}

// Test encryption (verify it's working)
void testEncryption() {
  #if ENABLE_ENCRYPTION
    Serial.println("\n🔒 Testing encryption...");

    // Test string
    String original = "LED:1,TEMP:25.5";
    Serial.print("Original:  ");
    Serial.println(original);
    Serial.print("Hex:       ");
    Serial.println(toHex(original));

    // Encrypt
    String encrypted = encryptPayload(original);
    Serial.print("Encrypted: ");
    Serial.println(encrypted);
    Serial.print("Hex:       ");
    Serial.println(toHex(encrypted));

    // Decrypt
    String decrypted = decryptPayload(encrypted);
    Serial.print("Decrypted: ");
    Serial.println(decrypted);

    // Verify
    if (original == decrypted) {
      Serial.println("✓ Encryption test PASSED");
    } else {
      Serial.println("❌ Encryption test FAILED!");
      Serial.print("Expected: ");
      Serial.println(original);
      Serial.print("Got:      ");
      Serial.println(decrypted);
    }
  #else
    Serial.println("⚠️  Encryption is disabled");
  #endif
}

// Change encryption key at runtime (use carefully!)
void setEncryptionKey(uint8_t newKey) {
  #if ENABLE_ENCRYPTION
    Serial.print("🔑 Changing encryption key: 0x");
    Serial.print(encStats.currentKey, HEX);
    Serial.print(" → 0x");
    Serial.println(newKey, HEX);

    encStats.currentKey = newKey;

    Serial.println("⚠️  WARNING: Both devices must use same key!");
    Serial.println("⚠️  Old messages cannot be decrypted with new key!");
  #endif
}

// Get current encryption key
uint8_t getEncryptionKey() {
  #if ENABLE_ENCRYPTION
    return encStats.currentKey;
  #else
    return 0;
  #endif
}

// Check if encryption is enabled
bool isEncryptionEnabled() {
  return encStats.isEnabled;
}

// Encrypt with visual feedback (for debugging)
String encryptWithDebug(String plaintext) {
  #if ENABLE_ENCRYPTION
    Serial.println("┌─ ENCRYPTION DEBUG ─");
    Serial.print("│ Plaintext:  ");
    Serial.println(plaintext);
    Serial.print("│ Length:     ");
    Serial.println(plaintext.length());
    Serial.print("│ Key:        0x");
    Serial.println(ENCRYPTION_KEY, HEX);

    String encrypted = encryptPayload(plaintext);

    Serial.print("│ Ciphertext: ");
    Serial.println(encrypted);
    Serial.print("│ Hex:        ");
    Serial.println(toHex(encrypted));
    Serial.println("└────────────────────");

    return encrypted;
  #else
    return plaintext;
  #endif
}

// Decrypt with visual feedback (for debugging)
String decryptWithDebug(String ciphertext) {
  #if ENABLE_ENCRYPTION
    Serial.println("┌─ DECRYPTION DEBUG ─");
    Serial.print("│ Ciphertext: ");
    Serial.println(ciphertext);
    Serial.print("│ Hex:        ");
    Serial.println(toHex(ciphertext));
    Serial.print("│ Key:        0x");
    Serial.println(ENCRYPTION_KEY, HEX);

    String decrypted = decryptPayload(ciphertext);

    Serial.print("│ Plaintext:  ");
    Serial.println(decrypted);
    Serial.print("│ Length:     ");
    Serial.println(decrypted.length());
    Serial.println("└────────────────────");

    return decrypted;
  #else
    return ciphertext;
  #endif
}

#endif // ENCRYPTION_H
