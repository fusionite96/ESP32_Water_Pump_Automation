#ifndef VARIABLES_H
#define VARIABLES_H

// -------------------- WiFi Configuration --------------------
const char* WIFI_SSID = "ssid";
const char* WIFI_PASSWORD = "password";

// -------------------- Pump Relay GPIO --------------------
const int PUMP_RELAY_PIN = 5;  // Change based on your board

// Set to true to interpret input duration as minutes, false for seconds
const bool INPUT_IN_MINUTES = true;

// -------------------- Timer Configuration (in seconds) --------------------
const unsigned long DEFAULT_PUMP_TIMER = 1200; // 20 minutes
const unsigned long MAX_PUMP_TIMER = 1800;     // 30 minutes failsafe
const unsigned long SESSION_TIMEOUT = 180;     // 3 minutes inactivity

// -------------------- SPIFFS Paths --------------------
const char* USERS_FILE = "/users.json";
const char* STATE_FILE = "/state.json";  // For power failure recovery

// -------------------- Session Configuration --------------------
const int MAX_SESSIONS = 10;  // Max concurrent sessions allowed

// -------------------- Default Users (for first boot only) --------------------
const int NUM_DEFAULT_USERS = 2;

struct DefaultUser {
  const char* username;
  const char* password;
  const char* role; // "admin" or "user"
};

// You can change these default users as needed
DefaultUser defaultUsers[NUM_DEFAULT_USERS] = {
  { "admin", "admin123", "admin" },
  { "user1", "password", "user" }
};

#endif