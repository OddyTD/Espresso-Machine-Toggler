#include "network.hpp"
#include "globals.hpp"
#include <FS.h> // File type on ESP8266

// Upload state + 500 KB cap for ESP8266 safety
static File upFile;
static String upTarget;
static size_t upSize = 0;
static bool upTooLarge = false;
static const size_t BG_MAX_BYTES = 500 * 1024; // 500 KB (safer for ESP8266)

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
            // ---- finished (success or fail) ----
            [this]()
            {
    if (upFile) upFile.close();

    // No file / wrong type
    if (upTarget.isEmpty()) {
      if (LittleFS.exists("/bg_user.tmp")) LittleFS.remove("/bg_user.tmp");
      server.send(415, "text/plain", "Unsupported type. Use .webp or .jpg");
      upSize = 0; upTooLarge = false; return;
    }

    // Too large - ESP8266 protection
    if (upTooLarge || upSize > BG_MAX_BYTES) {
      if (LittleFS.exists("/bg_user.tmp")) LittleFS.remove("/bg_user.tmp");
      server.sendHeader("Connection", "close"); // nudge some clients to finish
      server.send(413, "text/plain", "File too large. Max 500 KB for ESP8266 safety.");
      Serial.printf("[UPLOAD] REJECTED: %u bytes exceeds 500KB limit\n", (unsigned)upSize);
      upSize = 0; upTarget = ""; upTooLarge = false; return;
    }

    // Success: atomically replace existing
    if (LittleFS.exists(upTarget)) LittleFS.remove(upTarget);
    LittleFS.rename("/bg_user.tmp", upTarget);
    server.send(200, "text/plain", "Background updated (" + String(upSize) + " bytes).");
    Serial.printf("[UPLOAD] SUCCESS: %u bytes saved as %s\n", (unsigned)upSize, upTarget.c_str());

    // reset state
    upSize = 0; upTarget = ""; upTooLarge = false; },

            // ---- chunked upload ----
            [this]()
            {
    HTTPUpload& u = server.upload();

    if (u.status == UPLOAD_FILE_START) {
      // Check Content-Length header for early rejection
      if (server.hasHeader("Content-Length")) {
        int contentLength = server.header("Content-Length").toInt();
        if (contentLength > (int)BG_MAX_BYTES) {
          Serial.printf("[UPLOAD] EARLY REJECT: Content-Length %d > 500KB\n", contentLength);
          upTooLarge = true;
          upTarget = ""; // Clear target to trigger rejection
          return; // Don't even start processing
        }
      }

      upFile = File();
      upSize = 0;
      upTooLarge = false;
      upTarget = "";

      String fn = u.filename; fn.toLowerCase();
      if (fn.endsWith(".webp"))                         upTarget = "/bg_user.webp";
      else if (fn.endsWith(".jpg") || fn.endsWith(".jpeg")) upTarget = "/bg_user.jpg";

      // open temp only if type ok and not already rejected
      if (!upTarget.isEmpty() && !upTooLarge) {
        upFile = LittleFS.open("/bg_user.tmp", "w");
        Serial.printf("[UPLOAD] START: %s -> temp (max 500KB)\n", u.filename.c_str());
      } else {
        Serial.printf("[UPLOAD] REJECTED: %s (unsupported or too large)\n", u.filename.c_str());
      }
    }
    else if (u.status == UPLOAD_FILE_WRITE) {
      // Always track size; even if we stop writing, finalizer will decide
      upSize += u.currentSize;

      // If we crossed the cap, stop writing further and mark it
      if (!upTooLarge && upSize > BG_MAX_BYTES) {
        upTooLarge = true;
        Serial.printf("[UPLOAD] SIZE LIMIT HIT: %u bytes > 500KB, stopping write\n", (unsigned)upSize);
        if (upFile) { upFile.close(); }
        // keep consuming the rest of the chunks; we'll reply 413 at finish
      }

      // Only write if within cap and file is open
      if (!upTooLarge && upFile) {
        // Write in larger chunks for better performance
        size_t written = upFile.write(u.buf, u.currentSize);
        if (written != u.currentSize) {
          Serial.println(F("[UPLOAD] Write error - filesystem full?"));
          upTooLarge = true;
          upFile.close();
        }
      }
    }
    else if (u.status == UPLOAD_FILE_END) {
      if (upFile) upFile.close();
      Serial.printf("[UPLOAD] END: %u bytes total (ESP8266 limit: 500KB)\n", (unsigned)upSize);
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
  Serial.println(F("[WEBSERVER] Web Server started! Upload limit: 500KB"));
}

void WebServerConfig::handleClient()
{
  server.handleClient();
}
