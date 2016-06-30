/* Design Xanio
 * Create 27/06/2016
 * Sensor  -LDR                   - DS3231
 *         -DHT11                 - ESP8266
 *         -Soil Moisture         
*/
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <Time.h>
#include <Adafruit_GFX.h>
#include <ESP_Adafruit_SSD1306.h>

RtcDS3231           Rtc;
#define             OLED_RESET 4  
Adafruit_SSD1306    display(OLED_RESET);
#define countof(a) (sizeof(a) / sizeof(a[0]))

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  //initialize I2C addr 0x3c
  display.clearDisplay();                     // clears the screen and buffer
  display.drawPixel(127, 63, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();

}

void loop() {
  // put your main code here, to run repeatedly:

}

void setInterval(long milisec){
  
}

