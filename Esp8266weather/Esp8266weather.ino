#include <ESP8266MQTTClient.h>
#include <ESP8266WiFi.h>
MQTTClient mqtt;
#include "DHT.h"
#define DHTPIN          2   //Pin to attach the DHT
#define DHTTYPE DHT22       //type of DTH  
const char* ssid     = "Your SSID";
const char* password = "Your password";
const int sleepTimeS = 18000; //18000 for Half hour, 300 for 5 minutes etc.
//Get sensor data
float tempc
float tempf
float humidity
float dewptf
int level

///////////////Weather////////////////////////
char server [] = "weatherstation.wunderground.com";
char WEBPAGE [] = "GET /weatherstation/updateweatherstation.php?";
char ID [] = "YourWeatherID";
char PASSWORD [] = "YourPasswordOfWunderground";


/////////////IFTTT///////////////////////
const char* host = "maker.ifttt.com";//dont change
const String IFTTT_Event = "WSTAT_LOW";
const int portHost = 80;
const String Maker_Key = "YourMakerKey";
String stringIFTTT = "POST /trigger/" + IFTTT_Event + "/with/key/" + Maker_Key + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Content-Type: application/x-www-form-urlencoded\r\n\r\n";

//////////////////////////////////////////
DHT dht(DHTPIN, DHTTYPE);



void setup()
{
  Serial.begin(115200);
  dht.begin();
  delay(1000);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  mqtt.begin("mqtt://test.mosquitto.org:1883");
  //  mqtt.begin("mqtt://test.mosquitto.org:1883", {.lwtTopic = "hello", .lwtMsg = "offline", .lwtQos = 0, .lwtRetain = 0});
  //  mqtt.begin("mqtt://user:pass@mosquito.org:1883");
  //  mqtt.begin("mqtt://user:pass@mosquito.org:1883#clientId");

}

void loop() {
  //Check battery
  mqtt.handle();
  level = analogRead(A0);
  level = map(level, 0, 1024, 0, 100);
  if (level < 50)
  {
    batLow(); //Send IFTT
    Serial.println("Low battery");
    delay(500);
  }
  //Get sensor data
  tempc = dht.readTemperature();
  tempf =  (tempc * 9.0) / 5.0 + 32.0;
  humidity = dht.readHumidity();
  dewptf = (dewPoint(tempf, dht.readHumidity()));
  //check sensor data
  Serial.println("+++++++++++++++++++++++++");
  Serial.print("tempF= ");
  Serial.print(tempf);
  Serial.println(" *F");
  Serial.print("tempC= ");
  Serial.print(tempc);
  Serial.println(" *C");
  Serial.print("dew point= ");
  Serial.println(dewptf);
  Serial.print("humidity= ");
  Serial.println(humidity);

  //Send data to Weather Underground
  Serial.print("connecting to ");
  Serial.println(server);
  WiFiClient client;
  if (!client.connect(server, 80)) {
    Serial.println("Conection Failed");
    return;
  }
  client.print(WEBPAGE);
  client.print("ID=");
  client.print(ID);
  client.print("&PASSWORD=");
  client.print(PASSWORD);
  client.print("&dateutc=");
  client.print("now");
  client.print("&tempf=");
  client.print(tempf);
  client.print("&dewptf=");
  client.print(dewptf);
  client.print("&humidity=");
  client.print(humidity);
  client.print("&softwaretype=ESP%208266O%20version1&action=updateraw&realtime=1&rtfreq=2.5");
  client.println();
  delay(2500);
  sleepMode();

}


double dewPoint(double tempf, double humidity) //Calculate dew Point
{
  double A0 = 373.15 / (273.15 + tempf);
  double SUM = -7.90298 * (A0 - 1);
  SUM += 5.02808 * log10(A0);
  SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / A0))) - 1) ;
  SUM += 8.1328e-3 * (pow(10, (-3.49149 * (A0 - 1))) - 1) ;
  SUM += log10(1013.246);
  double VP = pow(10, SUM - 3) * humidity;
  double T = log(VP / 0.61078);
  return (241.88 * T) / (17.558 - T);
}

void batLow() {
  WiFiClient client;
  if (!client.connect(host, portHost)) //Check connection
  {
    Serial.println("Failed connection");
    return;
  }
  client.print(stringIFTTT);//Send information
  delay(10);
  while (client.available())
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}

void newData() {
  WiFiClient client;
  if (!client.connect(host, portHost)) //Check connection
  {
    Serial.println("Failed connection");
    return;
  }
  senddata()
  while (client.available())
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}

void senddata(JsonObject& json)
{
  mqtt.publish("/Weater", String(tempc) + "°C " + String(humidity) + "%hum  " + String(level) + "/1024 bat", 0, 0);
}

void sleepMode() {
  Serial.print(F("Sleeping..."));
  ESP.deepSleep(sleepTimeS * 1000000);
}

