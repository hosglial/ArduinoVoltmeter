#include <String.h>
#include <Wire.h> // библиотека для протокола I2C
#include <LiquidCrystal_I2C.h> // библиотека для LCD 1602
LiquidCrystal_I2C LCD(0x27,20,4); // присваиваем имя дисплею
#include <SPI.h>                // Подключаем библиотеку SPI
#include <SD.h>                 // Подключаем библиотеку SD
#include <Time.h>
#include <RTClib.h>



uint8_t pinArr[10] = {A1, A2, A3, A4, A5, A6, A7, A8, A9, A10}; //массив пинов
float COEF = 0.0048; //коэффициент для перевода в вольты
float COEFGALV = 1.77; //коэффициент для перевода в вольты
Sd2Card card;                   // Указываем переменные
SdVolume volume;                // Указываем переменные
SdFile root;                    // Указываем переменные
 
const int chipSelect = 49;     


void SdSetup();

RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);

  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  
  
  LCD.init(); // инициализация дисплея
  LCD.backlight(); // включение подсветки
    
  pinMode(22, OUTPUT);
  digitalWrite(22, LOW);
  
  for (int i = 0; i < 10; i++) {
    pinMode(pinArr[i], INPUT);
    analogWrite(pinArr[i], LOW);
  }

   Serial.println("Initializing SD card...");

  
  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization failed!");
  }
  else {
    Serial.println("Initialization done.");
  }
  
  
}

void loop() {
  float dataArr[10];
  char date[19] = "DD MMM YY hh:mm:ss";

  String logData = String(rtc.now().toString(date)) + "    ";
  LCD.print(rtc.now().toString(date));
  for (int i = 0; i<10; i++) {
    dataArr[i] = 0;
    dataArr[i] = analogRead(pinArr[i]);
    dataArr[i] *= COEF;
    dataArr[i] *= COEFGALV;
    //Serial.println("Voltage: " + String(i+1) + "-" + String(dataArr[i]));
    logData += String(dataArr[i]);
    logData += "    ";    
    LCD.setCursor((i%4) * 5,int(i/4)+1); // ставим курсор на 1 символ первой строке
    LCD.print(dataArr[i]); // выводим напряжение на дисплей
    
  }
  Serial.println(logData);
  File dataFile = SD.open("log.txt", FILE_WRITE);
    if (dataFile) {
    dataFile.println(logData);
    dataFile.close();
  }
  else {
    Serial.println("error opening log.txt");
  }
  delay(500);
  LCD.clear(); // очищаем экран дисплея

}
