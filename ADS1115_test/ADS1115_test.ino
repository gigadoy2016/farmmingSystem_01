#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads(0x48);
float Voltage = 0.0;
float Voltage2 = 0.0;

void setup(void) 
{
  Serial.begin(115200);  
  ads.begin();
  ads.setGain(GAIN_TWOTHIRDS);
}

void loop(void) 
{
  int16_t adc0, adc1;  // we read from the ADC, we have a sixteen bit integer as a result

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  Voltage = (adc0 * 0.1875)/1000;
  Voltage2 = (adc1 * 0.1875)/1000;
  
  Serial.print("AIN0: "); 
  Serial.print(adc0);
  Serial.print("\tVoltage: ");
  Serial.println(Voltage, 7);  
  Serial.println();

  Serial.print("AIN1: "); 
  Serial.print(adc1);
  Serial.print("\tVoltage: ");
  Serial.println(Voltage2, 7);  
  Serial.println();
  
  
  delay(1000);
}
