# ESP32 Water Pump Automation

A secure, robust, real-time ESP32 web server project to control a 5 HP water pump via a 24V relay. Built with C++ (Arduino IDE), HTML, and JavaScript — optimized for performance, security, and mobile responsiveness.

---

## 🚀 Features

### 🔧 Core Pump Control
- Start pump with user-defined timer (input as minutes or seconds)
- Default to 20 minutes if no input
- Manual stop anytime
- Failsafe 30-minute hard timeout
- 24V relay control (direct GPIO trigger)
- Rejects second start attempt if pump is already running (shows inline error)

### 🔐 Authentication System
- Secure login form (username + password)
- Session timeout after 3 minutes of inactivity
- Role-based access (admin/user)
- Case-insensitive usernames
- Logout clears session and redirects to login
- SHA-256 password hashing with mbedTLS

### 👥 User Management (Admin Only)
- Add new users with roles
- Delete users (with confirmation)
- Change user passwords without old password
- Admin/user landing page UI adjusted dynamically

### 🌐 Real-Time WebSocket UI
- Real-time pump status and countdown updates
- Pushes updates every second
- Connected clients stay synced
- Displays countdown in `MM:ss` format

### 📲 Responsive Web Interface
- Hosted entirely from ESP32 using inline HTML/CSS/JS
- Mobile-friendly Material UI look
- Inline alert messages (e.g. "Pump already running", "Invalid credentials")
- Navigation buttons for logout, user management
- 404 and session expired pages

### 💾 SPIFFS-Based Storage
- `/users.json` for persistent user database
- `/state.json` for storing pump state
- Automatically creates files on first boot

### 🔁 Power Recovery
- Pump state is restored after power outage
- Countdown continues from previous state
- System remains operational and safe

### ⚙️ Configurable Options (via `variables.h`)
- WiFi credentials
- GPIO pin for relay
- Default/max timer
- Session timeout
- File paths
- Default users and number to initialize
- `INPUT_IN_MINUTES` toggle for user input format

---

## 📁 Project Structure

```
ESP32_Water_Pump_Automation/
├── ESP32_Water_Pump_Automation.ino    # Main file with route handling and loop
├── variables.h                         # Central config (WiFi, GPIO, timeouts)
├── authentication.h                   # Login, session, hashing
├── pump_control.h                     # Relay control, timer, power recovery
├── websocket.h                        # WebSocket server for real-time updates
├── filesystem.h                       # SPIFFS init, default user/state creation
├── ui.h                               # Inline HTML/CSS/JS frontend
└── README.md                          # Project documentation
```

---

## 🛠 Setup Instructions

### 1. 🔧 Hardware
- ESP32 Dev Board
- Relay module (24V compatible)
- 5 HP pump starter (external, controlled by relay)

### 2. 🔌 Wiring
- Relay IN pin connected to GPIO defined in `variables.h` (default: GPIO 5)
- 24V relay switches the pump starter signal

