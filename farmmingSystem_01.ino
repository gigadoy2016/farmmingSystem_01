/* Design Xanio
 * Create 27/06/2016
 * Sensor  -LDR                   - DS3231 
 *         -DHT11                 - ESP8266
 *         -Soil Moisture         - ADS1115
*/

#include <Wire.h>  
#include <RtcDS3231.h>
#include <Time.h>
#include <Adafruit_GFX.h>
#include <ESP_Adafruit_SSD1306.h>
#include "ESP8266WiFi.h"
#include "PubSubClient.h"
#include "DHT.h"
#include "connection.h"
#include "ArduinoJson.h"
#include "TimeInterval.h"
#include "LED.h"


/*******************************************
 * Configgulation
 *******************************************/

#define                 WLAN_SSID       "no_angel"
#define                 WLAN_PASS       "o8o9237579"

#define                 MQTT_USER       "controller_01"
#define                 MQTT_PASSWORD   "123456789"
#define                 TOPIC_SUB       "DEVICE/01/command/"
#define                 TOPIC_PUB       "DEVICE/01/status/"

/********************************************
 *        Define Sensor & Module
 *********************************************/
#define             DHTPIN D4                         // Sensor DHT11
#define             OLED_RESET 4  
#define countof(a) (sizeof(a) / sizeof(a[0]))

DHT                 dht(DHTPIN, DHT11);
RtcDS3231           Rtc;
Adafruit_SSD1306    display(OLED_RESET);

WiFiClient              wclient;
PubSubClient            mqttClient(wclient);

LED                     relay_0(D5);
LED                     relay_1(D6);
LED                     relay_2(D7);
LED                     relay_3(D8);

TimeInterval            timeToPublish(15000);     // ส่ง MQTT ทุกๆๆ ค่าเวลาที่เรากำหนด

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  
  display.clearDisplay();                     
  display.drawPixel(127, 63, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();

  Serial.println("Start Farmimg..");
  connectionWIFI(WLAN_SSID,WLAN_PASS);
  
  mqttClient.setServer("m11.cloudmqtt.com", 15507);             //->Connection to MQTT Cloud.
  mqttClient.setCallback(callback);
  MQTTConnect();
}

void loop() {
  mqttClient.loop(); 
  MQTTConnect();                                                // Mqtt รออ่านข้อมูลจาก CloudMQTT
  if(timeToPublish.every(15000)){
    DataToCloud();
  }
}

void MQTTConnect() {
  
  if (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP8266_MQTT", MQTT_USER, MQTT_PASSWORD)) {         // Attempt to connect
      Serial.println("cloudmqtt connected ");                                  // Once connected, publish an announcement..
      mqttClient.subscribe(TOPIC_SUB);                                         // อ่านหัวข้อ Topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}

/*****************************************************************
 *  Function callBack รอคำสั่งจาก MQTT Cloud ในรูปแบบ JSON 
 *****************************************************************/
void callback(char* topic, byte* payload, unsigned int length){
  String message="";
  Serial.print("MQTT Read      =>");
  Serial.print(topic);
  Serial.print("=> ");
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(message);
  
  String device = root["deviceID"];
  if(device == "0001"){
    relay_0.set((int)root["relay_0"]);
    relay_1.set((int)root["relay_1"]);
    relay_2.set((int)root["relay_2"]);
    relay_3.set((int)root["relay_3"]);
    DataToCloud();                                         // Return status Data ทั้งหมดขึ้นสู่ Cloud
  }
}
/******************************************************
 *  function ส่งค่าต่างจาก sensor  ขึ้น cloud
 ******************************************************/
void DataToCloud(){
  float humidity        = dht.readHumidity();             // Sensor วัดความชื้นและอุณหภูมิ
  float temperature     = dht.readTemperature();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("!! ERROR DHT11 can't read data !!");
    timeToPublish.reset();     
  } else {
    String dataJson = "{\"temp\":\""+(String)temperature+"\",\"Hum\":\""+(String)humidity+"\",\"relay_0\":"+relay_0.readStatusLED()
                      +",\"relay_1\":"+relay_1.readStatusLED()
                      +",\"relay_2\":"+relay_2.readStatusLED()
                      +",\"relay_3\":"+relay_3.readStatusLED()+"}";
    thingsSpeak(temperature,humidity);
    char charBuf[200];
    dataJson.toCharArray(charBuf,200);
    mqttClient.publish(TOPIC_PUB,charBuf );
    Serial.print("MQTT Send !    =>");
    Serial.print("Topic:"+(String)TOPIC_PUB);
    Serial.println(charBuf);
  }
}

