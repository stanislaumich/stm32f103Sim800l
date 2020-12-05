/* 
http://codius.ru/articles/GSM_%D0%BC%D0%BE%D0%B4%D1%83%D0%BB%D1%8C_SIM800L_%D1%87%D0%B0%D1%81%D1%82%D1%8C_2

Serial.begin(9600);
 Serial1.begin(9600);  
 Serial2.begin(9600);
 Serial3.begin(9600);
 
 Имя: mts  
APN: mts
Прокси: не установлено
Порт: не установлено
Имя пользователя: mts
Пароль: mts
MCC (где не проставлено автоматически): 257
MNC (где не проставлено автоматически): 02
Тип аутентификации: РАР
Тип APN: выбрать "Интернет" (где нет автоматического выбора - написать default)

AT+SAPBR=3,1,"CONTYPE","GPRS"
OK
AT+SAPBR=3,1,"APN","mts"
OK
AT+SAPBR=3,1,"USER","mts"
OK
AT+SAPBR=3,1,"PWD","mts"
OK
AT+SAPBR=1,1
OK
//
AT+HTTPINIT
OK
AT+HTTPPARA="CID",1
OK
AT+HTTPPARA="URL","http://byhov-cge.by/time.php"   AT+CCLK="2020/12/05 17:59:29+03"
OK
AT+HTTPACTION=0
OK
+HTTPACTION:0,200,1647

AT+HTTPREAD

// тут страница


AT+HTTPTERM
OK



AT+CIPGSMLOC=2,1 // Get GSM time and date


// закрываем соединение
AT+SAPBR=0,1
OK
 */
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>


#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

void log(String s){
  Serial.println(s);
}
String s1writewait(String s){
  String res="";
  Serial1.println(s);
  res=Serial1.readString();
  return res;
}
bool initsim800time(void){
 String s; 
 log("Start time settings");
 s="AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"";
 //log(s);
 log(s1writewait(s));
 s="AT+SAPBR=3,1,\"APN\",\"mts\"";
 //log(s);
 log(s1writewait(s));
 s="AT+SAPBR=3,1,\"USER\",\"mts\"";
 //log(s);
 log(s1writewait(s));
 s="AT+SAPBR=3,1,\"PWD\",\"mts\"";
 //log(s);
 log(s1writewait(s));
 s="AT+SAPBR=1,1";
 //log(s);
 log(s1writewait(s));
 s="AT+HTTPINIT";
 //log(s);
 log(s1writewait(s));
 s="AT+HTTPPARA=\"CID\",1";
 //log(s);
 log(s1writewait(s));
 s="AT+HTTPPARA=\"URL\",\"http://byhov-cge.by/time.php\"";
 //log(s);
 log(s1writewait(s));
 s="AT+HTTPACTION=0";
 //log(s);
 log(s1writewait(s));
 s=Serial1.readString();// тут ответом 2 строки
 s="AT+HTTPREAD";
 //log(s);
 s=s1writewait(s);// тута сформирована команда установки времени
 //log(s);
 log(s1writewait(s));// выполним её и получим ответ
 s="AT+HTTPTERM";
 //log(s);
 log(s1writewait(s));
 s="AT+SAPBR=0,1";
 //log(s);
 log(s1writewait(s));
 // время установлено в модуле AT+CCLK?
 return true;
}



void setup() {
  Serial1.begin(57600);
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
Serial.println("Starting...");


if (initsim800time()){log("Time is set.");log(s1writewait("AT+CCLK?"));}else{log("Time set error.");}
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
//    char inByte = Serial.read();
    String instr = Serial.readString();
    Serial1.println(instr);
//    Serial1.write(inByte);
  }
  delay(200);
  String s="";
  while (Serial1.available() > 0) {
    //char inByte = Serial1.read();
    s=Serial1.readString();
    //s+=inByte;
    //Serial.write(inByte);
    Serial.println(s);
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
 //delay(100); AT+CCLK="20/12/05,00:11:33+03"  AT+CCLK? AT&W0   ATZ0   AT+COPS=? — список операторов в сети.
}//AT+CCLK="20/12/05,18:03:00+03"


/*
+CMTI: "SM",7 - приход смс




*/