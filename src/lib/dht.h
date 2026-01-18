//DHT Section
#include <DHT.h>

String dhtInfo = "";
int    dhtRun  = 0;

void getDHT(){
  DHT dht(4, DHT11);
  dht.begin();

  float humidity    = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if read was successful
  if (isnan(humidity) || isnan(temperature)) {
    dhtInfo= "Failed to read from DHT sensor!";
  }else{
    dhtInfo = "Temperature: " + String(temperature) + " °C, Humidity: " + String(humidity) + " %";
  }
  Serial.println(dhtInfo);
}
