#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "variables.h"
#include "authentication.h"

bool initializeSPIFFS() {
    if (!SPIFFS.begin(true)) {
      Serial.println("Failed to mount SPIFFS.");
      return false;
    }
  
    // -------------------- Create Default Users --------------------
    if (!SPIFFS.exists(USERS_FILE)) {
      Serial.println("User file missing. Creating default users...");
      File file = SPIFFS.open(USERS_FILE, FILE_WRITE);
      if (!file) {
        Serial.println("Failed to create users file.");
        return false;
      }
  
      StaticJsonDocument<2048> doc;
      JsonArray arr = doc.to<JsonArray>();
  
      for (int i = 0; i < NUM_DEFAULT_USERS; i++) {
        JsonObject user = arr.createNestedObject();
        user["username"] = defaultUsers[i].username;
        user["password"] = hashPassword(defaultUsers[i].password);
        user["role"] = defaultUsers[i].role;
      }
  
      if (serializeJsonPretty(doc, file) == 0) {
        Serial.println("Failed to write user data.");
        file.close();
        return false;
      }
      file.close();
      Serial.println("Default users created.");
    }
  
    // -------------------- Create Default Pump State --------------------
    if (!SPIFFS.exists(STATE_FILE)) {
      Serial.println("Pump state file missing. Creating default state...");
      File stateFile = SPIFFS.open(STATE_FILE, FILE_WRITE);
      if (!stateFile) {
        Serial.println("Failed to create state file.");
        return false;
      }
  
      StaticJsonDocument<256> stateDoc;
      stateDoc["running"] = false;
      stateDoc["start"] = 0;
      stateDoc["duration"] = 0;
  
      if (serializeJson(stateDoc, stateFile) == 0) {
        Serial.println("Failed to write default pump state.");
        stateFile.close();
        return false;
      }
  
      stateFile.close();
      Serial.println("Default pump state created.");
    }
  
    return true;
  }  
#endif