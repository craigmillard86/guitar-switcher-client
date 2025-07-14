#include <Arduino.h>
#include <WiFiManager.h>
#include <ElegantOTA.h>
#include <globals.h>
#include "utils.h"

WebServer server(80);

// === Start OTA and WiFiManager ===
void startOTA() {
  log(LOG_INFO, "=== Starting OTA Setup Mode ===");
  WiFiManager wm;

  if (!wm.autoConnect("OTA_Config_Portal")) {
    log(LOG_ERROR, "Failed to connect to WiFi during OTA setup");
    ESP.restart();
  }

  log(LOG_INFO, "WiFi connected during OTA setup");
  log(LOG_INFO, "IP Address: " + WiFi.localIP().toString());
  
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
    log(LOG_INFO, "Reboot requested via web interface");
    delay(1000);
    ESP.restart();
  });

  ElegantOTA.begin(&server);
  server.begin();
  log(LOG_INFO, "Web server started for OTA updates");

  unsigned long start = millis();
  const unsigned long TIMEOUT = 5 * 60 * 1000; // 5 min
  log(LOG_INFO, "OTA mode active for 5 minutes");
  
  while (millis() - start < TIMEOUT) {
    server.handleClient();
    delay(10);
  }

  log(LOG_WARN, "OTA timeout reached, rebooting...");
  ESP.restart();
}