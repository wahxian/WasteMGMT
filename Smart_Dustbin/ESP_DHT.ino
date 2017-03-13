#include <ESP8266WiFi.h>
#include "DHT.h"

#define DHTPIN 2        //Digital data pin of DHT
#define DHTTYPE DHT11  

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Checking H&T");
  delay(2000);
  float h = dht.readHumidity();
  Serial.print("Humidity= ");
  Serial.println(h);
  float t = dht.readTemperature();
  Serial.print("Temperature= ");
  Serial.println(t);
  float f = dht.readTemperature(true);
  Serial.print("Temperature(F)= ");
  Serial.println(f);  
  float hif = dht.computeHeatIndex(f, h);
  Serial.print("HeatIndex= ");
  Serial.println(hif);  
  float hic = dht.computeHeatIndex(t, h, false);
  Serial.print("HeatIndex(F)= ");
  Serial.println(hic);    
  if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
  }
}
