#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
ESP8266WebServer server(80); // Web server on port 80

int relay_state1 = LOW;
int relay_state2 = LOW;
// uncompiled, untested
const byte Relay_1_ON[] = {0xA0, 0x01, 0x01, 0xA2};
const byte Relay_1_OFF[] = {0xA0, 0x01, 0x00, 0xA1};
const byte Relay_2_ON[] = {0xA0, 0x02, 0x01, 0xA3};
const byte Relay_2_OFF[] = {0xA0, 0x02, 0x00, 0xA2};

String getHTML() {
  String html = "<!DOCTYPE HTML>";
  html += "<html>";
  html += "<head>";
  html += "<link rel='icon' href='data:,'>";
  html += "</head>";
  html += "<p>Relay state 1: <span style='color: red;'>";

  if (relay_state1 == LOW)
    html += "OFF";
  else
    html += "ON";

  html += "</span></p>";
  html += "<a href='/relay1/on'>Turn ON</a>";
  html += "<br><br>";
  html += "<a href='/relay1/off'>Turn OFF</a>";

  html += "<p>Relay state 2: <span style='color: red;'>";
  if (relay_state2 == LOW)
    html += "OFF";
  else
    html += "ON";

  html += "</span></p>";
  html += "<a href='/relay2/on'>Turn ON</a>";
  html += "<br><br>";
  html += "<a href='/relay2/off'>Turn OFF</a>";
  html += "</html>";

  return html;
}

void setup() {
    // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.
    delay(5000);
    // put your setup code here, to run once:
    Serial.begin(115200);
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        //Serial.println("connected...yeey :)");
        // home page
        server.on("/", HTTP_GET, []() {
          //Serial.println("ESP8266 Web Server: New request received:");
          //Serial.println("GET /");
          server.send(200, "text/html", getHTML());
        });

        // Route to control the Relay 1
        server.on("/relay1/on", HTTP_GET, []() {
          //Serial.println("ESP8266 Web Server: New request received:");
          //Serial.println("GET /relay1/on");
          Serial.write(Relay_1_ON, sizeof(Relay_1_ON));
          relay_state1 = HIGH;
          server.send(200, "text/html", getHTML());
        });
        server.on("/relay1/off", HTTP_GET, []() {
          //Serial.println("ESP8266 Web Server: New request received:");
          //Serial.println("GET /relay1/off");
          Serial.write(Relay_1_OFF, sizeof(Relay_1_OFF));
          relay_state1 = LOW;
          server.send(200, "text/html", getHTML());
        });

        // Route to control the Relay 2
        server.on("/relay2/on", HTTP_GET, []() {
          //Serial.println("ESP8266 Web Server: New request received:");
          //Serial.println("GET /relay2/on");
          Serial.write(Relay_2_ON, sizeof(Relay_2_ON));
          relay_state2 = HIGH;
          server.send(200, "text/html", getHTML());
        });
        server.on("/relay2/off", HTTP_GET, []() {
          //Serial.println("ESP8266 Web Server: New request received:");
          //Serial.println("GET /relay2/off");
          Serial.write(Relay_2_OFF, sizeof(Relay_2_OFF));
          relay_state2 = LOW;
          server.send(200, "text/html", getHTML());
        });

        // Start the server
        //Serial.println("connected...yeey :)");
        server.begin();
    }

}

void loop() {
    // put your main code here, to run repeatedly:   
  // Handle client requests
  server.handleClient();
}
