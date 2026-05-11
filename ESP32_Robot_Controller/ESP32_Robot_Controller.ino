#include <WiFi.h>  
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== LCD =====
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== MOTOR PINS =====
#define IN1 14
#define IN2 27
#define ENA 25
#define IN3 26
#define IN4 33
#define ENB 32

// ===== ULTRASONIC =====
#define TRIG 5
#define ECHO 18

// ===== PIR =====
#define PIR 19

// ===== RELAYS =====
#define RELAY 4
#define RELAY2 16   // NEW


// ===== AIR QUALITY =====
int airValue = 0;
String airStatus = "Unknown";

unsigned long airTimer = 0;


unsigned long pirRelayStart = 0;
bool pirRelayRunning = false;
// ===== MODE =====
bool autoMode = false;
bool manualMode = false;
bool stoppedMode = true;

// ===== WIFI =====
const char* ssid = "THARUN10";
const char* password ="12345678";

WebServer server(80);
int speedValue = 150;

unsigned long autoTimer = 0;

unsigned long pirTimer = 0;
bool pirActive = true;

// ===== LCD =====
void lcdShow(String l1, String l2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(l1.substring(0, 16));
  lcd.setCursor(0, 1);
  lcd.print(l2.substring(0, 16));
}

// ===== MOTOR =====
void applySpeed(int s) {
  analogWrite(ENA, s);
  analogWrite(ENB, s);
}

void forward() {
  digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);  digitalWrite(IN4,HIGH);
  applySpeed(speedValue);
  lcdShow("AUTO/MANUAL","FORWARD");
}

void backward() {
  digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
  digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);
  applySpeed(speedValue);
  lcdShow("MANUAL","BACKWARD");
}

void left() {
  digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);
  digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);
  applySpeed(speedValue);
  lcdShow("MANUAL","LEFT");
}

void right() {
  digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH);
  digitalWrite(IN3,LOW); digitalWrite(IN4,HIGH);
  applySpeed(speedValue);
  lcdShow("MANUAL","RIGHT");
}

void stopRobot() {
  applySpeed(0);
  digitalWrite(IN1,LOW); digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW); digitalWrite(IN4,LOW);
  lcdShow("ROBOT","STOPPED");
}

// ===== DISTANCE =====
float getDistance() {
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long d = pulseIn(ECHO, HIGH, 30000);
  if(d==0) return -1;
  return d*0.034/2;
}

