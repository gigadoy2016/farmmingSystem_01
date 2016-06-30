// ทดสอบการอ่านค่า จาก Moisture Sensor ผ่าน Analog Port A0

// โดย Arduitronics 

// www.arduitronics.com  หรือ  www.facebook.com/arduitronics


int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  // declare the ledPin as an OUTPUT:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);  
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);    
  // turn the ledPin on
  digitalWrite(ledPin, HIGH);  
  // stop the program for milliseconds:
  delay(sensorValue);          
  // turn the ledPin off:        
  digitalWrite(ledPin, LOW);   
  // stop the program for for milliseconds:
  delay(sensorValue);  

  Serial.print("Moisture of Soil:  ");
  Serial.println(sensorValue);
  delay(2000);
}
