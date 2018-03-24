/*
  Simple web configuration portal
  When it is connected to a WiFi network, everything is fine
  When connection fails, it create a portal to insert a new SSID and password,
  If the new credentials works, it will be stored in ESP32 non volatile storage (NVS).
  In a nutshell: it's a https://github.com/tzapu/WiFiManager for ESP32 with a non blocking implementation and simple design.
  Connect to wifi network generated by ESP32 and tip in your browser:
  AP IPv4: 192.168.4.1  or  AP IPv6
  by Evandro Luis Copercini - 2017
  Public Domain
*/

/*
  TODO: integrate with https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiScan/WiFiScan.ino
*/

#include "WiFi.h"
#include <Preferences.h>

#define AP_SSID  "learnsteamiots"       //can set ap hostname here
#define AP_PASSWD "atobot2013"

int led_pin           = 2;
int relay_pin         = 2;

WiFiServer server(80);
Preferences preferences;
static volatile bool wifi_connected = false;
String wifiSSID, wifiPassword;


void WiFiEvent(WiFiEvent_t event)
{
  /* 
   *  ipv6 enabled cause connection fails when 2018/01/15, updated by atto.jeon@gmail.com 
  */
  switch (event) {

    case SYSTEM_EVENT_AP_START:
      //can set ap hostname here
      WiFi.softAPsetHostname(AP_SSID);
      //enable ap ipv6 here               
      //WiFi.softAPenableIpV6();
      break;

    case SYSTEM_EVENT_STA_START:
      //set sta hostname here
      WiFi.setHostname(AP_SSID);
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      //enable sta ipv6 here
      //WiFi.enableIpV6();
      break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
      //both interfaces get the same event
      //Serial.print("STA IPv6: ");
      //Serial.println(WiFi.localIPv6());
      //Serial.print("AP IPv6: ");
      //Serial.println(WiFi.softAPIPv6());
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println(">>> wifi connected.");
      wifiOnConnect();
      wifi_connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println(">>> wifi disconnected.");
      wifi_connected = false;
      wifiOnDisconnect();
      break;
    default:
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);      // set the LED pin mode
  pinMode(relay_pin, OUTPUT);

  delay(10);
  
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAP(AP_SSID, AP_PASSWD);
  Serial.println("AP Started");
  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IPv4: ");
  Serial.println(WiFi.softAPIP());

  /* 
   *  NVS에 저정된 wifi ssid/pwd로 시작한다.
   */
  /*
  preferences.begin("wifi", false);
  wifiSSID =  preferences.getString("ssid", "none");           //NVS key ssid
  wifiPassword =  preferences.getString("password", "none");   //NVS key password
  preferences.end();
  Serial.print("Stored SSID: ");
  Serial.println(wifiSSID);

  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  */
  server.begin();
}

void loop()
{
  if (wifi_connected) {
    wifiConnectedLoop();
  } else {
    wifiDisconnectedLoop();
  }
}

//when wifi connects
void wifiOnConnect()
{
  Serial.println("STA Connected");
  Serial.print("STA SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("STA IPv4: ");
  Serial.println(WiFi.localIP());
  //Serial.print("STA IPv6: ");
  //Serial.println(WiFi.localIPv6());
  WiFi.mode(WIFI_MODE_STA);           //close AP network
}

//when wifi disconnects
void wifiOnDisconnect()
{
  //Serial.println("STA Disconnected");
  delay(1000);
  //WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  WiFi.begin(AP_SSID, AP_PASSWD);
}

//while wifi is connected
void wifiConnectedLoop()
{
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  delay(1000);
}

void wifiDisconnectedLoop()
{
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(led_pin, HIGH);             // GET /H turns the LED on
          digitalWrite(relay_pin, HIGH);
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(led_pin, LOW);                // GET /L turns the LED off
          digitalWrite(relay_pin, LOW);
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }

  /*
  if (client) {                             // if you get a client,
    Serial.println("New client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor

        
        
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("<form method='get' action='a'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
          continue;
        }

        if (currentLine.startsWith("GET /a?ssid=") ) {
          //Expecting something like:
          //GET /a?ssid=blahhhh&pass=poooo
          Serial.println("");
          Serial.println("Cleaning old WiFi credentials from ESP32");
          // Remove all preferences under opened namespace
          preferences.clear();

          String qsid;
          qsid = currentLine.substring(12, currentLine.indexOf('&')); //parse ssid
          Serial.println(qsid);
          Serial.println("");
          String qpass;
          qpass = currentLine.substring(currentLine.lastIndexOf('=') + 1, currentLine.lastIndexOf(' ')); //parse password
          Serial.println(qpass);
          Serial.println("");

          preferences.begin("wifi", false); // Note: Namespace name is limited to 15 chars
          Serial.println("Writing new ssid");
          preferences.putString("ssid", qsid);

          Serial.println("Writing new pass");
          preferences.putString("password", qpass);
          delay(300);
          preferences.end();

          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();

          // the content of the HTTP response follows the header:
          client.print("<h1>OK! Restarting in 5 seconds...</h1>");
          client.println();
          Serial.println("Restarting in 5 seconds...");
          delay(5000);
          ESP.restart();
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
  */
}
