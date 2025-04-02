#ifndef UI_H
#define UI_H

#include <Arduino.h>

const char COMMON_HEADER[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 Pump Control</title>
<style>
body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  margin: 0;
  padding: 0;
  background: #f4f6f8;
}

.container {
  max-width: 500px;
  margin: 2em auto;
  background: #ffffff;
  border-radius: 16px;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.1);
  padding: 2em;
  box-sizing: border-box;
}

h1, h2 {
  font-weight: 600;
  color: #333;
  text-align: center;
  margin-bottom: 1em;
}

button {
  background: #1976d2;
  color: white;
  padding: 12px 20px;
  border: none;
  border-radius: 8px;
  font-size: 16px;
  cursor: pointer;
  margin: 10px 0;
  width: 100%;
  box-shadow: 0 3px 6px rgba(25, 118, 210, 0.3);
  transition: background 0.3s ease;
}

button:hover {
  background: #1565c0;
}

input, select {
  width: 100%;
  padding: 12px 14px;
  margin: 8px 0 20px 0;
  border-radius: 8px;
  border: 1px solid #ccc;
  font-size: 16px;
  box-sizing: border-box;
}

.navbar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 1em;
  padding-bottom: 1em;
  border-bottom: 1px solid #ddd;
}

.alert {
  background: #ffebee;
  color: #c62828;
  padding: 1em;
  margin-top: 1em;
  border-radius: 8px;
  font-weight: 500;
}
</style>
</head><body><div class="container">
)rawliteral";

const char COMMON_FOOTER[] PROGMEM = R"rawliteral(
</div></body></html>
)rawliteral";

const char LOGIN_PAGE[] PROGMEM = R"rawliteral(
<h2>Login</h2>
<form method="POST" action="/login">
  <input type="text" name="username" placeholder="Username" required />
  <input type="password" name="password" placeholder="Password" required />
  <button type="submit">Login</button>
</form>
)rawliteral";

const char SESSION_EXPIRED_PAGE[] PROGMEM = R"rawliteral(
<h2>Session Expired</h2>
<div class="alert">Your session has expired due to inactivity.</div>
<a href="/login"><button>Login Again</button></a>
)rawliteral";

const char NOT_FOUND_PAGE[] PROGMEM = R"rawliteral(
<h2>404 - Page Not Found</h2>
<div class="alert">Oops! The page you're looking for doesn't exist.</div>
<a href="/"><button>Home</button></a>
)rawliteral";

const char PUMP_CONTROL_PAGE[] PROGMEM = R"rawliteral(
<div class="navbar">
  <div><strong>ESP32 Pump</strong></div>
  <div>
    %ADMIN_BUTTONS%
    <a href="/logout"><button>Logout</button></a>
  </div>
</div>

<h2>Pump Control</h2>
<div id="pumpAlert" class="alert" style="display:none;"></div>
<p>Status: <span id="status">-</span></p>
<p>Remaining Time: <span id="remaining">00:00</span></p>

<form onsubmit="startPump(); return false;">
  <input type="number" id="timer" placeholder="Enter time (seconds)" min="10" />
  <!--<input type="number" id="timer" placeholder="Enter time (minutes)" min="1" />-->
  <button type="submit">Start Pump</button>
</form>
<button onclick="stopPump()">Stop Pump</button>

<script>
let socket = new WebSocket("ws://" + location.hostname + "/ws");
socket.onmessage = function(event) {
  let data = JSON.parse(event.data);
  document.getElementById("status").textContent = data.status;
  function formatTime(seconds) {
    let mins = Math.floor(seconds / 60);
    let secs = seconds % 60;
    return String(mins).padStart(2, '0') + ":" + String(secs).padStart(2, '0');
  }

  socket.onmessage = function(event) {
    let data = JSON.parse(event.data);
    document.getElementById("status").textContent = data.status;
    document.getElementById("remaining").textContent = formatTime(data.remaining);
  };
};

function startPump() {
  const time = document.getElementById("timer").value;
  fetch("/api/start", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: "duration=" + encodeURIComponent(time)
  })
  .then(res => {
    if (!res.ok) {
      return res.text().then(msg => { throw new Error(msg); });
    }
    return res.text();
  })
  .then(() => setTimeout(() => location.reload(), 500))
  .catch(err => {
    const alertBox = document.getElementById("pumpAlert");
    alertBox.style.display = "block";
    alertBox.textContent = err.message;
    setTimeout(() => alertBox.style.display = "none", 5000);
  });
}

function stopPump() {
  fetch("/api/stop", { method: "POST" }).then(() => setTimeout(() => location.reload(), 500));
}
</script>
)rawliteral";

const char USER_MGMT_PAGE[] PROGMEM = R"rawliteral(
<div class="navbar">
  <div><strong>User Management</strong></div>
  <div>
    <a href="/pump"><button>Control</button></a>
    <a href="/logout"><button>Logout</button></a>
  </div>
</div>

<h2>Manage Users</h2>

<h3>Add New User</h3>
<form method="POST" action="/adduser">
  <input type="text" name="username" placeholder="Username" required />
  <input type="password" name="password" placeholder="Password" required />
  <select name="role">
    <option value="user">User</option>
    <option value="admin">Admin</option>
  </select>
  <button type="submit">Add User</button>
</form>

<h3>Delete User</h3>
<form method="POST" action="/deleteuser">
  <select name="username">%USER_OPTIONS%</select>
  <button type="submit" onclick="return confirm('Are you sure you want to delete this user?')">Delete</button>
</form>

<h3>Change User Password</h3>
<form method="POST" action="/changepassword">
  <select name="username">%USER_OPTIONS%</select>
  <input type="password" name="password" placeholder="New Password" required />
  <button type="submit">Change Password</button>
</form>
)rawliteral";

#endif