#include <Arduino.h>
#include <WiFiManager.h>
#include <ElegantOTA.h>
#include <globals.h>

WebServer server(80);

// === Check if OTA mode should start ===
bool checkOtaTrigger() {
  pinMode(OTA_BUTTON_PIN, INPUT_PULLUP);
  unsigned long pressStart = millis();
  while (digitalRead(OTA_BUTTON_PIN) == LOW) {
    if (millis() - pressStart >= OTA_HOLD_TIME) {
      return true;
    }
    delay(10);
  }
  return false;
}

// === Start OTA and WiFiManager ===
// === Start OTA and WiFiManager ===
void startOTA() {
  Serial.println("\n[OTA] Entering OTA Setup Mode...");
  WiFiManager wm;

  if (!wm.autoConnect("OTA_Config_Portal")) {
    Serial.println("[OTA] Failed to connect");
    ESP.restart();
  }

  Serial.print("[OTA] Connected, IP: ");
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, []() {
    String html = "<html><head><style>body{font-family:sans-serif;text-align:center;padding:2em;}h1{color:#333;}p{margin:1em 0;}a,input[type=submit]{padding:0.5em 1em;background:#007bff;color:#fff;border:none;border-radius:5px;}a:hover,input[type=submit]:hover{background:#0056b3;}</style></head><body>";
    html += "<h1>ESP32 OTA Ready</h1>";
    html += "<p><b>Firmware Version:</b> " FIRMWARE_VERSION "</p>";
    html += "<p><b>IP:</b> " + WiFi.localIP().toString() + "</p>";
    html += "<p><a href='/update'>Go to OTA Update</a></p>";
    html += "<form action='/reboot' method='POST'><input type='submit' value='Reboot ESP32'></form>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/reboot", HTTP_POST, []() {
    server.send(200, "text/plain", "Rebooting...");
    delay(1000);
    ESP.restart();
  });

  ElegantOTA.begin(&server);
  server.begin();
  Serial.println("[OTA] Web Server started");

  unsigned long start = millis();
  const unsigned long TIMEOUT = 5 * 60 * 1000; // 5 min
  while (millis() - start < TIMEOUT) {
    server.handleClient();
    delay(10);
  }

  Serial.println("[OTA] Timeout. Rebooting...");
  ESP.restart();
}