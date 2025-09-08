/*
MIT License

Copyright (c) 2025 Dony Mahardhika

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ---------------- WiFi ----------------
const char* ssid = "InspectionReminder";
const char* password = "12345678";

ESP8266WebServer server(80);

// ---------------- DFPlayer ----------------
SoftwareSerial mp3Serial(D5, D6);
DFRobotDFPlayerMini myDFPlayer;

// ---------------- Recipe Structure ----------------
struct Recipe {
  char name[20];
  uint8_t sequence; 
  unsigned int d12;
  unsigned int d23;
  unsigned int d34;
  unsigned int d4end;
  unsigned int intervalRest;
  unsigned int delayRest;
  unsigned int delayReady;
  uint8_t volume;
};

#define MAX_RECIPES 20
Recipe recipes[MAX_RECIPES];
int currentRecipe = 0;

// ---------------- Status ----------------
bool running = false;
bool autoRun = false;
unsigned long lastAction = 0;
int stepIndex = 0;
unsigned long restStart = 0;
unsigned long readyStart = 0;
bool resting = false;
unsigned long lastCycleStart = 0;

// ---------------- Role ----------------
int currentRole = 0; 
// 0=Guest,1=Staff,2=Supervisor,3=Admin

// ---------------- Password Role ----------------
String pwdStaff = "111222";
String pwdSupervisor = "333444";
String pwdAdmin = "555666";

// ---------------- EEPROM State ----------------
struct StateData {
  int lastRecipe;
  bool autoRunEnabled;
};
StateData stateData;

// ---------------- EEPROM ----------------
void loadRecipes() {
  EEPROM.begin(sizeof(recipes) + sizeof(stateData));
  EEPROM.get(0, recipes);
  EEPROM.end();

  for (int i = 0; i < MAX_RECIPES; i++) {
    if (recipes[i].name[0] == '\0') {
      snprintf(recipes[i].name, sizeof(recipes[i].name), "Recipe %d", i+1);
      recipes[i].volume = 20;
    }
  }
}

void saveRecipes() {
  EEPROM.begin(sizeof(recipes) + sizeof(stateData));
  EEPROM.put(0, recipes);
  EEPROM.commit();
  EEPROM.end();
}

// Save state
void saveState() {
  EEPROM.begin(sizeof(recipes) + sizeof(stateData));
  EEPROM.put(sizeof(recipes), stateData);
  EEPROM.commit();
  EEPROM.end();
}

// Read state
void loadState() {
  EEPROM.begin(sizeof(recipes) + sizeof(stateData));
  EEPROM.get(sizeof(recipes), stateData);
  EEPROM.end();

  if (stateData.lastRecipe < 0 || stateData.lastRecipe >= MAX_RECIPES) {
    stateData.lastRecipe = 0;
    stateData.autoRunEnabled = false;
  }
  currentRecipe = stateData.lastRecipe;
  autoRun = stateData.autoRunEnabled;
}

// ---------------- Execution ----------------
void playTrack(uint8_t track) {
  myDFPlayer.play(track);
  Serial.print("Play: "); Serial.println(track);
}

void processRecipe() {
  if (!running) return;
  Recipe &rec = recipes[currentRecipe];
  unsigned long now = millis();

  if (resting) {
    if (readyStart > 0) {
      if (now - readyStart >= (rec.delayReady * 1000UL)) {
        resting = false;
        readyStart = 0;
        stepIndex = 0;
        lastAction = now;
      }
    } else {
      if (now - restStart >= (rec.delayRest * 1000UL)) {
        readyStart = now;
        playTrack(6);
      }
    }
    return;
  }

  switch (stepIndex) {
    case 0: playTrack(1); stepIndex=1; lastAction=now; break;
    case 1:
      if (now - lastAction >= rec.d12*1000UL) {
        playTrack(rec.sequence==1 ? 2 : 3);
        stepIndex=2; lastAction=now;
      }
      break;
    case 2:
      if (now - lastAction >= rec.d23*1000UL) {
        playTrack(rec.sequence==1 ? 3 : 2);
        stepIndex=3; lastAction=now;
      }
      break;
    case 3:
      if (now - lastAction >= rec.d34*1000UL) {
        playTrack(4); stepIndex=4; lastAction=now;
      }
      break;
    case 4:
      if (now - lastAction >= rec.d4end*1000UL) {
        if (rec.intervalRest>0 && (now-lastCycleStart>=rec.intervalRest*1000UL)) {
          playTrack(5);
          resting=true; restStart=now; lastCycleStart=now;
        }
        stepIndex=0; lastAction=now;
      }
      break;
  }
}

void stopSequence() {
  running = false; resting=false; autoRun=false;
}

// ---------------- HTML Template ----------------
String styleCSS() {
  String css = "<style>";
  css += "body{font-family:sans-serif;text-align:center;margin:20px;background:#f4f4f9;}";
  css += "h2{margin-bottom:20px;}";
  css += ".card{background:#fff;padding:15px;margin:15px auto;border-radius:10px;box-shadow:0 2px 5px rgba(0,0,0,0.2);max-width:400px;text-align:left;}";
  css += ".card h3{text-align:center;margin-bottom:15px;color:#333;}";
  css += "label{font-weight:bold;display:block;margin-top:10px;text-align:left;}";
  css += "input,select{width:100%;padding:8px;margin-top:5px;border:1px solid #ccc;border-radius:5px;font-size:16px;}";
  css += "input[type='submit'],button{padding:10px;margin-top:10px;border:none;border-radius:8px;color:white;cursor:pointer;width:100%;font-size:16px;}";
  css += ".btn-green{background:#4CAF50;} .btn-green:hover{background:#45a049;}";
  css += ".btn-blue{background:#2196f3;} .btn-blue:hover{background:#0b7dda;}";
  css += ".btn-red{background:#f44336;} .btn-red:hover{background:#d32f2f;}";
  css += "</style>";
  return css;
}

// ---------------- Home Page ----------------
void handleRoot() {
  Recipe &rec = recipes[currentRecipe];
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += styleCSS();
  html += "</head><body>";
  html += "<h2>Inspection Reminder</h2>";

  // Status
  html += "<div class='card'><h3>Status</h3>";
  html += "<p>Active Recipe: "+String(rec.name)+"</p>";
  html += "<p>Active Sequence: "+String((rec.sequence==1)?"Amber":"Flint")+"</p>";
  html += "<p>Auto-run: "+String(autoRun?"ON":"OFF")+"</p>";
  html += "<p>Volume: "+String(rec.volume)+"/30</p>";
  html += "</div>";

  // Staff +
  if (currentRole>=1) {
    html += "<div class='card'><h3>Recipe Selection</h3>";
    html += "<form action='/setrecipe' method='get'><select name='r'>";
    for (int i=0;i<MAX_RECIPES;i++) {
      html += "<option value='"+String(i)+"'";
      if (i==currentRecipe) html+=" selected";
      html += ">"+String(recipes[i].name)+"</option>";
    }
    html += "</select><input type='submit' value='Select' class='btn-green'></form>";
    html += "</div>";
  }

  // Supervisor +
  if (currentRole>=2) {
    html += "<div class='card'>";
    html += "<a href='/edit'><button class='btn-blue'>Edit Recipe</button></a>";
    html += "</div>";
  }

  // Staff + Supervisor + Admin +
  if (currentRole==1 || currentRole==2 || currentRole==3) {
    html += "<div class='card'><h3>Controls</h3>";
    html += "<a href='/run'><button class='btn-green'>Run</button></a>";
    html += "<a href='/stop'><button class='btn-red'>Stop</button></a>";
    html += "</div>";
  }

  // Admin
  if (currentRole==3) {
    html += "<div class='card'>";
    html += "<a href='/changepwd'><button class='btn-blue'>Change Passwords</button></a>";
    html += "</div>";
  }

  // Login/Logout
  html += "<div class='card'>";
  if (currentRole==0) {
    html += "<a href='/login'><button class='btn-blue'>Login</button></a>";
  } else {
    html += "<a href='/logout'><button class='btn-red'>Logout</button></a>";
  }
  html += "</div>";

  html += "</body></html>";
  server.send(200,"text/html",html);
}

void handleLogin() {
  String html="<html><head>"+styleCSS()+"</head><body>";
  html += "<h2>Inspection Reminder</h2>";
  html+="<div class='card'><h2>Login</h2>";
  html+="<form action='/checklogin' method='post'>";
  html+="<label>Password:</label><input type='password' name='pwd'>";
  html+="<input type='submit' value='Login' class='btn-green'></form>";
  html+="<a href='/'><button class='btn-blue'>Cancel</button></a></div></body></html>";
  server.send(200,"text/html",html);
}

void handleCheckLogin() {
  if (!server.hasArg("pwd")) {handleLogin();return;}
  String p=server.arg("pwd");
  if (p==pwdStaff) currentRole=1;
  else if (p==pwdSupervisor) currentRole=2;
  else if (p==pwdAdmin) currentRole=3;
  else { currentRole=0; }
  server.sendHeader("Location","/");
  server.send(303);
}

void handleLogout() {
  currentRole=0;
  server.sendHeader("Location","/");
  server.send(303);
}

// Supervisor: edit recipe
void handleEdit() {
  if (currentRole<2) {server.sendHeader("Location","/");server.send(303);return;}
  Recipe &rec=recipes[currentRecipe];
  String html="<html><head>"+styleCSS()+"</head><body>";
  html+="<h2>Inspection Reminder</h2><form action='/save' method='get'>";
  html+="<div class='card'><h3>Edit Recipe</h3>";
  html+="<label>Name:</label>max. 19 character<input type='text' name='name' value='"+String(rec.name)+"' maxlength='19'>";
  html+="<label>Sequence:</label><select name='seq'>";
  html+="<option value='1'"; if(rec.sequence==1)html+=" selected"; html+=">Amber (Putih-Hitam)</option>";
  html+="<option value='2'"; if(rec.sequence==2)html+=" selected"; html+=">Flint (Hitam-Putih)</option></select>";
  html+="<label>Volume:</label>max. 30<select name='vol'>";
          for (int i=1; i<=30; i++) {
            html+="<option value='"+String(i)+"'";
            if (i==rec.volume) html+=" selected";
            html+=">"+String(i)+"</option>";
          }
  html+="</select>";
  html+="</div>";
  html+="<div class='card'><h3>Delays (second)</h3>";
  html+="<label>Ambil:</label><input type='number' name='d12' value='"+String(rec.d12)+"'>";
  html+="<label>Putih:</label><input type='number' name='d23' value='"+String(rec.d23)+"'>";
  html+="<label>Hitam:</label><input type='number' name='d34' value='"+String(rec.d34)+"'>";
  html+="<label>Letakkan:</label><input type='number' name='d4end' value='"+String(rec.d4end)+"'>";
  html+="<label>Interval Istirahat:</label><input type='number' name='int' value='"+String(rec.intervalRest)+"'>";
  html+="<label>Istirahat:</label><input type='number' name='rest' value='"+String(rec.delayRest)+"'>";
  html+="<label>Siap-siap:</label><input type='number' name='ready' value='"+String(rec.delayReady)+"'>";
  html+="<input type='submit' value='Save' class='btn-green'></form>";
  html+="<a href='/'><button class='btn-blue'>Cancel</button></a>";
  html+="</div></body></html>";
  server.send(200,"text/html",html);
}

void handleSave() {
  if (currentRole<2) {server.sendHeader("Location","/");server.send(303);return;}
  Recipe &rec=recipes[currentRecipe];
  if (server.hasArg("name")) server.arg("name").toCharArray(rec.name,sizeof(rec.name));
  rec.sequence=server.arg("seq").toInt();
  rec.volume=server.arg("vol").toInt();
  rec.d12=server.arg("d12").toInt();
  rec.d23=server.arg("d23").toInt();
  rec.d34=server.arg("d34").toInt();
  rec.d4end=server.arg("d4end").toInt();
  rec.intervalRest=server.arg("int").toInt();
  rec.delayRest=server.arg("rest").toInt();
  rec.delayReady=server.arg("ready").toInt();
  saveRecipes();
  myDFPlayer.volume(rec.volume);
  server.sendHeader("Location","/");
  server.send(303);
}

// Admin: change password
void handleChangePwd() {
  if (currentRole<3) {server.sendHeader("Location","/");server.send(303);return;}
  String html="<html><head>"+styleCSS()+"</head><body>";
  html+="<h2>Inspection Reminder</h2><form action='/savepwd' method='post'>";
  html+="<div class='card'><h3>Change Passwords</h3>";
  html+="<label>Staff:</label><input type='text' name='staff' value='"+pwdStaff+"'>";
  html+="<label>Supervisor:</label><input type='text' name='spv' value='"+pwdSupervisor+"'>";
  html+="<label>Admin:</label><input type='text' name='adm' value='"+pwdAdmin+"'>";
  html+="<input type='submit' value='Save' class='btn-green'></form>";
  html+="<a href='/'><button class='btn-blue'>Cancel</button></a>";
  html+="</div></body></html>";
  server.send(200,"text/html",html);
}

void handleSavePwd() {
  if (currentRole<3) {server.sendHeader("Location","/");server.send(303);return;}
  if (server.hasArg("staff")) pwdStaff=server.arg("staff");
  if (server.hasArg("spv")) pwdSupervisor=server.arg("spv");
  if (server.hasArg("adm")) pwdAdmin=server.arg("adm");
  server.sendHeader("Location","/");
  server.send(303);
}

// ---------------- Run/Stop ----------------
void handleRun() {
  if (currentRole<1) {server.sendHeader("Location","/");server.send(303);return;}
  running=true; stepIndex=0; lastAction=millis(); lastCycleStart=millis();
  autoRun=true;
  stateData.lastRecipe = currentRecipe;
  stateData.autoRunEnabled = true;
  saveState();
  myDFPlayer.volume(recipes[currentRecipe].volume);
  server.sendHeader("Location","/"); server.send(303);
}

void handleStop() {
  if (currentRole<1) {server.sendHeader("Location","/");server.send(303);return;}
  stopSequence();
  autoRun=false;
  stateData.lastRecipe = currentRecipe;
  stateData.autoRunEnabled = false;
  saveState();
  server.sendHeader("Location","/"); server.send(303);
}

void handleSetRecipe() {
  if (currentRole<1) {server.sendHeader("Location","/");server.send(303);return;}
  currentRecipe=server.arg("r").toInt();
  if (currentRecipe<0) currentRecipe=0;
  if (currentRecipe>=MAX_RECIPES) currentRecipe=MAX_RECIPES-1;
  myDFPlayer.volume(recipes[currentRecipe].volume);

  stateData.lastRecipe = currentRecipe;
  saveState();

  handleRoot();
}

// ---------------- Setup & Loop ----------------
void setup() {
  Serial.begin(115200);
  mp3Serial.begin(9600);
  if (!myDFPlayer.begin(mp3Serial)) {
    Serial.println("DFPlayer not found!"); while(true);
  }

  loadRecipes();
  loadState();
  myDFPlayer.volume(recipes[currentRecipe].volume);

  if (autoRun) {
    running = true;
    stepIndex = 0;
    lastAction = millis();
    lastCycleStart = millis();
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid,password);
  Serial.println("AP started: "+String(ssid));

  server.on("/",handleRoot);
  server.on("/login",handleLogin);
  server.on("/checklogin",handleCheckLogin);
  server.on("/logout",handleLogout);
  server.on("/setrecipe",handleSetRecipe);
  server.on("/run",handleRun);
  server.on("/stop",handleStop);
  server.on("/edit",handleEdit);
  server.on("/save",handleSave);
  server.on("/changepwd",handleChangePwd);
  server.on("/savepwd",handleSavePwd);

  server.begin();
  Serial.println("Webserver ready 192.168.4.1");
}

void loop() {
  server.handleClient();
  processRecipe();
}