String homePage() {
return R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{
  margin:0;
  font-family:Arial;
  background:linear-gradient(135deg,#0f2027,#203a43,#2c5364);
  color:white;
  display:flex;
  justify-content:center;
  align-items:center;
  height:100vh;
}
.card{
  background:#ffffff10;
  backdrop-filter:blur(15px);
  padding:30px;
  border-radius:20px;
  text-align:center;
  width:320px;
  box-shadow:0 10px 30px rgba(0,0,0,0.5);
}
h1{margin-bottom:20px;}

.airBox{
  margin:20px 0;
}

.value{
  font-size:35px;
  font-weight:bold;
}

.status{
  font-size:18px;
  opacity:0.8;
}

button{
  width:100%;
  padding:12px;
  margin:8px 0;
  font-size:16px;
  border:none;
  border-radius:10px;
  background:#00c6ff;
  color:white;
  font-weight:bold;
}
button:hover{background:#0072ff;}

svg{
  width:80px;
  height:80px;
}
</style>
</head>

<body>

<div class="card">
<h1>Robot Dashboard</h1>

<!-- SVG ICON -->
<svg viewBox="0 0 100 100">
  <circle cx="50" cy="50" r="40" stroke="#00eaff" stroke-width="5" fill="none"/>
  <text x="50%" y="55%" text-anchor="middle" fill="#00eaff" font-size="18">AQ</text>
</svg>

<div class="airBox">
  <div class="value" id="airValue">--</div>
  <div class="status" id="airStatus">Loading...</div>
</div>

<button onclick="location.href='/autoPage'">AUTO MODE</button>
<button onclick="location.href='/manualPage'">MANUAL MODE</button>

</div>

<script>
function updateAir(){
 fetch('/airData')
 .then(r=>r.json())
 .then(d=>{
   document.getElementById('airValue').innerHTML = d.value;
   document.getElementById('airStatus').innerHTML = d.status;
 });
}

setInterval(updateAir,2000);
updateAir();
</script>

</body>
</html>
)HTML";
}

String autoPage() {
return R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{
  margin:0;
  font-family:Arial;
  background:#f1f5f9;
  text-align:center;
}
.card{
  max-width:400px;
  margin:40px auto;
  background:white;
  padding:25px;
  border-radius:20px;
  box-shadow:0 8px 20px rgba(0,0,0,0.15);
}
h2{margin-top:0;}
.value{
  font-size:40px;
  font-weight:bold;
  color:#111;
}
button{
  width:90%;
  padding:12px;
  margin:8px 0;
  border:none;
  border-radius:10px;
  font-size:16px;
  font-weight:bold;
  color:white;
}
.green{background:#28a745;}
.red{background:#dc3545;}
.blue{background:#007bff;}
.gray{background:#6c757d;}
</style>
</head>

<body>
<div class="card">
<h2>AUTO MODE</h2>

<p>Distance</p>
<div class="value"><span id="d">--</span> cm</div>

<button class="green" onclick="relayOn()">Relay ON</button>
<button class="red" onclick="relayOff()">Relay OFF</button>

<button class="blue" onclick="checkPir()">Check PIR</button>

<button class="gray" onclick="stopAuto()">STOP</button>
<button class="gray" onclick="location.href='/'">BACK</button>
</div>

<script>
function update(){
 fetch('/autoData').then(r=>r.json()).then(d=>{
  document.getElementById('d').innerHTML=d.distance;

  if(d.blocked=="1"){
    alert("Obstacle detected! Motor stopped");
  }
 });
}

function relayOn(){fetch('/relay?state=on');}
function relayOff(){fetch('/relay?state=off');}

function checkPir(){
 fetch('/pir').then(r=>r.text()).then(d=>{
  if(d=="1") alert("Human Detected!");
  else alert("No Human");
 });
}

function stopAuto(){fetch('/mode?m=stop');}

fetch('/mode?m=auto');
setInterval(update,1500);
</script>

</body>
</html>
)HTML";
}

String manualPage() {
return R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{
  margin:0;
  font-family:Arial;
  background:#0f172a;
  color:white;
  text-align:center;
}
.container{
  padding-top:30px;
}
.grid{
  display:grid;
  grid-template-columns:80px 80px 80px;
  grid-gap:10px;
  justify-content:center;
  margin-top:20px;
}
.btn{
  background:#1e293b;
  padding:20px;
  border-radius:15px;
  font-size:25px;
}
.btn:active{background:#334155;}
.slider{width:80%;}
</style>
</head>

<body>
<div class="container">
<h2>MANUAL CONTROL</h2>

<div class="grid">
<div></div><div class="btn" onclick="send('forward')">↑</div><div></div>
<div class="btn" onclick="send('left')">←</div>
<div class="btn" onclick="send('stop')">■</div>
<div class="btn" onclick="send('right')">→</div>
<div></div><div class="btn" onclick="send('backward')">↓</div><div></div>
</div>

<br>
Speed: <span id="s">150</span><br>
<input type="range" min="0" max="255" value="150" class="slider" oninput="sp(this.value)">

<br><br>
<button onclick="location.href='/'">BACK</button>
</div>

<script>
fetch('/mode?m=manual');

function send(d){fetch('/move?dir='+d);}
function sp(v){
 document.getElementById('s').innerHTML=v;
 fetch('/speed?value='+v);
}
</script>

</body>
</html>
)HTML";
}

void updateAirQuality() {
  airValue = random(2150, 3749);



  Serial.print("Air: ");
  Serial.print(airValue);
  Serial.print(" | ");
  Serial.println(airStatus);

  // Show on LCD (short format)
  lcdShow("Air:" + String(airValue), airStatus);
}


// ===== SETUP =====
void setup() {
  // === 1. ALLOW THE LAPTOP TO TALK TO THE ROBOT ===
  server.enableCORS(true);

  server.on("/status", [](){
    float d = getDistance();
    int p = digitalRead(PIR);
    
    // We added the airValue variable to the JSON string below:
    String json = "{\"distance\":\"" + String(d) + "\",\"pir\":\"" + String(p) + "\",\"air\":\"" + String(airValue) + "\"}";
    
    server.send(200, "application/json", json);
  });
Serial.begin(115200);

pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT);
pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT);
pinMode(ENA,OUTPUT); pinMode(ENB,OUTPUT);

pinMode(TRIG,OUTPUT); pinMode(ECHO,INPUT);
pinMode(PIR,INPUT);

pinMode(RELAY,OUTPUT);
pinMode(RELAY2,OUTPUT);

digitalWrite(RELAY,LOW);
digitalWrite(RELAY2,LOW);

lcd.init(); lcd.backlight();
lcdShow("Connecting","WiFi");

WiFi.begin(ssid,password);

while(WiFi.status()!=WL_CONNECTED){
  delay(500);
  Serial.print(".");
}

Serial.println("\nWiFi Connected!");
Serial.print("IP Address: ");
Serial.println(WiFi.localIP());

// SHOW IP ON LCD PROPERLY
String ip = WiFi.localIP().toString();
lcd.clear();
lcd.setCursor(0,0);
lcd.print("WiFi Connected");
lcd.setCursor(0,1);
lcd.print(ip);

// WAIT so user can see it
delay(4000);

// ROUTES
server.on("/",[](){server.send(200,"text/html",homePage());});
server.on("/autoPage",[](){server.send(200,"text/html",autoPage());});
server.on("/manualPage",[](){server.send(200,"text/html",manualPage());});

// ✅ MANUAL FIX
server.on("/move",[](){

 if(!manualMode){
   server.send(200,"text/plain","IGNORED");
   return;
 }

 String d=server.arg("dir");

 if(d=="forward") forward();
 else if(d=="backward") backward();
 else if(d=="left") left();
 else if(d=="right") right();
 else stopRobot();

 server.send(200,"text/plain","OK");
});

server.on("/speed",[](){
 speedValue=server.arg("value").toInt();
 lcdShow("Speed",String(speedValue));
 server.send(200,"text/plain","OK");
});

server.on("/mode",[](){
 String m=server.arg("m");

 if(m=="auto"){autoMode=true;manualMode=false;lcdShow("MODE","AUTO");}
 else if(m=="manual"){manualMode=true;autoMode=false;stopRobot();lcdShow("MODE","MANUAL");}
 else{autoMode=false;manualMode=false;stopRobot();lcdShow("MODE","STOP");}

 server.send(200,"text/plain","OK");
});

server.on("/relay",[](){
  String device = server.arg("device");
  String s = server.arg("state");

  // 1. Control the UV Lamp (RELAY2)
  if (device == "uv") {
    if(s == "on"){ digitalWrite(RELAY2, HIGH); lcdShow("UV Lamp", "ON"); }
    else{ digitalWrite(RELAY2, LOW); lcdShow("UV Lamp", "OFF"); }
  }
  // 2. Control the Mopping Pump (RELAY)
  else if (device == "mop") {
    if(s == "on"){ digitalWrite(RELAY, HIGH); lcdShow("Mop Pump", "ON"); }
    else{ digitalWrite(RELAY, LOW); lcdShow("Mop Pump", "OFF"); }
  }
  
  server.send(200,"text/plain","OK");
});
// PIR
// server.on("/pir",[](){

//  if(!pirActive){
//    server.send(200,"text/plain","0");  // PIR OFF state
//    return;
//  }


// server.on("/pir",[](){

//   // Check relay instead of PIR sensor
//   if (digitalRead(RELAY2) == HIGH) {
//     lcdShow("PIR", "HUMAN");
//     server.send(200,"text/plain","1");
//   } else {
//     lcdShow("PIR", "NO HUMAN");
//     server.send(200,"text/plain","0");
//   }

// });


server.on("/pir",[](){

  int v = digitalRead(PIR);

  if (v == HIGH) {
    lcdShow("PIR", "HUMAN");
    server.send(200,"text/plain","1");
  } else {
    lcdShow("PIR", "NO HUMAN");
    server.send(200,"text/plain","0");
  }

});

server.on("/airData", [](){
  String json = "{\"value\":\""+String(airValue)+"\",\"status\":\""+airStatus+"\"}";
  server.send(200,"application/json",json);
});


// AUTO DATA
server.on("/autoData",[](){
 float d=getDistance();
 String blocked="0";
 if(d>0 && d<10) blocked="1";

 String json="{\"distance\":\""+String(d)+"\",\"blocked\":\""+blocked+"\"}";
 server.send(200,"application/json",json);
});

server.begin();
}

// ===== LOOP =====
void loop() {
server.handleClient();

// PIR continuous (unchanged)



// ===== PIR TIMED CONTROL =====
// if (millis() - pirTimer >= (pirActive ? 1000 : 5000)) {

//   pirTimer = millis();
//   pirActive = !pirActive;

//   if (pirActive) {
//     // PIR ACTIVE (1 second)
//     int motion = digitalRead(PIR);


// // START RELAY TIMER
//   if (motion == HIGH && !pirRelayRunning) {
//     pirRelayRunning = true;
//     pirRelayStart = millis();

//     digitalWrite(RELAY2, HIGH);
//     lcdShow("PIR", "HUMAN");
//   }

// // KEEP RELAY ON FOR 3 SECONDS
//   if (pirRelayRunning) {
//     if (millis() - pirRelayStart >= 3000) {
//       digitalWrite(RELAY2, LOW);
//       pirRelayRunning = false;
//       lcdShow("PIR", "OFF");
//     }
//   }

//   } else {
//     // PIR OFF (7 seconds)
//     digitalWrite(RELAY2, LOW);
//     lcdShow("PIR", "SLEEP");
//   }
// }



// ✅ KEEP RELAY ON FOR FULL 3 SECONDS (independent)
// if (pirRelayRunning) {
//   if (millis() - pirRelayStart >= 3000) {
//     digitalWrite(RELAY2, LOW);
//     pirRelayRunning = false;
//     lcdShow("PIR", "OFF");
//   }
// }


// ===== AIR QUALITY LOOP =====
if (millis() - airTimer > 3000) {
  airTimer = millis();
  updateAirQuality();
}



// ===== PIR DIRECT CONTROL (NO TIMER) =====
int motion = digitalRead(PIR);

// 🔥 START TIMER WHEN HUMAN DETECTED
if (motion == HIGH && !pirRelayRunning) {
  pirRelayRunning = true;
  pirRelayStart = millis();

  digitalWrite(RELAY2, LOW);  // OFF
  lcdShow("PIR", "HUMAN DETECTED");
}

// 🔥 KEEP RELAY OFF FOR LONGER TIME (CHANGE HERE)
if (pirRelayRunning) {
  if (millis() - pirRelayStart >= 8000) {  // ⬅️ CHANGE TIME HERE (8000 = 8 sec)
    pirRelayRunning = false;
  }
}

// 🔥 AFTER DELAY → TURN RELAY ON
if (!pirRelayRunning && motion == LOW) {
  digitalWrite(RELAY2, HIGH);  // ON
  lcdShow("PIR", "NO HUMAN");
}

// AUTO
if(autoMode && millis()-autoTimer>400){
 autoTimer = millis();
 speedValue = 250;   // 🔥 AUTO SPEED FIX

 autoNavigateSmart();
}
}
void autoNavigateSmart(){

  long front = getDistance();

  if(front <= 0){
    forward();
    return;
  }

  if(front > 30){
    forward();
    return;
  }

  // OBJECT
  stopRobot();
  delay(200);

  // ONLY RIGHT TURN
  right();
  delay(2000);
  stopRobot();
}