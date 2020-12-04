/*#include <Arduino.h>

#define LED_BUILTIN PC13

void setup()
{
 pinMode(LED_BUILTIN, OUTPUT);
 Serial.begin(9600);
 Serial1.begin(9600);  
 Serial2.begin(9600);
 Serial3.begin(9600);
}

void loop()

{
 digitalWrite(LED_BUILTIN, 1);
 Serial1.println("Serial LED OFF");
 Serial.println("Serial zero LED OFF");
 Serial2.println("Serial DUE LED OFF");
 Serial3.println("Serial TRE LED OFF");

 delay(1000);

 digitalWrite(LED_BUILTIN, 0);
 Serial1.println("Serial LED ON");
 Serial.println("Serial zero LED ON");
 Serial2.println("Serial DUE LED ON");
 Serial3.println("Serial TRE LED ON");

 delay(1000);
}
*/

#include <Arduino.h>
//#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>
/// pio lib install "mbed-chag/SoftwareSerial"

USBSerial usb;

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//SoftwareSerial portOne(PA10, PA9);//pa9 white -- pa10 green
//SoftwareSerial portTwo(RX, TX);

void setup() {
  Serial1.begin(57600);
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
Serial.println("Starting...");

 // portOne.begin(57600);
//portTwo.begin(9600);
}

void loop() {
   while (Serial.available() > 0) {
    char inByte = Serial.read();
    Serial1.write(inByte);
  }
  delay(200);

  //portOne.listen();
  while (Serial1.available() > 0) {
    char inByte = Serial1.read();
    Serial.write(inByte);
  }
//--  portTwo.listen();
//--  Serial.println("Data from port two:");
/*--  while (portTwo.available() > 0) {
    char inByte = portTwo.read();
    Serial.write(inByte);
  }
--*/
 
 delay(100);
}