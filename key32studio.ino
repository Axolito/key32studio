/*
 * Bitfocus Companion Controller - ESP32
 * 4x MX Switch Panel — Pins: 32, 33, 25, 26
 * Protocole : HTTP REST API Companion v3+
 *
 * Dependances Arduino :
 *   - ArduinoJson (v6) -> Gestionnaire de bibliotheques
 *   - WiFi, WebServer, Preferences, HTTPClient -> inclus dans le core ESP32
 */

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "secrets.h"

#include "index_html.h"

// === Configuration Companion par defaut =======================================
String companion_ip   = "192.168.1.100";
int    companion_port = 8888;

// === Pins des touches =========================================================
const int NUM_KEYS   = 4;
const int KEY_PINS[] = {32, 33, 25, 26};

// === Configuration par touche =================================================
struct KeyConfig {
  String label;
  int    page;
  int    row;
  int    col;
  bool   enabled;
};

KeyConfig keys[NUM_KEYS] = {
  {"Key 1", 1, 0, 0, true},
  {"Key 2", 1, 0, 1, true},
  {"Key 3", 1, 0, 2, true},
  {"Key 4", 1, 0, 3, true},
};

// === Etat des boutons =========================================================
bool lastKeyState[NUM_KEYS];
bool currentKeyState[NUM_KEYS];
unsigned long lastDebounce[NUM_KEYS];
const unsigned long DEBOUNCE_MS = 50;

// === Serveur Web ==============================================================
WebServer server(80);
Preferences prefs;


// === Prototypes ===============================================================
void companionPost(int keyIndex, const char* action);
void loadConfig();
void saveConfig();
void setupRoutes();


// ==============================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Companion Controller ===");

  for (int i = 0; i < NUM_KEYS; i++) {
    pinMode(KEY_PINS[i], INPUT_PULLUP);
    lastKeyState[i]    = HIGH;
    currentKeyState[i] = HIGH;
    lastDebounce[i]    = 0;
  }

  loadConfig();

  Serial.printf("Connexion a %s ...\n", wifi_ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nConnecte ! IP : %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nEchec WiFi -> mode Access Point");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Key32Studio", "key32studio");
    Serial.printf("AP IP : %s\n", WiFi.softAPIP().toString().c_str());
  }

  setupRoutes();
  server.begin();
  Serial.println("Serveur web demarre");
}

// ==============================================================================
void loop() {
  server.handleClient();
  scanKeys();
}

// === Lecture & debounce des touches ===========================================
void scanKeys() {
  unsigned long now = millis();
  for (int i = 0; i < NUM_KEYS; i++) {
    bool reading = digitalRead(KEY_PINS[i]);

    if (reading != lastKeyState[i]) lastDebounce[i] = now;

    if ((now - lastDebounce[i]) > DEBOUNCE_MS) {
      if (reading != currentKeyState[i]) {
        currentKeyState[i] = reading;
        if (!keys[i].enabled) continue;

        if (currentKeyState[i] == LOW) {
          Serial.printf("-> Key %d DOWN (%s)\n", i + 1, keys[i].label.c_str());
          companionPost(i, "down");
        } else {
          Serial.printf("-> Key %d UP   (%s)\n", i + 1, keys[i].label.c_str());
          companionPost(i, "up");
        }
      }
    }
    lastKeyState[i] = reading;
  }
}

// === Envoi HTTP vers Companion ================================================
void companionPost(int keyIndex, const char* action) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("  WiFi non connecte, envoi ignore");
    return;
  }

  String url = "http://" + companion_ip + ":" + companion_port
             + "/api/location/"
             + keys[keyIndex].page + "/"
             + keys[keyIndex].row  + "/"
             + keys[keyIndex].col  + "/"
             + action;

  Serial.printf("  POST %s\n", url.c_str());

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST("");
  Serial.printf("  -> HTTP %d\n", code);
  http.end();
}

