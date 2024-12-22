/*
 * Normally open relays assumed.
 * HIGH (bool:true)   close the circuit -> turn on
 * LOW  (bool: false) open the circuit  -> turn off
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>


#define RELAYS_NUM      4
#define RELAY_0         5   //D1 (GPIO 5)
#define RELAY_1         4   //D2 (GPIO 4)
#define RELAY_2         14  //D5 (GPIO 14)
#define RELAY_3         12  //D6 (GPIO 12)


//Wifi params
//const char* ssid = "Hoang Anh";
//const char* password = "999999999";
//Variables to save values from HTML form
String ssid;
String pass;
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";

// Async Web Server + websocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


// State variables for each relay
bool r0 = false;
bool r1 = false;
bool r2 = false;
bool r3 = false;
bool restart = false;

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
  file.close();
}

String boolToState(bool b){
  if(b) return "on";
  else  return "off";
}


void resolveConnect(uint32_t clientID){
  String msg = "r0";
  msg += boolToState(r0); 
  ws.text(clientID, msg);
  msg = "r1";
  msg += boolToState(r1);  
  ws.text(clientID, msg);
  msg = "r2";
  msg += boolToState(r2);  
  ws.text(clientID, msg);
  msg = "r3";
  msg += boolToState(r3);  
  ws.text(clientID, msg);
}


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;
    Serial.println(message);
    if(message.indexOf("toggle-r0") >= 0){
      r0 = !r0;
      if(!r0){
        //digitalWrite(RELAY_0, LOW);
        Serial.print("toggle0-r0");
      } else {
        //digitalWrite(RELAY_0, HIGH);
        Serial.print("toggle1-r0");
      }
      String msg = "r0";
      msg += boolToState(r0);
      ws.textAll(msg);
    }
    else if(message.indexOf("toggle-r1") >= 0){
      r1 = !r1;
      if(!r1){
        //digitalWrite(RELAY_1, LOW);
        Serial.print("toggle0-r1");
      } else {
        //digitalWrite(RELAY_1, HIGH);
        Serial.print("toggle1-r1");
      }
      String msg = "r1";
      msg += boolToState(r1);
      ws.textAll(msg);    
    }
    else if(message.indexOf("toggle-r2") >= 0){
      r2 = !r2;
      if(!r2){
        //digitalWrite(RELAY_2, LOW);
        Serial.print("toggle0-r2");
      } else {
        //digitalWrite(RELAY_2, HIGH);
        Serial.print("toggle1-r2");
      }
      String msg = "r2";
      msg += boolToState(r2);  
      ws.textAll(msg);    
    }
    else if(message.indexOf("toggle-r3") >= 0){
      r3 = !r3;
      if(!r3){
        //digitalWrite(RELAY_3, LOW);
        Serial.print("toggle0-r3");
      } else {
        //digitalWrite(RELAY_0, HIGH);
        Serial.print("toggle1-r3");
      }
      String msg = "r3";
      msg += boolToState(r3);
      ws.textAll(msg);    
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      //Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      resolveConnect(client->id());
      break;
    case WS_EVT_DISCONNECT:
      //Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

// Initialize WiFi
bool initWiFi() {
  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  if(ssid==""){
    Serial.println("Undefined SSID.");
    return false;
  }

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  Serial.println("connected");
  Serial.println(WiFi.localIP());
  return true;
}


void setup() {
  delay(2000);
  Serial.begin(115200);
  Serial.println("Start");
  //Set relay pins as output
  //pinMode(RELAY_0, OUTPUT);
  //pinMode(RELAY_1, OUTPUT);
  //pinMode(RELAY_2, OUTPUT);
  //pinMode(RELAY_3, OUTPUT);

  //N.O. configurations: HIGH -> close the circuit, LOW -> open the circuit
  //digitalWrite(RELAY_0, LOW);
  //digitalWrite(RELAY_1, LOW);
  //digitalWrite(RELAY_2, LOW);
  //digitalWrite(RELAY_3, LOW);

  
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }

  ws.onEvent(onEvent);
  server.addHandler(&ws);
  if(initWiFi()) {
    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("here\r\n");
      request->send(LittleFS, "/index.html", "text/html");
    });
    server.serveStatic("/", LittleFS, "/");

    // HTTP Requests for relays configuration
    server.on("/all", HTTP_GET, [](AsyncWebServerRequest *request){
      String relays = "";
      relays.reserve(2000);
      for(uint8_t i=0; i < RELAYS_NUM; i++) {
        relays += "<div class='button-div'><h2>Relay ";
        relays += i;
        relays += "</h2><button class='toggle-button' id='r";
        relays += i;
        relays += "'>";
        relays += "</button></div>";
      }
      request->send(200, "text/plain", relays);
    });

    server.on("/r0", HTTP_GET, [](AsyncWebServerRequest *request){
      String relay = "<div class='button-div'><button class='toggle-button' id='r0'></button></div>";
      request->send(200, "text/plain", relay);
    });

    server.on("/r1", HTTP_GET, [](AsyncWebServerRequest *request){
      String relay = "<div class='button-div'><button class='toggle-button' id='r1'></button></div>";
      request->send(200, "text/plain", relay);
    });

    server.on("/r2", HTTP_GET, [](AsyncWebServerRequest *request){
      String relay = "<div class='button-div'><button class='toggle-button' id='r2'></button></div>";
      request->send(200, "text/plain", relay);
    });

    server.on("/r3", HTTP_GET, [](AsyncWebServerRequest *request){
      String relay = "<div class='button-div'><button class='toggle-button' id='r3'></button></div>";
      request->send(200, "text/plain", relay);
    });
    // Start server
    Serial.println("Start server");
    server.begin();
  } else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", LittleFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == "ssid") {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == "pass") {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
          }
        }
      }
      restart = true;
      request->send(200, "text/plain", "Done. ESP will restart.");
    });
    server.begin();
  }
}

void loop() {}
