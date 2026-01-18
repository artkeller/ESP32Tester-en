//Connected to D2
const char* VERSION = "1.1.6";

//Whether to display debug information
const int SHOWINFO = 1;

//Auto restart flag
boolean restart = false;

int intReset = 0;
int intReal  = 0;

//Whether in sleep mode
int canTest = 0;

//Board type settings
#ifdef BOARD_TYPE
    #if BOARD_TYPE == ESP32C3
        const char* BOARD   = "ESP32C3";
    #elif BOARD_TYPE == D1
        const char* BOARD   = "D1";
    #elif BOARD_TYPE == ESP8266
        const char* BOARD   = "ESP8266";
    #else
        const char* BOARD = "Unknown";
    #endif
#else
    const char* BOARD   = "Undefined";
#endif

#define intSerial 115200

WiFiClient   espClient;
PubSubClient client(espClient);

//Enable webserver
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");    // WebSocket object, url is /

//OTA Online Update
const char* OTAURL   = "/firmware.bin";
const char* OTAFSURL = "/spiffs.bin";

//Content modifiable via webserver
String ssid;
String pass;
String ip;
String gateway;
String ota;
String otahost;
String checkid;

//Temporary variables
char buffer[20];

//System configuration
const  char* sysPath = "/sys.txt";

IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 255, 0);

//Temporary variables
String strTmp;
int    intTmp;