### 3. 💻 Software
- Install [Arduino IDE](https://www.arduino.cc/en/software)
- Add ESP32 Board via Board Manager
- Install required libraries:
  - `WebServer`
  - `WebSocketsServer`
  - `ArduinoJson`
  - `SHA256`

### 4. 🧠 Flash and Run
- Open `ESP32_Water_Pump_Automation.ino`
- Fill in your WiFi credentials in `variables.h`
- Flash to ESP32
- Monitor serial output for IP address

### 5. 🌐 Access Web UI
- Open browser → `http://<esp32-ip>`
- Default credentials:
  - **admin / admin123**
  - **guest / guest123**

---

## 📡 Exposing to Internet (Optional)
- Use Dynamic DNS (e.g. DuckDNS)
- Configure port forwarding on your router (HTTP + WebSocket port 81)
- Secure with reverse proxy or TLS if exposed publicly

---

## 🔐 Security Notes
- All passwords stored as SHA-256 hashes
- No plaintext credential handling
- No access to pump or user APIs without valid session + role check
- Sessions expire after inactivity
- No `unordered_map` used — static memory-friendly session tracking

---

## 📌 Future Enhancements
- Audit trail/logs for pump operations
- Remote firmware updates
- MQTT integration / Matter support
- Dark mode toggle for UI

---

## 🧠 Author
Built with ♥ for secure, reliable pump control automation. Let me know if you’d like help expanding the system!---

## 📚 Required Libraries

Install these libraries via Arduino Library Manager or manually from GitHub:

| Library                | GitHub Link                                               |
|------------------------|------------------------------------------------------------|
| ESPAsyncWebServer      | https://github.com/me-no-dev/ESPAsyncWebServer            |
| AsyncTCP               | https://github.com/me-no-dev/AsyncTCP                     |
| ArduinoJson            | https://github.com/bblanchon/ArduinoJson                  |
| ESP32 Board Support    | https://github.com/espressif/arduino-esp32                |

> Note: `AsyncTCP` is required for `ESPAsyncWebServer` to compile.

---

## ✅ Test Plan – Functional Validation

### 🔌 Setup
- [ ] Flash firmware and connect ESP32 to WiFi
- [ ] Confirm IP is printed on serial monitor

### 🔐 Login & Session
- [ ] Visit `http://<esp32-ip>/login`
- [ ] Try login with:
  - [ ] Correct admin credentials
  - [ ] Incorrect password
- [ ] Session times out after 3 minutes (verify logout)

### 🧑‍💼 Admin Panel
- [ ] Add a new user with role `user`
- [ ] Delete existing user
- [ ] Change password for any user
- [ ] Navigation to Control and Logout works

### 💧 Pump Control
- [ ] Start pump with custom time (e.g., 10s)
- [ ] Default to 20 mins if no value
- [ ] Manual stop works
- [ ] Pump stops after timeout (max 30 mins cap)

### 🌐 Real-time UI
- [ ] Multiple devices reflect status live via WebSocket
- [ ] Countdown updates every second
- [ ] WebSocket reconnects on refresh

### 🔁 Power Recovery
- [ ] Start pump, reboot ESP32 → pump resumes countdown
- [ ] After timer expires, pump stops automatically

### 🧪 Security
- [ ] Test direct access to `/pump`, `/usermgmt`, `/api/start` without login → should redirect or block
- [ ] Cookies expire after session timeout
- [ ] User can’t access admin-only pages

---

# ✅ ESP32 Water Pump Automation – Final Test Checklist

## 🔧 Setup Check
- [ ] Flash the firmware
- [ ] Confirm ESP32 connects to Wi-Fi and prints IP
- [ ] Open browser and navigate to `http://<ESP32_IP>`

## 🔐 Authentication & Session
- [ ] Login as **admin**
- [ ] Login as **user**
- [ ] Wrong password shows "Invalid credentials"
- [ ] After 3 minutes of inactivity, session expires and redirects to login
- [ ] Logout redirects to login

## 👤 Admin Panel
- [ ] Add new user (admin/user)
- [ ] Delete existing user
- [ ] Change user password (no old password required)
- [ ] Navigation between UserMgmt, Control, Logout works
- [ ] Users dropdown updates dynamically

## 💧 Pump Control
- [ ] Enter valid time → starts pump
- [ ] Leave time empty → defaults to 20 minutes
- [ ] Enter too large value (>30 min) → fails gracefully
- [ ] Press "Stop Pump" manually → stops instantly
- [ ] Press "Start Pump" while already running → shows “Pump already running” inline message

## 🌐 Real-time WebSocket Updates
- [ ] Open pump control page in 2+ tabs/devices
- [ ] Start/Stop pump from one → updates reflect in all others
- [ ] Countdown updates every second in `MM:ss` format

## 🔁 Power Recovery
- [ ] Start pump → reboot ESP32
- [ ] After reboot, pump resumes if within allowed time
- [ ] Remaining time continues counting correctly

## 🔐 Security Tests
- [ ] Try accessing `/pump`, `/usermgmt`, `/api/start` directly without login → blocked
- [ ] User cannot access `/usermgmt`
- [ ] Invalid session cookie redirects to login
- [ ] Passwords are hashed using SHA-256

## 🧪 Edge Case Tests
- [ ] Start pump with exactly `30` minutes → allowed
- [ ] Try `31` minutes → rejected
- [ ] Set `INPUT_IN_MINUTES = false` → test time as seconds
- [ ] Refresh browser repeatedly → WebSocket reconnects without issues

## 🧹 Final UX Polishing
- [ ] UI looks clean and mobile-responsive
- [ ] Alert messages (login, pump errors) appear inline and fade after 5 seconds
- [ ] Navigation feels smooth

## 🔧 Bonus Tests (Optional)
- [ ] Test with power failure mid-pump
- [ ] Test session timeout while pump is running
- [ ] View `remaining time` countdown after reboot