// === Persistance NVS ==========================================================
void loadConfig() {
  prefs.begin("k32s", false);
  wifi_ssid      = prefs.getString("wifi_ssid", wifi_ssid);
  wifi_password  = prefs.getString("wifi_pass", wifi_password);
  companion_ip   = prefs.getString("comp_ip",   companion_ip);
  companion_port = prefs.getInt   ("comp_port", companion_port);
  for (int i = 0; i < NUM_KEYS; i++) {
    String p = "k" + String(i) + "_";
    keys[i].label   = prefs.getString((p + "label").c_str(), keys[i].label);
    keys[i].page    = prefs.getInt   ((p + "page").c_str(),  keys[i].page);
    keys[i].row     = prefs.getInt   ((p + "row").c_str(),   keys[i].row);
    keys[i].col     = prefs.getInt   ((p + "col").c_str(),   keys[i].col);
    keys[i].enabled = prefs.getBool  ((p + "en").c_str(),    keys[i].enabled);
  }
  prefs.end();
  Serial.println("Config chargee depuis NVS");
}

void saveConfig() {
  prefs.begin("k32s", false);
  prefs.putString("wifi_ssid", wifi_ssid);
  prefs.putString("wifi_pass", wifi_password);
  prefs.putString("comp_ip",   companion_ip);
  prefs.putInt   ("comp_port", companion_port);
  for (int i = 0; i < NUM_KEYS; i++) {
    String p = "k" + String(i) + "_";
    prefs.putString((p + "label").c_str(), keys[i].label);
    prefs.putInt   ((p + "page").c_str(),  keys[i].page);
    prefs.putInt   ((p + "row").c_str(),   keys[i].row);
    prefs.putInt   ((p + "col").c_str(),   keys[i].col);
    prefs.putBool  ((p + "en").c_str(),    keys[i].enabled);
  }
  prefs.end();
  Serial.println("Config sauvegardee dans NVS");
}

// === Routes HTTP ==============================================================
void setupRoutes() {

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  // GET /api/config
  server.on("/api/config", HTTP_GET, []() {
    StaticJsonDocument<1024> doc;
    doc["wifi_ssid"]      = wifi_ssid;
    doc["wifi_password"]  = "";
    doc["companion_ip"]   = companion_ip;
    doc["companion_port"] = companion_port;
    JsonArray arr = doc.createNestedArray("keys");
    for (int i = 0; i < NUM_KEYS; i++) {
      JsonObject k = arr.createNestedObject();
      k["label"]   = keys[i].label;
      k["page"]    = keys[i].page;
      k["row"]     = keys[i].row;
      k["col"]     = keys[i].col;
      k["enabled"] = keys[i].enabled;
      k["pin"]     = KEY_PINS[i];
    }
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
  });

  // POST /api/config
  server.on("/api/config", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"no body\"}");
      return;
    }
    StaticJsonDocument<1024> doc;
    if (deserializeJson(doc, server.arg("plain"))) {
      server.send(400, "application/json", "{\"error\":\"json invalide\"}");
      return;
    }
    if (doc.containsKey("wifi_ssid"))    wifi_ssid    = doc["wifi_ssid"].as<String>();
    if (doc.containsKey("wifi_password") && doc["wifi_password"].as<String>().length() > 0)
                                         wifi_password = doc["wifi_password"].as<String>();
    if (doc.containsKey("companion_ip")) companion_ip  = doc["companion_ip"].as<String>();
    if (doc.containsKey("companion_port")) companion_port = doc["companion_port"].as<int>();
    if (doc.containsKey("keys")) {
      int i = 0;
      for (JsonObject k : doc["keys"].as<JsonArray>()) {
        if (i >= NUM_KEYS) break;
        if (k.containsKey("label"))   keys[i].label   = k["label"].as<String>();
        if (k.containsKey("page"))    keys[i].page    = k["page"].as<int>();
        if (k.containsKey("row"))     keys[i].row     = k["row"].as<int>();
        if (k.containsKey("col"))     keys[i].col     = k["col"].as<int>();
        if (k.containsKey("enabled")) keys[i].enabled = k["enabled"].as<bool>();
        i++;
      }
    }
    saveConfig();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  });

  // POST /api/test  {"key": 0}
  server.on("/api/test", HTTP_POST, []() {
    if (!server.hasArg("plain")) { server.send(400); return; }
    StaticJsonDocument<64> doc;
    deserializeJson(doc, server.arg("plain"));
    int idx = doc["key"] | -1;
    if (idx < 0 || idx >= NUM_KEYS) {
      server.send(400, "application/json", "{\"error\":\"index invalide\"}");
      return;
    }
    companionPost(idx, "press");
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  });

  // POST /api/reboot
  server.on("/api/reboot", HTTP_POST, []() {
    server.send(200, "application/json", "{\"status\":\"reboot\"}");
    delay(500);
    ESP.restart();
  });
}
