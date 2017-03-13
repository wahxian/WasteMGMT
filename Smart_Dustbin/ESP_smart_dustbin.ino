/*
 ESP8266 --> ThingSpeak Channel via MKR1000 Wi-Fi
 
 This sketch sends the Wi-Fi Signal Strength (RSSI) of an ESP8266 to a ThingSpeak
 channel using the ThingSpeak API (https://www.mathworks.com/help/thingspeak).
 
 Requirements:
 
   * ESP8266 Wi-Fi Device
   * Arduino 1.6.9+ IDE
   * Additional Boards URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json
   * Library: esp8266 by ESP8266 Community
 
 ThingSpeak Setup:
 
   * Sign Up for New User Account - https://thingspeak.com/users/sign_up
   * Create a new Channel by selecting Channels, My Channels, and then New Channel
   * Enable one field
   * Note the Channel ID and Write API Key
    
 Setup Wi-Fi:
  * Enter SSID
  * Enter Password
  
 Tutorial: http://nothans.com/measure-wi-fi-signal-levels-with-the-esp8266-and-thingspeak
   
 Created: Feb 1, 2017 by Hans Scharler (http://nothans.com)
*/

#include <ESP8266WiFi.h>
#include "HX711.h"
#include "Ultrasonic.h"
#include "DHT.h"

//Update ssid, pw, channel id, writeAPI for use

//----------------------- Wi-Fi Settings--------------------------------------------
const char* ssid = "xxx"; // your wireless network name (SSID)
const char* password = "xxx"; // your Wi-Fi network password

WiFiClient client;  //Create client to connect to specified IP and port defined in .connect()

//---------------------ThingSpeak Settings-----------------------------------------
const int channelID = xxx;
String writeAPIKey = "xxx"; // write API key for your ThingSpeak Channel
const char* server = "api.thingspeak.com";
const int postingInterval = 20 * 1000; // post data every 20 seconds

//---------------------HX711 and weight sensor settings-----------------------------------------
#define DHTPIN 2        //Digital data pin of DHT
#define DHTTYPE DHT11   

HX711 scale(A1,A0);
Ultrasonic ultrasonic(9,8);
DHT dht(DHTPIN, DHTTYPE);

const int radius 10;    //radius of round dustbin in cm
const int height 50;    //Height of round dustbin in cm
const int FIELDS 5;     //Number of fields we are sending to server

//---------------------Setup WiFi and Serial-----------------------------------------
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // WEIGHT: calibration and tarring
  scale.set_scale(<calibration value>.f);
  scale.tare(); 

  // TEMP HUMID:
  dht.begin();

}

//---------------------Setup WiFi and Serial--------------------------------------
void loop() {
  if (client.connect(server, 80)) {     //If able to connect to thingspeak server

    // TEMP and HUMIDITY
    delay(2000);
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float temperature = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float f = dht.readTemperature(true);
    // Compute heat index in Fahrenheit (the default)
    float heatIndex = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    //float hic = dht.computeHeatIndex(t, h, false);
    if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // WEIGHT: Measure Weight of Dustbin
    Serial.println("Reading Weight");
    long weight = scale.get_units(10); //get the average weight from 10 readings

    // VOLUME
    Serial.println("Reading volume");    
    long range = ultrasonic.Ranging(CM);
    long volume = 3.14159*radius*radius*(height-range);

    //Change this to analog input to send sensor data to server 

    //................Format that ThingSpeak uses...................
    //Construct API request body
    String body = "field1=";
           body += String(weight);

    String body2 = "field2=";
           body2 += String(volume);

    String body3 = "field3=";
           body3 += String(humidity);

    String body4 = "field4=";
           body4 += String(temperature);

    String body5 = "field5=";
           body5 += String(heatIndex);
    
    Serial.print("weight: ");
    Serial.println(weight); 

    Serial.print("volume: ");
    Serial.println(volume);

    Serial.print("humidity: ");
    Serial.println(humidity);

    Serial.print("temperature: ");
    Serial.println(temperature);

    Serial.print("heatIndex: ");
    Serial.println(heatIndex);

    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println("X-THINGSPEAKAPIKEY: " + writeAPIKey);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.print(body.length()+body_two.length()+FIELDS-1);
    client.print("\n\n");
    client.print(body+"&"+body2+"&"+body3+"&"+body4+"&"+body5);
    client.print("\n\n");

    
    //client.print("POST https://api.thingspeak.com/update.json?api_key="+writeAPIKey+"&"+body+"&"+body_two+"\n");


  }
  client.stop();  //Stop connecting to Thingspeak server

  // wait and then post again
  scale.power_down();
  delay(postingInterval);
  scale.power_up();
}
