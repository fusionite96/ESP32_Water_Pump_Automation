#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "variables.h"
#include "mbedtls/sha256.h"

struct Session {
  String sessionId;
  String username;
  String role;
  unsigned long lastActive;
  bool active;
};

Session sessions[MAX_SESSIONS];

// Hash password using mbedTLS SHA-256 (ESP32 native crypto)
String hashPassword(const String& password) {
  byte shaResult[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);

#if defined(mbedtls_sha256_starts_ret)
  mbedtls_sha256_starts_ret(&ctx, 0);
  mbedtls_sha256_update_ret(&ctx, (const unsigned char*)password.c_str(), password.length());
  mbedtls_sha256_finish_ret(&ctx, shaResult);
#else
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update(&ctx, (const unsigned char*)password.c_str(), password.length());
  mbedtls_sha256_finish(&ctx, shaResult);
#endif

  mbedtls_sha256_free(&ctx);

  String hashed = "";
  for (int i = 0; i < 32; i++) {
    if (shaResult[i] < 16) hashed += "0";
    hashed += String(shaResult[i], HEX);
  }
  return hashed;
}

// Convert string to lowercase (for case-insensitive usernames)
String toLowerCase(const String& str) {
  String lower = str;
  lower.toLowerCase();
  return lower;
}

// Generate random session ID
String generateSessionId() {
  String id = "";
  for (int i = 0; i < 16; i++) id += char(random(97, 123));
  return id;
}

// Load user from SPIFFS
bool loadUser(const String& uname, String& hashedPass, String& roleOut) {
  if (!SPIFFS.exists(USERS_FILE)) return false;

  File file = SPIFFS.open(USERS_FILE, "r");
  if (!file) return false;

  StaticJsonDocument<2048> doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return false;

  String unameLower = toLowerCase(uname);
  for (JsonObject user : doc.as<JsonArray>()) {
    if (toLowerCase(user["username"].as<String>()) == unameLower) {
      hashedPass = user["password"].as<String>();
      roleOut = user["role"].as<String>();
      return true;
    }
  }
  return false;
}

// Validate login and create session
bool authenticate(const String& username, const String& password, String& sessionId, String& roleOut) {
  String storedHash, role;
  if (!loadUser(username, storedHash, role)) return false;

  if (hashPassword(password) != storedHash) return false;

  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (!sessions[i].active) {
      sessions[i].sessionId = generateSessionId();
      sessions[i].username = toLowerCase(username);
      sessions[i].role = role;
      sessions[i].lastActive = millis();
      sessions[i].active = true;
      sessionId = sessions[i].sessionId;
      roleOut = role;
      return true;
    }
  }
  return false;
}

// Validate session
bool isSessionValid(const String& sessionId, String& usernameOut, String& roleOut) {
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (sessions[i].active && sessions[i].sessionId == sessionId) {
      if ((millis() - sessions[i].lastActive) > (SESSION_TIMEOUT * 1000)) {
        sessions[i].active = false;
        return false;
      }
      sessions[i].lastActive = millis();
      usernameOut = sessions[i].username;
      roleOut = sessions[i].role;
      return true;
    }
  }
  return false;
}

// Logout session
bool logoutSession(const String& sessionId) {
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (sessions[i].active && sessions[i].sessionId == sessionId) {
      sessions[i].active = false;
      return true;
    }
  }
  return false;
}

#endif