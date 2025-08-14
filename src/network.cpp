#include "network.hpp"
#include "globals.hpp"
#include <FS.h> // File type on ESP8266

WebServerConfig::WebServerConfig() : server(80) {}

void NetworkConfig::ConfigWiFi(const char *ssid, const char *password)
{
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);

  Serial.print(F("\n[WIFI] Connexion en cours"));
  const unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 5000UL)
  {
    delay(500);
    Serial.print(F("."));
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println(F("\n[WIFI] Connecté ✅"));
    Serial.print(F("[INFO] Adresse IP: "));
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println(F("\n[ERREUR] Échec de connexion."));
  }
}

void WebServerConfig::begin()
{
  if (!LittleFS.begin())
  {
    Serial.println(F("[LITTLEFS] init failed!"));
    return;
  }

  // Redirect "/" to "/index.html" (no manual file streaming needed)
  server.on("/", HTTP_GET, [this]()
            {
    server.sendHeader("Location", "/index.html");
    server.send(302, "text/plain", ""); });

  // Serve ANY file under LittleFS with correct MIME types (images, CSS, JS, index.html, etc.)
  server.serveStatic("/", LittleFS, "/");

  // --- Minimal control API (only three actions) ---
  server.on("/espresso", [this]()
            {
    Serial.println(F("[HTTP] Espresso toggle"));
    servo.clickEspresso();                    // forward, pause, reverse
    server.send(200, "text/plain", "Espresso toggle"); });

  server.on("/jog_forward", [this]()
            {
    Serial.println(F("[HTTP] Jog forward"));
    servo.forwardMove(JOG_MOVE_MS);           // short forward nudge
    server.send(200, "text/plain", "Jog forward"); });

  server.on("/jog_reverse", [this]()
            {
    Serial.println(F("[HTTP] Jog reverse"));
    servo.reverseMove(JOG_MOVE_MS);           // short reverse nudge
    server.send(200, "text/plain", "Jog reverse"); });

  server.begin();
  Serial.println(F("[WEBSERVER] Web Server started!"));
}

void WebServerConfig::handleClient()
{
  server.handleClient();
}
