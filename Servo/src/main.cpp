#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "ServoTasks.h"

// Simple web server on port 80
WebServer server(80);

// WiFi credentials (as requested)
const char* ssid = "SBB-FREE";
const char* password = "angela.merkel";

// Mode: true = hardware buttons (read pins), false = software (use web toggles)
bool useHardwareButtons = true;

// Software-held switch states (HIGH/LOW like your globals)
int sw_bit0 = HIGH;
int sw_bit1 = HIGH;
int sw_bit2 = HIGH;

// Forward
String processor(const String& var);

// HTML content served by ESP32 (single-file UI)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width,initial-scale=1" />
<title>ESP32 Servo UI</title>
<style>
  :root{
    --bg:#0f1724; --card:#0b1220; --accent:#00bcd4; --muted:#9aa6b2; --glass: rgba(255,255,255,0.04);
    --radius:14px;
  }
  html,body{height:100%;margin:0;font-family:Inter,system-ui,Segoe UI,Roboto,"Helvetica Neue",Arial;color:#e6eef6;background: linear-gradient(180deg,#071028 0%, #071a2f 60%);}
  .wrap{max-width:980px;margin:32px auto;padding:28px;border-radius:18px;background: linear-gradient(180deg, rgba(255,255,255,0.02), rgba(255,255,255,0.01));box-shadow: 0 12px 30px rgba(2,6,23,0.65);backdrop-filter: blur(6px);}
  header{display:flex;align-items:center;gap:18px;margin-bottom:18px;}
  .logo{width:56px;height:56px;border-radius:12px;background:linear-gradient(135deg,var(--accent),#7be6ff);display:flex;align-items:center;justify-content:center;font-weight:700;color:#032;box-shadow:0 8px 20px rgba(0,188,212,0.12), inset 0 -6px 20px rgba(255,255,255,0.02);}
  h1{font-size:20px;margin:0}
  p.lead{margin:0;color:var(--muted);font-size:13px}

  main.grid{display:grid;grid-template-columns: 360px 1fr;gap:20px;}
  .card{background:var(--card);padding:18px;border-radius:12px;box-shadow: 0 6px 18px rgba(2,6,23,0.5);border:1px solid rgba(255,255,255,0.03);}
  .controls{display:flex;flex-direction:column;gap:12px}
  .row{display:flex;align-items:center;justify-content:space-between;gap:12px}
  .switch{display:flex;align-items:center;gap:12px}
  .toggle{position:relative;width:60px;height:34px;background:var(--glass);border-radius:999px;padding:4px;cursor:pointer;transition:all 180ms ease;}
  .knob{position:absolute;left:4px;top:4px;width:26px;height:26px;background:#fff;border-radius:999px;box-shadow:0 6px 14px rgba(2,6,23,0.4);transition:all 180ms cubic-bezier(.2,.9,.3,1);}
  .on{background:linear-gradient(90deg,var(--accent),#7be6ff);}
  .knob.on{left:30px;transform:scale(1.02)}
  .label{font-size:13px;color:var(--muted)}
  button.btn{background:transparent;border:1px solid rgba(255,255,255,0.06);padding:8px 12px;border-radius:10px;color:#e9f7fb;cursor:pointer}
  .mode{display:flex;gap:8px;align-items:center}

  .status{font-size:13px;color:var(--muted);margin-top:8px}
  .graph-wrap{display:flex;flex-direction:column;gap:12px}
  canvas#pwm{width:100%;height:260px;border-radius:8px;background:linear-gradient(180deg,#061022 0%, #061526 100%);box-shadow:inset 0 2px 8px rgba(0,0,0,0.4);}
  .legend{display:flex;gap:12px;align-items:center;color:var(--muted);font-size:13px}

  footer{margin-top:14px;font-size:12px;color:var(--muted);display:flex;justify-content:space-between;align-items:center}
  .dot{display:inline-block;width:10px;height:10px;border-radius:50%;}
  .ip{font-family:monospace;color:#bfeefc}
  /* subtle animations */
  .card:hover{transform:translateY(-4px);transition:transform 220ms ease}
  .pulse{animation: pulse 2.2s infinite}
  @keyframes pulse{0%{box-shadow:0 0 0 0 rgba(0,188,212,0.12)}70%{box-shadow:0 0 0 20px rgba(0,188,212,0)}100%{box-shadow:0 0 0 0 rgba(0,188,212,0)}}
  .topright{display:flex;gap:10px;align-items:center}
</style>
</head>
<body>
<div class="wrap">
  <header>
    <div class="logo">SV</div>
    <div>
      <h1>ESP32 — Servo Control</h1>
      <p class="lead">Local UI — PWM visualisation • hardware/software buttons • three digital switches</p>
    </div>
    <div style="margin-left:auto" class="topright">
      <div class="mode card" style="padding:8px 12px">
        <span class="label">Mode</span>
        <button id="modeBtn" class="btn">Switch</button>
      </div>
    </div>
  </header>

  <main class="grid">
    <aside class="card">
      <div class="controls">
        <div class="row">
          <div>
            <div style="font-weight:700">Switches</div>
            <div class="status">Three software toggles (Bit0..Bit2). Hardware mode reads physical pins for Bit0/Bit1.</div>
          </div>
        </div>

        <div class="row" style="margin-top:8px">
          <div class="switch">
            <div>
              <div class="label">Bit0</div>
              <div class="label">(used by tasks B/D/E)</div>
            </div>
            <div id="sw0" class="toggle" onclick="toggleSW(0)"><div class="knob" id="knob0"></div></div>
          </div>
        </div>

        <div class="row">
          <div class="switch">
            <div>
              <div class="label">Bit1</div>
              <div class="label">(used by tasks D/E)</div>
            </div>
            <div id="sw1" class="toggle" onclick="toggleSW(1)"><div class="knob" id="knob1"></div></div>
          </div>
        </div>

        <div class="row">
          <div class="switch">
            <div>
              <div class="label">Bit2</div>
              <div class="label">(used by Task F: save position)</div>
            </div>
            <div id="sw2" class="toggle" onclick="toggleSW(2)"><div class="knob" id="knob2"></div></div>
          </div>
        </div>

        <div style="display:flex;gap:8px;margin-top:10px">
          <button class="btn" onclick="refreshState()">Refresh</button>
          <button class="btn" onclick="sendCommand('/help')">Serial Help</button>
        </div>

        <div class="status" id="connStatus">Connecting...</div>
        <div class="status">Current task: <span id="task">—</span></div>
        <div class="status">Servo angle: <strong id="angle">—</strong>°</div>
      </div>
    </aside>

    <section class="card graph-wrap">
      <div style="display:flex;justify-content:space-between;align-items:center">
        <div>
          <div style="font-weight:700">PWM Signal</div>
          <div class="label">Voltage 0–3.3V · Time 0–20ms</div>
        </div>
        <div class="legend">
          <div style="display:flex;gap:8px;align-items:center"><div style="width:12px;height:8px;background:var(--accent);border-radius:3px"></div><div class="label">Pulse (HIGH)</div></div>
          <div style="display:flex;gap:8px;align-items:center"><div style="width:12px;height:8px;background:#243241;border-radius:3px"></div><div class="label">Low</div></div>
        </div>
      </div>
      <canvas id="pwm" width="800" height="400"></canvas>
      <div style="display:flex;justify-content:space-between;align-items:center">
        <div class="label">Pulse width (ms): <span id="pulsems">—</span> ms</div>
        <div class="label">Duty cycle: <span id="duty">—</span>%</div>
      </div>
    </section>
  </main>

  <footer>
    <div class="label">Local server — open the ESP32 IP printed in Serial</div>
    <div class="ip" id="ipaddr">—</div>
  </footer>
</div>

<script>
let useHW = true;
let ipShown = false;

function setModeUI() {
  document.getElementById('modeBtn').innerText = useHW ? 'Hardware' : 'Software';
  document.getElementById('modeBtn').classList.toggle('on', useHW);
}

document.getElementById('modeBtn').addEventListener('click', ()=>{
  // toggle mode on server
  fetch('/set?mode=' + (useHW?0:1)).then(refreshState);
});

function toggleSW(i){
  // only toggle if in software mode
  if (useHW) return;
  let id = 'knob' + i;
  let knob = document.getElementById(id);
  let isOn = knob.classList.contains('on');
  // flip and send to server
  fetch('/set?bit=' + i + '&state=' + (isOn?1:0)).then(refreshState);
}

function refreshState(){
  fetch('/state').then(r=>r.json()).then(s=>{
    useHW = s.useHW;
    setModeUI();
    document.getElementById('ipaddr').innerText = s.ip || '—';
    document.getElementById('angle').innerText = s.angle;
    document.getElementById('task').innerText = s.task;
    document.getElementById('connStatus').innerText = s.conn;
    // set toggle visuals (software bits)
    function setKnob(i, val){
      let k = document.getElementById('knob'+i);
      if(!k) return;
      if(val==0){ k.classList.remove('on'); k.parentElement.classList.remove('on'); } else { k.classList.add('on'); k.parentElement.classList.add('on'); }
    }
    setKnob(0, s.bit0==0?0:1);
    setKnob(1, s.bit1==0?0:1);
    setKnob(2, s.bit2==0?0:1);

    // draw PWM
    drawPWM(s.angle);
  }).catch(e=>{
    console.warn('state fetch failed',e);
  });
}

function sendCommand(cmd){
  fetch('/cmd?c=' + encodeURIComponent(cmd)).then(()=>{});
}

/* Canvas drawing: show 0-3.3V on Y and 0-20ms on X.
   Map servo angle to pulse in ms: 0°->1ms, 180°->2ms
*/
const canvas = document.getElementById('pwm');
const ctx = canvas.getContext('2d');

function drawPWM(angle){
  // clear
  const w = canvas.width;
  const h = canvas.height;
  ctx.clearRect(0,0,w,h);

  // background grid
  ctx.fillStyle = '#05101b';
  ctx.fillRect(0,0,w,h);

  // grid lines
  ctx.strokeStyle = 'rgba(255,255,255,0.03)';
  ctx.lineWidth = 1;
  for(let i=0;i<=4;i++){
    ctx.beginPath();
    ctx.moveTo(0, i*(h/4));
    ctx.lineTo(w, i*(h/4));
    ctx.stroke();
  }
  // axes labels
  ctx.fillStyle = '#9fbec8';
  ctx.font = '12px monospace';
  ctx.fillText('3.3V', 8, 14);
  ctx.fillText('0V', 8, h-8);
  ctx.fillText('0 ms', 8, h-18);
  ctx.fillText('20 ms', w-48, h-18);

  // compute pulse width
  const pulse_ms = 1.0 + (angle / 180.0) * 1.0; // 1ms..2ms
  const duty = (pulse_ms/20.0)*100.0;
  document.getElementById('pulsems').innerText = pulse_ms.toFixed(3);
  document.getElementById('duty').innerText = duty.toFixed(2);

  // draw low baseline
  ctx.fillStyle = '#223241';
  ctx.fillRect(60, 24, w-120, h-64);

  // draw pulse region: map 0..20ms to area width
  const regionX = 60;
  const regionW = w-120;
  const msToX = (ms) => regionX + (ms/20.0) * regionW;
  const yHigh = 32;
  const yLow = h-32;

  // draw low line
  ctx.fillStyle = '#0f2130';
  ctx.fillRect(regionX, yHigh, regionW, yLow - yHigh);

  // draw pulse (HIGH area)
  const px0 = msToX(0);
  const px1 = msToX(pulse_ms);
  ctx.fillStyle = '#00bcd4';
  ctx.beginPath();
  ctx.moveTo(px0, yLow);
  ctx.lineTo(px0, yHigh);
  ctx.lineTo(px1, yHigh);
  ctx.lineTo(px1, yLow);
  ctx.closePath();
  ctx.fill();

  // draw borders
  ctx.strokeStyle = 'rgba(255,255,255,0.06)';
  ctx.strokeRect(regionX, yHigh, regionW, yLow - yHigh);

  // draw annotated pulse ms
  ctx.fillStyle = '#bfeefc';
  ctx.fillText(pulse_ms.toFixed(3) + ' ms', px1 + 6, yHigh + 16);
}

setInterval(refreshState, 700);
window.onload = refreshState;
</script>
</body>
</html>
)rawliteral";

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// Return current state as JSON
void handleState() {
  char buff[512];
  // Build a lightweight JSON
  snprintf(buff, sizeof(buff),
           "{\"useHW\":%d,\"bit0\":%d,\"bit1\":%d,\"bit2\":%d,\"angle\":%d,\"task\":%d,\"ip\":\"%s\",\"conn\":\"%s\"}",
           useHardwareButtons ? 1 : 0,
           bit0, bit1, bit2,
           currentAngle,
           currentTask,
           WiFi.localIP().toString().c_str(),
           WiFi.isConnected() ? "Wi-Fi connected" : "No Wi-Fi");
  server.send(200, "application/json", buff);
}

// Toggle mode or set bits via query params
void handleSet() {
  if (server.hasArg("mode")) {
    // mode=0 => switch to software mode (server-side toggles); mode=1 => hardware
    int mode = server.arg("mode").toInt();
    useHardwareButtons = (mode == 1);
    // reply with new mode
    handleState();
    return;
  }

  if (server.hasArg("bit") && server.hasArg("state")) {
    int bit = server.arg("bit").toInt();    // 0..2
    int state = server.arg("state").toInt(); // 0 -> LOW, 1 -> HIGH
    int logic = (state == 0) ? LOW : HIGH;

    // Only allow software to set these while in software mode.
    // But allow setting even in hardware mode for convenience; UI prevents toggle.
    switch (bit) {
      case 0: sw_bit0 = logic; bit0 = logic; break;
      case 1: sw_bit1 = logic; bit1 = logic; break;
      case 2: sw_bit2 = logic; bit2 = logic; break;
      default: break;
    }
    handleState();
    return;
  }

  server.send(400, "text/plain", "bad request");
}

// Endpoint to forward plain serial-like commands (optional)
void handleCmd() {
  if (!server.hasArg("c")) { server.send(400, "text/plain", "missing"); return; }
  String c = server.arg("c");
  c.trim();
  handleCommand(c); // use your existing handler from ServoTasks.cpp
  server.send(200, "text/plain", "ok");
}

void setup() {
  Serial.begin(115200);
  delay(20);
  Serial.println();
  Serial.println("Booting...");

  // Attach servo as before
  myServo.attach(25, 0, 180, false);

  // Physical switches (keep original pins)
  pinMode(switchPin0, INPUT_PULLUP);
  pinMode(switchPin1, INPUT_PULLUP);

  writeServo(90, "Servo initialized at");
  printHelp();

  // Start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to Wi-Fi '%s' ...\n", ssid);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
    delay(250);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Wi-Fi connection failed or timed out. The web UI still runs on AP? (not created). Check network.");
  }

  // HTTP routes
  server.on("/", []() {
    server.send_P(200, "text/html", index_html);
  });
  server.on("/state", handleState);
  server.on("/set", handleSet);
  server.on("/cmd", handleCmd);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  // If hardware mode is enabled, read the pins and set bit0/bit1 accordingly.
  if (useHardwareButtons) {
    bit0 = digitalRead(switchPin0);
    bit1 = digitalRead(switchPin1);
    // Ensure software mirrors these too
    sw_bit0 = bit0;
    sw_bit1 = bit1;
  } else {
    // software mode: override hardware bits with software toggles
    bit0 = sw_bit0;
    bit1 = sw_bit1;
    bit2 = sw_bit2;
  }

  // Serial commands handling (unchanged)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    handleCommand(cmd);
  }

  // Run the chosen task (unchanged behavior)
  if (currentTask >= 1 && currentTask <= 7) {
    // The tasks inside ServoTasks use the global bit0..bit3 variables directly
    typedef void (*TaskFunc)();
    TaskFunc taskTable[7] = {taskA, taskB, taskC, taskD, taskE, taskF, taskG};
    taskTable[currentTask - 1]();
  }

  // let webserver handle clients
  server.handleClient();

  // light delay to keep things responsive
  delay(10);
}
