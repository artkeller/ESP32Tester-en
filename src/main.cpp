//2024-8-11 21:41:32
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

//#include "LittleFS.h"
#include "SPIFFS.h"

#include <HTTPUpdate.h>

#include <PubSubClient.h>


/*Test section*/
/*Test section - end*/

//Custom libraries-----------------------------------
#include "lib/ws2812.h"
#include "lib/sr04.h"
#include "lib/switch.h"
#include "lib/oled.h"
#include "lib/dht.h"
#include "lib/ir.h"

//Variables
#include "common.h"

//Functions
#include "func.h"
//-------------------------------------------


//////////////////////////////


////////////////////////////////////////////////////////
void setup() {
  Serial.begin(intSerial);  

  /*Test section*/
  /*Test section - end*/

  //Initialize SPIFFS
  initFS();

  //---------------------------
  //Read content from FS - system configuration
  File f = SPIFFS.open(sysPath, "r");
  strTmp = f.readString();

  int count = 1;
  int i = 0;
  for (i = 0; i < strTmp.length(); i++) {
      if (strTmp.charAt(i) == '|') {
          count++; // Encounter separator, increment counter
      }
  }  
  
  char charArray[strTmp.length() + 1];
  strTmp.toCharArray(charArray, sizeof(charArray));

  // Use strtok to split string by | and store in variables
  char* token = strtok(charArray, "|");
  char* tokens[count];
  
  i = 0;
  while (token != NULL && i < count) {
      tokens[i++] = token;
      token = strtok(NULL, "|");
  }

  ssid    = tokens[0];
  pass    = tokens[1];
  ip      = tokens[2];
  gateway = tokens[3];
  ota     = tokens[4];
  otahost = tokens[5];
  checkid = tokens[6];

  sinfo("ssid=",    ssid);
  sinfo("pass=",    pass);
  sinfo("ip=",      ip);
  sinfo("gateway=", gateway);
  sinfo("ota=",     ota);
  sinfo("otahost=", otahost);
  sinfo("checkid=", checkid);
  //
  server.serveStatic("/", SPIFFS, "/");

  //Scan for network
  Serial.println("Search Wifi");
  ///////////////
  if(initWiFi()) {

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });


    server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request) {
      sinfo("begin ota", OTAFSURL);

      ws.textAll("Start OTA");
      
      ota = "1";
      writeFile(SPIFFS, "sys");

      ESP.restart();
    });    

    server.on("/autofs", HTTP_GET, [](AsyncWebServerRequest *request) {
      sinfo("begin ota fs", OTAFSURL);

      ws.textAll("Start OTA FS");
      
      ota = "2";
      writeFile(SPIFFS, "sys");

      ESP.restart();
    });
    
    // Reboot
    server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/html", "<script>alert('Restarting, please refresh page after 5 seconds.');</script>");
      
      ESP.restart();
    });
    
    // Reset
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
      ssid = "";
      writeFile(SPIFFS, "sys");
      ESP.restart();      
      
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });

    server.on("/otahost", HTTP_POST, [](AsyncWebServerRequest *request){
      // Get submitted form content
      if (request->hasArg("otahost")) {
        otahost = request->arg("otahost");
        writeFile(SPIFFS, "sys");

        sinfo("otahost: ", otahost);
      }
  
      request->send(200, "text/html",  "<script>alert('OTA host modified. You can restart and check if it works.')</script>");
    });

    //DHT11 via HTTP request
    server.on("/dht", HTTP_GET, [](AsyncWebServerRequest *request){
      dhtRun = 1;

      sinfo(dhtInfo);

      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", dhtInfo);
      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
      response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
      request->send(response);

      //request->send(200, "text/plain", dhtInfo);
    });

    //OLED via HTTP request
    server.on("/oled", HTTP_GET, [](AsyncWebServerRequest *request){
      oledRun = 1;

      //CORS headers
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Check if display works");
      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
      response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
      request->send(response);      
    });


    
    server.begin();
    canTest = 1;

    ///////////
    // Initialize OTA
    
    if(ota == "1") updateOTA();
    if(ota == "2") updateOTAFS();

    if(ota == "1" || ota == "2"){
      ota = "0";
      writeFile(SPIFFS, "sys");
      ESP.restart();
    }
    
    sinfo(BOARD, VERSION);

    /////////////
    //Wifi found
    Serial.println("Search Wifi Success");
  } else {
    sinfo("Setting AP (Access Point)");
    
    //AP name
    strTmp = "ZJY-" + String(ESP.getEfuseMac());
    WiFi.softAP(strTmp, "123456");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });

    //Direct connection
    server.on("/c", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });    
    
    server.serveStatic("/", SPIFFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      if (request->hasArg("ssid"))    ssid    = request->arg("ssid");
      if (request->hasArg("pass"))    pass    = request->arg("pass");
      if (request->hasArg("ip"))      ip      = request->arg("ip");
      if (request->hasArg("gateway")) gateway = request->arg("gateway");
      if (request->hasArg("checkid")) checkid = request->arg("checkid");
      writeFile(SPIFFS, "sys");

      request->send(200, "text/html",  "<html><meta http-equiv='Content-Type' content='text/html; charset=UTF-8'/><body><h1>Write completed, restarting. Modified IP: <a href='http://"+ip+"'>"+ip+" Click to access</a></h1></body></html>");

      restart = true;
    });

    server.begin();
  }

  ws.onEvent(onEventHandle); // Bind callback function
  server.addHandler(&ws);    // Add WebSocket to server
  server.on("/ws", HTTP_GET, handleRoot); //Register route "/ws" with corresponding callback function
  server.begin(); //Start server
  delay(1000);
  ws.textAll("websocket server On"); // Send data to all connected clients

  //
  irrecv.enableIRIn(); // Start infrared receiver


}

void loop() {
  ws.cleanupClients();     // Clean up excessive WebSocket connections to save resources

  if (restart){
    delay(5000);
    ESP.restart();
  }
  
  //This will execute during normal operation
  if(canTest==1){
    //Serial.println("running version 1.0.0");
    delay(1000);
  }

  if(dhtRun == 1){
    getDHT();
    dhtRun = 0;
  }

  if(oledRun == 1){
    getOLED();
    oledRun = 0;
  }

  /*Test start*/
  if (irrecv.decode(&results)) { // If infrared signal received
    Serial.println(results.value, HEX); // Output infrared signal value (hex format)
    irrecv.resume(); // Continue receiving next infrared signal

    ws.textAll("Button code:"+String(results.value));
  }
  /*Test end*/
}
