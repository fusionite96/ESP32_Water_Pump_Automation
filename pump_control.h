#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "variables.h"
#include "websocket.h"


extern AsyncWebSocket ws;

bool pumpRunning = false;
unsigned long pumpStartTime = 0;
unsigned long pumpDuration = 0; // in seconds
unsigned long pumpEndTime = 0;

void savePumpState() {
  StaticJsonDocument<256> doc;
  doc["running"] = pumpRunning;
  doc["start"] = pumpStartTime;
  doc["duration"] = pumpDuration;

  File file = SPIFFS.open(STATE_FILE, FILE_WRITE);
  if (file) {
    serializeJson(doc, file);
    file.close();
  }
}

void loadPumpState() {
  if (!SPIFFS.exists(STATE_FILE)) return;

  File file = SPIFFS.open(STATE_FILE, FILE_READ);
  if (!file) return;

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) return;

  pumpRunning = doc["running"] | false;
  pumpStartTime = doc["start"] | 0;
  pumpDuration = doc["duration"] | 0;

  if (pumpRunning) {
    unsigned long now = millis();
    unsigned long elapsed = (now - pumpStartTime) / 1000;
    if (elapsed < pumpDuration && elapsed < MAX_PUMP_TIMER) {
      pumpEndTime = pumpStartTime + pumpDuration * 1000;
      digitalWrite(PUMP_RELAY_PIN, HIGH);
    } else {
      pumpRunning = false;
      digitalWrite(PUMP_RELAY_PIN, LOW);
    }
  }
}

unsigned long getRemainingTime() {
  if (!pumpRunning) return 0;

  long remaining = (pumpEndTime - millis()) / 1000;
  return remaining > 0 ? remaining : 0;
}

void stopPump() {
  digitalWrite(PUMP_RELAY_PIN, LOW);
  pumpRunning = false;
  pumpStartTime = 0;
  pumpDuration = 0;
  pumpEndTime = 0;
  savePumpState();
  broadcastPumpStatus(ws, pumpRunning, getRemainingTime()); // Broadcast updated status
}

bool startPump(unsigned long durationInput) {
  if (pumpRunning) return false;
  
  if (INPUT_IN_MINUTES) {
    durationInput *= 60;
  }

  if (durationInput == 0) durationInput = DEFAULT_PUMP_TIMER;
  if (durationInput > MAX_PUMP_TIMER) return false;

  pumpRunning = true;
  pumpStartTime = millis();
  pumpDuration = durationInput;
  pumpEndTime = pumpStartTime + pumpDuration * 1000;
  digitalWrite(PUMP_RELAY_PIN, HIGH);
  savePumpState();
  broadcastPumpStatus(ws, pumpRunning, getRemainingTime());
  return true;
}

void updatePumpState() {
  if (pumpRunning && millis() > pumpEndTime) {
    stopPump(); 
    broadcastPumpStatus(ws, pumpRunning, getRemainingTime()); // Notify clients of change
  }
}

void setupPumpControl() {
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, LOW);
  loadPumpState(); // Restore state after power reboot
}

#endif