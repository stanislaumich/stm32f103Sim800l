/* Serial.begin(9600);
 Serial1.begin(9600);  
 Serial2.begin(9600);
 Serial3.begin(9600);*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>


USBSerial usb;

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
  Serial1.begin(57600);
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
Serial.println("Starting...");
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
display.display();
delay(500);
display.clearDisplay();
display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hello!");
  display.display();
delay(1000);
}
bool cc=false;
void loop() {
  
   while (Serial.available() > 0) {
    char inByte = Serial.read();
    Serial1.write(inByte);
  }
  delay(200);
  String s="";
  while (Serial1.available() > 0) {
    char inByte = Serial1.read();
    s+=inByte;
    Serial.write(inByte);
    cc=true;
  }
 display.clearDisplay();
 display.setTextSize(1);
 display.setTextColor(WHITE);
 display.setCursor(0,0);
 display.println(s);
 if(cc){
 display.display();
 cc=false;
 }
 //delay(100); AT+CCLK="20/12/05,00:11:33+03"  AT+CCLK? AT&W0   ATZ0
}