
// CONNECTIONS:
// DS3231 SDA --> SDA
// DS3231 SCL --> SCL
// DS3231 VCC --> 3.3v or 5v
// DS3231 GND --> GND


#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <Adafruit_ADS1015.h>
#include <Time.h>
#include <Adafruit_GFX.h>
#include <ESP_Adafruit_SSD1306.h>


#define             OLED_RESET 4  
#define             countof(a) (sizeof(a) / sizeof(a[0]))
RtcDS3231           Rtc;
Adafruit_SSD1306    display(OLED_RESET);
Adafruit_ADS1115    ads(0x48);
int                 posX      = 0;
float               Voltage[]   = {0.0,0.0};

void setup () 
{
    Serial.begin(115200);
    ads.begin();  
    ads.setGain(GAIN_TWOTHIRDS);
    
    // Display mode LED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  //initialize I2C addr 0x3c
    display.clearDisplay();                     // clears the screen and buffer
    display.drawPixel(127, 63, WHITE);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    //--------RTC SETUP ------------
    Rtc.Begin();
    RTC_check();
}

void loop () 
{
    if (!Rtc.IsDateTimeValid()) 
    {
        Serial.println("RTC lost confidence in the DateTime!");
    }
    int16_t adc0, adc1;                                           // we read from the ADC, we have a sixteen bit integer as a result
    adc0            = ads.readADC_SingleEnded(0);
    adc1            = ads.readADC_SingleEnded(1);
    Voltage[0]      = (adc0 * 0.1875)/1000;
    Voltage[1]      = (adc1 * 0.1875)/1000;
    
    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    Serial.println();

    RtcTemperature temp = Rtc.GetTemperature();
    Serial.print(temp.AsFloat());
    Serial.println("C");

    char dateString[20];
    char TimeString[20];
    char SecString[20];
    snprintf_P(dateString,countof(dateString),PSTR("%02u/%02u/%04u"),now.Day(), now.Month(), now.Year());
    snprintf_P(TimeString,countof(TimeString),PSTR("%02u:%02u:%02u"),now.Hour(), now.Minute(), now.Second());
    //snprintf_P(SecString,countof(SecString),PSTR("[%02u]"),now.Second());
    
    
    display.clearDisplay();                 // clears the screen and buffer
    display.setCursor(posX,0);              
    display.setTextSize(1);
    display.println(dateString);
    
    display.setCursor(20,9);
    display.setTextSize(2);
    display.println(TimeString);
    
    //display.setTextSize(3);
    //display.setCursor(30,28);
    //display.println(SecString);

    display.setCursor(0,28);
    display.setTextSize(2);
    String displaySensor = String("V0 :")+Voltage[0]+String("v.");
    display.print(displaySensor);
    display.setCursor(0,43);
    displaySensor = String("V1 :")+Voltage[1]+String("v.");
    display.print(displaySensor);

    
    display.display();
    delay(100); // ten seconds
    
    if(now.Second() < 32){
      posX = now.Second()*2;
    }else{
      posX -=1; 
    }
}



void printDateTime(const RtcDateTime& dt)
{
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

