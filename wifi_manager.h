#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "variables.h"

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retryCount++;
    if (retryCount % 20 == 0) Serial.println(); // newline every 10 seconds
  }

  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    connectToWiFi();
  }
}
#endif