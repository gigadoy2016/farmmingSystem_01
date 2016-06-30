
// CONNECTIONS:
// DS3231 SDA --> SDA
// DS3231 SCL --> SCL
// DS3231 VCC --> 3.3v or 5v
// DS3231 GND --> GND


#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <Time.h>
#include <Adafruit_GFX.h>
#include <ESP_Adafruit_SSD1306.h>

RtcDS3231           Rtc;
#define             OLED_RESET 4  
Adafruit_SSD1306    display(OLED_RESET);
#define             countof(a) (sizeof(a) / sizeof(a[0]))
int                 posX = 0;

void setup () 
{
    Serial.begin(115200);
    // Display mode LED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  //initialize I2C addr 0x3c
    display.clearDisplay();                     // clears the screen and buffer
    display.drawPixel(127, 63, WHITE);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    //--------RTC SETUP ------------
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

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

void loop () 
{
    if (!Rtc.IsDateTimeValid()) 
    {
        Serial.println("RTC lost confidence in the DateTime!");
    }

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
    snprintf_P(TimeString,countof(TimeString),PSTR("%02u:%02u"),now.Hour(), now.Minute());
    snprintf_P(SecString,countof(SecString),PSTR("[%02u]"),now.Second());
    
    display.clearDisplay(); // clears the screen and buffer
    display.setCursor((now.Second()*2),0);
    display.setTextSize(1);
    display.println(dateString);
    display.setCursor(20,9);
    display.setTextSize(2);
    display.println(TimeString);
    display.setTextSize(3);
    display.setCursor(30,30);
    display.println(SecString);
    display.display();
    delay(100); // ten seconds
    //posX ++;
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

