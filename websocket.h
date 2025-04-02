#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "pump_control.h"

// Declare externally (defined in main .ino file)
extern AsyncWebSocket ws;

// Broadcast pump status to all connected WebSocket clients
void broadcastPumpStatus(AsyncWebSocket& wsRef, bool running, unsigned long remaining) {
  StaticJsonDocument<128> doc;
  doc["status"] = running ? "running" : "stopped";
  doc["remaining"] = remaining;

  String msg;
  serializeJson(doc, msg);
  wsRef.textAll(msg);
}

#endif