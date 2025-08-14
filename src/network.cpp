#include "network.hpp"
#include "globals.hpp"
#include <FS.h> // File type on ESP8266

// Upload state + 256 KB cap
static File upFile;
static String upTarget; // final path (/bg_user.webp or /bg_user.jpg)
static size_t upSize = 0;
static const size_t BG_MAX_BYTES = 256 * 1024;

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

  // --- 1) Dynamic control endpoints FIRST ---
  server.on("/espresso", HTTP_GET, [this]()
            {
    Serial.println(F("[HTTP] /espresso"));
    servo.clickEspresso();
    server.send(200, "text/plain", "Espresso toggle"); });

  server.on("/jog_forward", HTTP_GET, [this]()
            {
    Serial.println(F("[HTTP] /jog_forward"));
    servo.forwardMove(JOG_MOVE_MS);
    server.send(200, "text/plain", "Jog forward"); });

  server.on("/jog_reverse", HTTP_GET, [this]()
            {
    Serial.println(F("[HTTP] /jog_reverse"));
    servo.reverseMove(JOG_MOVE_MS);
    server.send(200, "text/plain", "Jog reverse"); });

  // --- 2) Background endpoint: GET /bg returns user image or default ---
  server.on("/bg", HTTP_GET, [this]()
            {
    String path, ctype;

    // Prefer user-uploaded file
    if (LittleFS.exists("/bg_user.webp"))      { path = "/bg_user.webp"; ctype = "image/webp"; }
    else if (LittleFS.exists("/bg_user.jpg"))  { path = "/bg_user.jpg";  ctype = "image/jpeg"; }
    // Fallback to bundled defaults
    else if (LittleFS.exists("/bg.webp"))      { path = "/bg.webp";      ctype = "image/webp"; }
    else if (LittleFS.exists("/bg.jpg"))       { path = "/bg.jpg";       ctype = "image/jpeg"; }
    else { server.send(404, "text/plain", "bg not found"); return; }

    File f = LittleFS.open(path, "r");
    server.sendHeader("Cache-Control", "no-store"); // reflect updates immediately
    server.streamFile(f, ctype);
    f.close(); });

  // --- 3) Upload endpoint: POST /upload_bg (multipart/form-data) ---
  server.on("/upload_bg", HTTP_POST,
            // finished
            [this]()
            {
    if (upFile) upFile.close();

    // No file / wrong type
    if (upTarget.isEmpty()) {
      LittleFS.remove("/bg_user.tmp");
      server.send(415, "text/plain", "Unsupported type. Use .webp or .jpg");
      upSize = 0; return;
    }

    // Too large
    if (upSize > BG_MAX_BYTES) {
      LittleFS.remove("/bg_user.tmp");
      server.send(413, "text/plain", "File too large. Max 256 KB.");
      upSize = 0; upTarget = ""; return;
    }

    // Success: atomically replace existing
    if (LittleFS.exists(upTarget)) LittleFS.remove(upTarget);
    LittleFS.rename("/bg_user.tmp", upTarget);
    server.send(200, "text/plain", "Background updated (" + String(upSize) + " bytes).");

    // reset
    upSize = 0; upTarget = ""; },
            // chunked upload
            [this]()
            {
    HTTPUpload& u = server.upload();

    if (u.status == UPLOAD_FILE_START) {
      upSize = 0;
      upTarget = "";  // unknown until we inspect extension
      String fn = u.filename; fn.toLowerCase();

      if (fn.endsWith(".webp"))      upTarget = "/bg_user.webp";
      else if (fn.endsWith(".jpg") || fn.endsWith(".jpeg"))
                                   upTarget = "/bg_user.jpg";
      // open temp only if type ok
      if (!upTarget.isEmpty()) upFile = LittleFS.open("/bg_user.tmp", "w");
    }
    else if (u.status == UPLOAD_FILE_WRITE) {
      upSize += u.currentSize;                         // track total size
      if (upFile && upSize <= BG_MAX_BYTES) {
        upFile.write(u.buf, u.currentSize);            // write while under cap
      }
      // (if over cap, we just stop writing; finalizer will delete tmp)
    }
    else if (u.status == UPLOAD_FILE_END) {
      if (upFile) upFile.close();
    } });

  // --- 4) Serve index.html with no-cache (so UI edits show immediately) ---
  server.on("/", HTTP_GET, [this]()
            {
    File f = LittleFS.open("/index.html", "r");
    if (!f) { server.send(404, "text/plain", "index.html not found"); return; }
    server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "0");
    server.streamFile(f, "text/html");
    f.close(); });

  // --- 5) Catch-all for other static files (CSS/JS/images) ---
  server.serveStatic("/", LittleFS, "/"); // keep LAST so it doesn't shadow dynamic routes

  // Optional: debug
  server.onNotFound([this]()
                    {
    Serial.print(F("[404] ")); Serial.println(server.uri());
    server.send(404, "text/plain", "Not found: " + server.uri()); });

  server.begin();
  Serial.println(F("[WEBSERVER] Web Server started!"));
}

void WebServerConfig::handleClient()
{
  server.handleClient();
}
