#include <WiFi.h>
#include "wifi_manager.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "variables.h"
#include "filesystem.h"
#include "authentication.h"
#include "pump_control.h"
#include "websocket.h"
#include "ui.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
unsigned long lastBroadcastTime = 0;

// Get session ID from Cookie
String getSessionIdFromCookie(AsyncWebServerRequest *request) {
  if (request->hasHeader("Cookie")) {
    String cookie = request->getHeader("Cookie")->value();
    int idx = cookie.indexOf("session=");
    if (idx >= 0) {
      int end = cookie.indexOf(";", idx);
      return cookie.substring(idx + 8, end > 0 ? end : cookie.length());
    }
  }
  return "";
}

bool ensureAuthenticated(AsyncWebServerRequest *request, String &username, String &role) {
  String sessionId = getSessionIdFromCookie(request);
  return isSessionValid(sessionId, username, role);
}

void setupRoutes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/pump");
  });

  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", String(COMMON_HEADER) + LOGIN_PAGE + COMMON_FOOTER);
  });

  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("username", true) && request->hasParam("password", true)) {
      String uname = request->getParam("username", true)->value();
      String pass = request->getParam("password", true)->value();
      String sessionId, role;
      if (authenticate(uname, pass, sessionId, role)) {
        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
        response->addHeader("Set-Cookie", "session=" + sessionId + "; Path=/");
        response->addHeader("Location", "/pump");
        request->send(response);
      } else {
        request->send(200, "text/html", String(COMMON_HEADER) + "<div class='alert'>Invalid credentials</div>" + LOGIN_PAGE + COMMON_FOOTER);
      }
    }
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    String sid = getSessionIdFromCookie(request);
    logoutSession(sid);
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
    response->addHeader("Set-Cookie", "session=; Max-Age=0; Path=/");
    response->addHeader("Location", "/login");
    request->send(response);
  });

  server.on("/pump", HTTP_GET, [](AsyncWebServerRequest *request){
    String user, role;
    if (!ensureAuthenticated(request, user, role)) {
      request->send(200, "text/html", String(COMMON_HEADER) + SESSION_EXPIRED_PAGE + COMMON_FOOTER);
      return;
    }
    String html = PUMP_CONTROL_PAGE;
    html.replace("%ADMIN_BUTTONS%", (role == "admin") ? "<a href='/usermgmt'><button>Users</button></a>" : "");
    request->send(200, "text/html", String(COMMON_HEADER) + html + COMMON_FOOTER);
  });

  server.on("/usermgmt", HTTP_GET, [](AsyncWebServerRequest *request){
    String user, role;
    if (!ensureAuthenticated(request, user, role) || role != "admin") {
      request->send(200, "text/html", String(COMMON_HEADER) + SESSION_EXPIRED_PAGE + COMMON_FOOTER);
      return;
    }

    File f = SPIFFS.open(USERS_FILE, "r");
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, f);
    f.close();

    String userOptions = "";
    for (JsonObject u : doc.as<JsonArray>()) {
      userOptions += "<option value='" + u["username"].as<String>() + "'>" + u["username"].as<String>() + "</option>";
    }

    String html = USER_MGMT_PAGE;
    html.replace("%USER_OPTIONS%", userOptions);

    request->send(200, "text/html", String(COMMON_HEADER) + html + COMMON_FOOTER);
  });

  server.on("/adduser", HTTP_POST, [](AsyncWebServerRequest *request){
    String user, role;
    if (!ensureAuthenticated(request, user, role) || role != "admin") {
      request->send(403, "text/plain", "Unauthorized");
      return;
    }

    String uname = request->getParam("username", true)->value();
    String pass = request->getParam("password", true)->value();
    String urole = request->getParam("role", true)->value();

    File f = SPIFFS.open(USERS_FILE, "r+");
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, f);
    f.close();

    JsonArray arr = doc.as<JsonArray>();
    JsonObject u = arr.createNestedObject();
    u["username"] = uname;
    u["password"] = hashPassword(pass);
    u["role"] = urole;

    f = SPIFFS.open(USERS_FILE, "w");
    serializeJsonPretty(doc, f);
    f.close();

    request->redirect("/usermgmt");
  });

  server.on("/deleteuser", HTTP_POST, [](AsyncWebServerRequest *request){
    String user, role;
    if (!ensureAuthenticated(request, user, role) || role != "admin") {
      request->send(403, "text/plain", "Unauthorized");
      return;
    }

    String uname = request->getParam("username", true)->value();

    File f = SPIFFS.open(USERS_FILE, "r");
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, f);
    f.close();

    JsonArray arr = doc.as<JsonArray>();
    for (int i = 0; i < arr.size(); i++) {
      if (arr[i]["username"] == uname) {
        arr.remove(i);
        break;
      }
    }

    f = SPIFFS.open(USERS_FILE, "w");
    serializeJsonPretty(doc, f);
    f.close();

    request->redirect("/usermgmt");
  });

  server.on("/changepassword", HTTP_POST, [](AsyncWebServerRequest *request){
    String user, role;
    if (!ensureAuthenticated(request, user, role) || role != "admin") {
      request->send(403, "text/plain", "Unauthorized");
      return;
    }

    String uname = request->getParam("username", true)->value();
    String pass = request->getParam("password", true)->value();

    File f = SPIFFS.open(USERS_FILE, "r");
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, f);
    f.close();

    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject u : arr) {
      if (u["username"] == uname) {
        u["password"] = hashPassword(pass);
        break;
      }
    }

    f = SPIFFS.open(USERS_FILE, "w");
    serializeJsonPretty(doc, f);
    f.close();

    request->redirect("/usermgmt");
  });

  server.on("/api/start", HTTP_POST, [](AsyncWebServerRequest *request){
    String user, role;
    if (!ensureAuthenticated(request, user, role)) {
      request->send(403, "text/plain", "Unauthorized");
      return;
    }

    unsigned long duration = request->getParam("duration", true)->value().toInt();

    if (!startPump(duration)) {
      request->send(400, "text/plain", "Pump already running !!!");
      return;
    }   
      broadcastPumpStatus(ws, pumpRunning, getRemainingTime());
      request->send(200, "text/plain", "Pump started");
  });

  server.on("/api/stop", HTTP_POST, [](AsyncWebServerRequest *request){
    String user, role;
    if (!ensureAuthenticated(request, user, role)) {
      request->send(403, "text/plain", "Unauthorized");
      return;
    }
    stopPump();
    broadcastPumpStatus(ws, pumpRunning, getRemainingTime());
    request->send(200, "text/plain", "Pump stopped");
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/html", String(COMMON_HEADER) + NOT_FOUND_PAGE + COMMON_FOOTER);
  });
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    broadcastPumpStatus(ws, pumpRunning, getRemainingTime());
  }
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  
  initializeSPIFFS();
  setupPumpControl();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  setupRoutes();
  server.begin();
}

void loop() {
  checkWiFiConnection();  // ⬅️ Auto-reconnect if disconnected
  ws.cleanupClients();
  updatePumpState();

  if (pumpRunning && millis() - lastBroadcastTime >= 1000) {
    broadcastPumpStatus(ws, pumpRunning, getRemainingTime());
    lastBroadcastTime = millis();
  }

}