# ☕ ESP8266 Espresso Toggler

[![Status](https://img.shields.io/badge/status-working-brightgreen.svg)]()
![Platform](https://img.shields.io/badge/platform-ESP8266-blue.svg)
![Framework](https://img.shields.io/badge/framework-Arduino%20%2B%20PlatformIO-orange)
![FS](https://img.shields.io/badge/filesystem-LittleFS-lightgrey)
![Last commit](https://img.shields.io/github/last-commit/OddyTD/Espresso-Machine-Toggler)
![Repo size](https://img.shields.io/github/repo-size/OddyTD/Espresso-Machine-Toggler)

Control an espresso machine servo from any device on your LAN via a simple web page served directly by the ESP8266.

---

## ✨ Features
- Lightweight web UI (LittleFS) served by the ESP8266
- Three actions: **Espresso Toggle**, **Jog Forward**, **Jog Backward**
- Clean HTTP endpoints: `/espresso`, `/jog_forward`, `/jog_reverse`
- Minimal code, no external server required

---

## 🔌 Hardware
- **Board:** ESP8266 (ESP-12E)
- **Actuator:** standard hobby servo  
- **Pinout:** PWM on **D5 (GPIO14)** by default  
- **Power:** ensure a **separate, stable 5 V** rail for the servo (not from the ESP8266’s 3.3 V)

---

## 📁 Project Structure
```text
.
├─ include/
│  ├─ globals.hpp
│  ├─ messages.hpp
│  ├─ servo_action.hpp
│  └─ credentials.example.hpp   (copy → credentials.hpp)
├─ src/
│  ├─ main.cpp
│  ├─ network.cpp
│  └─ servo_action.cpp
├─ data/
│  ├─ index.html
│  └─ bg.webp / bg.jpg
└─ platformio.ini
```

---

## 🚀 Quick Start (PlatformIO)
1. **Clone & open** the repo in VS Code (PlatformIO installed).
2. **Wi-Fi credentials**
   - Copy `include/credentials.example.hpp` → `include/credentials.hpp`
   - Fill in:
     ```cpp
     #define WIFI_SSID     "YourSSID"
     #define WIFI_PASSWORD "YourPassword"
     ```
3. **Upload filesystem (LittleFS)**  
   PlatformIO → “Upload Filesystem Image” (or `pio run -t uploadfs`)
4. **Flash firmware**  
   PlatformIO → “Upload”
5. **Browse to the ESP IP** (shown in Serial Monitor) → control the servo.

---

## 🌐 Web UI & API
- **UI:** `GET /` → serves `index.html` from LittleFS
- **Actions:**
  - `GET /espresso` → forward → pause → reverse (one shot)
  - `GET /jog_forward` → short forward nudge
  - `GET /jog_reverse` → short reverse nudge

  The server uses:
```cpp
// Serve all static files from LittleFS with proper MIME types
server.serveStatic("/", LittleFS, "/");
```

---

## 👤 Author
- **Oddy (Tung Do) — Student in Electrical Engineering @ ÉTS**