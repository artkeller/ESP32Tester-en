//Debug display
void sinfo(String str, String strInfo=""){
  if (SHOWINFO == 1) {
    if (strInfo == ""){
      Serial.println(str);
    }else{
      Serial.println(str + "" + strInfo);
    }
  }

  //  sprintf(output, "Sensor value: %d, Temperature: %.2f", sensorValue, temperature);
}

//Initialize FileSystem
void initFS() {
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount Error");
  }
  else{
    Serial.println("SPIFFS mounted Successfully");
  }
}

//Write file content
void writeFile(fs::FS &fs, String type){
  String path = sysPath;
  String m    = ssid + "|" + pass + "|" + ip + "|" + gateway + "|" + ota + "|" + otahost + "|" + checkid;

  /*
  if(type=="soft") {
    path   = softPath;
    m =  String(intBright) + "|" +  String(mType) + "|" + biliID + "|" + wUserKey + "|" + wLocation;
  } 
  */

  sinfo("Writing:", m);

  File file = fs.open(path, "w");
  if(!file){
    sinfo("- failed to open file for writing");
    return;
  }

  if(file.print(m)){
    sinfo("- file written");
  } else {
    sinfo("- write failed");
  }
  
  file.close();
}

//Read file content
String readFile(fs::FS &fs, const char * path){
  sinfo("Reading file:", path);

  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    sinfo("- failed to open file for reading");
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

//////////////////////////////
//OTA Online Update Section
void update_started() {
  Serial.println("HTTP update process started");
}

void update_finished() {
  Serial.println("HTTP update process finished");
}

void update_progress(int cur, int total) {
  Serial.printf("HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("HTTP update fatal error code %d\n", err);
}

void updateOTA() {
  httpUpdate.setLedPin(LED_BUILTIN, LOW);

    // Add optional callback notifiers
    httpUpdate.onStart(update_started);
    httpUpdate.onEnd(update_finished);
    httpUpdate.onProgress(update_progress);
    httpUpdate.onError(update_error);
    httpUpdate.rebootOnUpdate(false); // remove automatic update
      
    t_httpUpdate_return ret = httpUpdate.update(espClient, otahost + OTAURL);

    ota = "0";
    writeFile(SPIFFS, "sys");

    switch (ret) {
      case HTTP_UPDATE_FAILED: Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str()); break;
      case HTTP_UPDATE_NO_UPDATES: Serial.println("HTTP_UPDATE_NO_UPDATES"); break;
      case HTTP_UPDATE_OK: Serial.println("HTTP_UPDATE_OK"); break;
    }  

    ESP.restart();
}

void updateOTAFS() {
  httpUpdate.setLedPin(LED_BUILTIN, LOW);

  // Add optional callback notifiers
  httpUpdate.onStart(update_started);
  httpUpdate.onEnd(update_finished);
  httpUpdate.onProgress(update_progress);
  httpUpdate.onError(update_error);
  httpUpdate.rebootOnUpdate(false); // remove automatic update
  
  t_httpUpdate_return ret = httpUpdate.updateSpiffs(espClient, otahost + OTAFSURL);
  
  ota = "0";
  writeFile(SPIFFS, "sys");

  switch (ret) {
    case HTTP_UPDATE_FAILED: Serial.printf("FS HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str()); break;
    case HTTP_UPDATE_NO_UPDATES: Serial.println("FS HTTP_UPDATE_NO_UPDATES"); break;
    case HTTP_UPDATE_OK: Serial.println("FS HTTP_UPDATE_OK"); break;
  }  
}

//////////////////////////////
// Initialize WIFI
bool initWiFi() {
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.println("Connecting to WiFi...");

  int intLoop = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");

    intLoop+=1;
    if(intLoop > 40){
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}

// Display various effects on web page
String processor(const String& var) {
  //Serial.println(var);

  if(var == "VERSION")      return VERSION;
  if(var == "BOARD")        return BOARD;
  if(var == "OTAHOST")      return otahost;
  if(var == "IP")           return ip.c_str();
  
  return String();
}

//--------------------------------------------------------------
//WebSocket Section----------------
//Callback function
void handleRoot(AsyncWebServerRequest *request) {
  Serial.println("User requested.");
  request->send(200, "html", "<p>Hello World!</p>"); //Send response and content to client
}

// WebSocket event callback function
void onEventHandle(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  if (type == WS_EVT_CONNECT) // Client connected
  {
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u !", client->id()); // Send data to client
    client->ping();                                    // Send ping to client
  }
  else if (type == WS_EVT_DISCONNECT) // Client disconnected
  {
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  }
  else if (type == WS_EVT_ERROR) // Error occurred
  {
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
  }
  else if (type == WS_EVT_PONG) // Received client's pong response to server ping
  {
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
  }
  else if (type == WS_EVT_DATA) // Received data from client
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);
    data[len] = 0;
    Serial.printf("%s\n", (char *)data);

    char strc[info->index + len]; // Extra space for null terminator '\0'
    // Convert uint8_t array to string
    for (int i = 0; i < info->index + len; i++) {
      strc[i] = (char)data[i];
    }

    /**
    client->printf((char *)data); // Send data to client
    /**/

    String input = String(strc);
    int partCount = 0;  
    int pos = 0;  
    
    // Calculate number of parts  
    for (int i = 0; i < input.length(); i++) {  
      if (input[i] == '|') {  
        partCount++;  
      }  
    }  
    partCount++; // Because string doesn't end with '|', add 1 to part count  
    
    // Allocate string array to store each part  
    String parts[partCount];  
    
    // Split string and store each part  
    for (int i = 0; i < partCount; i++) {  
      int nextPos = input.indexOf('|', pos);  
      if (nextPos == -1) {  
        nextPos = input.length(); // If '|' not found, go to end of string  
      }  
      parts[i] = input.substring(pos, nextPos);  
      pos = nextPos + 1; // Update position to find next part  
    }

    String strMode = parts[0];
    int isHas   = 0;

    //ws2812 LEDs
    if(strMode == "ws2812"){isHas = 1; ws2812Test(parts[1], parts[2], parts[3], parts[4]);}

    //sr04 distance sensor
    if(strMode == "sr04")  {isHas = 1; client->text(sr04Test(parts[1], parts[2]));}

    //switch
    if(strMode == "switch"){isHas = 1; switchTest(parts[1], parts[2]);}

    //oled
    if(strMode == "oled")  {isHas = 1; oledTest(parts[1], parts[2]);}

    if(isHas == 1){
      client->printf("Function matched");
    }else{
      client->printf("Function not matched");
    } 
    client->ping(); 
  }
}
