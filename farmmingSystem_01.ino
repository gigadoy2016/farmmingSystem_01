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
#include <Adafruit_ADS1015.h>
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

#define                 WLAN_SSID       "GIGADOY"
#define                 WLAN_PASS       ""

#define                 MQTT_USER       "controller_01"
#define                 MQTT_PASSWORD   "123456789"
#define                 TOPIC_SUB       "DEVICE/01/command/"
#define                 TOPIC_PUB       "DEVICE/01/status/"
TimeInterval            timeToPublish(4000);     // ส่ง MQTT ทุกๆๆ ค่าเวลาที่เรากำหนด

/********************************************
 *        Define Sensor & Module
 *********************************************/
#define             DHTPIN D3                         // Sensor อุณหภูมิ
#define             DHTTYPE DHT22                     // DHT 22  (AM2302), AM2321
#define             OLED_RESET 4  
#define countof(a) (sizeof(a) / sizeof(a[0]))

DHT                 dht(DHTPIN, DHTTYPE);
RtcDS3231           Rtc;
Adafruit_SSD1306    display(OLED_RESET);
Adafruit_ADS1115    ads(0x48);

WiFiClient              wclient;
PubSubClient            mqttClient(wclient);

LED                     relay_0(D5);
LED                     relay_1(D6);
LED                     relay_2(D7);
LED                     relay_3(D8);

float                   humidity;
float                   temperature;
float                   temperatureSystem;
float                   Voltage[]   = {0.0,0.0};



void setup() {
  Serial.begin(115200);
  ads.begin();  
  ads.setGain(GAIN_TWOTHIRDS);
  //--------RTC SETUP ------------
  Rtc.Begin();
  RTC_check();
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  
  display.clearDisplay();
  display.setTextColor(WHITE);                   
  display.display();

  Serial.println("Start Farmimg..");
  connectionWIFI(WLAN_SSID,WLAN_PASS);
  
  mqttClient.setServer("m11.cloudmqtt.com", 15507);             //->Connection to MQTT Cloud.
  mqttClient.setCallback(callback);
  MQTTConnect();
}

void loop() {
  if (!Rtc.IsDateTimeValid()){ Serial.println("RTC lost confidence in the DateTime!");  }
  int16_t         adc0, adc1;                                  // Define port analog 
  adc0            = ads.readADC_SingleEnded(0);
  adc1            = ads.readADC_SingleEnded(1);
  Voltage[0]      = (adc0 * 0.1875)/1000;
  Voltage[1]      = (adc1 * 0.1875)/1000;
 
  RtcTemperature  tempSystem  = Rtc.GetTemperature();
  temperatureSystem           = tempSystem.AsFloat();
  
  mqttClient.loop();                                            // MQTT check connection.
  MQTTConnect();                                                // Mqtt รออ่านข้อมูลจาก CloudMQTT

  displayScreenLCD();
    
  if(timeToPublish.every()){
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
  humidity        = dht.readHumidity();             // Sensor วัดความชื้นและอุณหภูมิ
  temperature     = dht.readTemperature();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("!! ERROR DHT11 can't read data !! ");
    return;
  } else {
    String dataJson = "{\"temp\":\""+(String)temperature+"\",\"Hum\":\""+(String)humidity+"\",\"relay_0\":"+relay_0.readStatusLED()
                      +",\"relay_1\":"+relay_1.readStatusLED()
                      +",\"relay_2\":"+relay_2.readStatusLED()
                      +",\"relay_3\":"+relay_3.readStatusLED()
                      +",\"tempSystem\":"+temperatureSystem
                      +"}";
    //thingsSpeak(temperature,humidity);

    float data[]={temperature,humidity,Voltage[0],Voltage[1],temperatureSystem};
    
    thingsSpeakData(data);
    char charBuf[200];
    dataJson.toCharArray(charBuf,200);
    mqttClient.publish(TOPIC_PUB,charBuf );
    Serial.print("MQTT Send !    =>");
    Serial.print("Topic:"+(String)TOPIC_PUB);
    Serial.println(charBuf);
    
  }
}

void printDateTime(const RtcDateTime& dt){
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Day(),
            dt.Month(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);

}

void RTC_check(){
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    
    if (!Rtc.IsDateTimeValid()) {
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void displayScreenLCD(){
  RtcDateTime now             = Rtc.GetDateTime();
  char dateString[20];
  char TimeString[20];
  snprintf_P(dateString,countof(dateString)
      ,PSTR("%02u/%02u/%04u %02u:%02u:%02u")
      ,now.Day(), now.Month(), now.Year()
      ,now.Hour(), now.Minute(), now.Second());
    display.clearDisplay();                 // clears the screen and buffer
    display.setCursor(2,0);              
    display.setTextSize(1);
    display.println(dateString);
    
    display.setCursor(0,10);
    String tempDisplay = "t0:"+(String)temperatureSystem + ".C t1:"+(String)temperature+".C";
    display.print(tempDisplay);
    
    display.setCursor(0,20);
    String analogDisplay = "V0:"+(String)Voltage[0]+"V. "+"V1:"+(String)Voltage[1]+"V. ";
    display.print(analogDisplay);
    display.display();
}
