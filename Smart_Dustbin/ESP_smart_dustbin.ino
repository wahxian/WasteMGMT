/*

    Quick and Dirty Code for sending weght, volume, humidity, temperature and heat index data
    up to ThingSpeak for data visualisation.

    Setup for ThingSpeak:
    1. Update SSID, Password, ChannelID, writeAPIKey, postingInterval 

    Setup OF HX711 with load cell: 
    1. Connection load cell to HX711: Red(E+) Black(E-) White(A+) Green(A-)
    2. HX711 to ESP8266: GND, DT(SCALEDOUT:GPIO12(D6)), SCK(GPIO14(D5)), VCC(2.6-5.5V)

    Setup for HC-SR04 (3.3V compatible):
    1. HCSR-04 to ESP8266: TRIGGER(RANGETRIG:GPIO5(D1)), ECHO(RANGEECHO:GPIO4(D2)), GND, VCC(3.3V)
    #Try with 5V for now and using potential divider

    Setup for DHT Humidity and Temp Sensor (Blue side facing up):
    1. Seria Data (DHTPIN:GPIO2(D4)), VCC (3.5-5.5V), GND
    #Mapping VCC, Data, NC, GND

    TODO 14/3/2017
    1. Solder wires to Load Cell
    2. Test out individually DHT(OK), HX711, HCSR04

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
#define SCALEDOUT 12
#define SCALESCK 14
#define RANGETRIG 5
#define RANGEECHO 4

HX711 scale(SCALEDOUT,SCALESCK);    //Default gain of 128 used
Ultrasonic ultrasonic(RANGETRIG,RANGEECHO);
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
    float heatIndex = dht.computeHeatIndex(temperature, humidity);
    // Compute heat index in Celsius (isFahreheit = false)
    //float hic = dht.computeHeatIndex(t, h, false);
    if (isnan(humidity) || isnan(temperature) || isnan(heatIndex)) {
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
