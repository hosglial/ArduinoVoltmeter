#include <String.h>
#include <Wire.h> // библиотека для протокола I2C
#include <LiquidCrystal_I2C.h> // библиотека для LCD 1602
LiquidCrystal_I2C LCD(0x27,20,4); // присваиваем имя дисплею
#include <SPI.h>                // Подключаем библиотеку SPI
#include <SD.h>                 // Подключаем библиотеку SD
#include <RTClib.h>
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;

#define EN 2
#define S0 3
#define S1 4
#define S2 5
#define S3 6



uint8_t pinArr[10] = {A1, A2, A3, A4, A5, A6, A7, A8, A9, A10}; //массив пинов
float COEF = 0.0048; //коэффициент для перевода в вольты
float COEFGALV = 1.77; //коэффициент для перевода в вольты
Sd2Card card;                   // Указываем переменные
SdVolume volume;                // Указываем переменные
SdFile root;                    // Указываем переменные
 
const int chipSelect = 49;      //адрес sd карты


void SdSetup();

RTC_DS3231 rtc;

String print_time(DateTime timestamp) {
  char message[120];

  int Year = timestamp.year();
  int Month = timestamp.month();
  int Day = timestamp.day();
  int Hour = timestamp.hour();
  int Minute = timestamp.minute();
  int Second= timestamp.second();

  sprintf(message, "%d-%d-%d %02d:%02d:%02d", Month,Day,Year,Hour,Minute,Second);
  
  return message;
}

int changeMux(int channel){

  int controlPin[] = {S0, S1, S2, S3};

  int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }
}

void setup() {  
    Serial.begin(9600);

    pinMode(EN, OUTPUT); 
    digitalWrite(EN, LOW);

    pinMode(S0, OUTPUT); 
    pinMode(S1, OUTPUT); 
    pinMode(S2, OUTPUT); 
    pinMode(S3, OUTPUT);   
  
    digitalWrite(S0, LOW);
    digitalWrite(S1, LOW);
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);


  // Установка коэффициента усиления
  ads.setGain(GAIN_TWOTHIRDS);

  // ВОЗМОЖНЫЕ ВАРИАНТЫ УСТАНОВКИ КУ:
  // ads.setGain(GAIN_TWOTHIRDS); | 2/3х | +/-6.144V | 1bit = 0.1875mV    |
  // ads.setGain(GAIN_ONE);       | 1х   | +/-4.096V | 1bit = 0.125mV     |
  // ads.setGain(GAIN_TWO);       | 2х   | +/-2.048V | 1bit = 0.0625mV    |
  // ads.setGain(GAIN_FOUR);      | 4х   | +/-1.024V | 1bit = 0.03125mV   |
  // ads.setGain(GAIN_EIGHT);     | 8х   | +/-0.512V | 1bit = 0.015625mV  |
  // ads.setGain(GAIN_SIXTEEN);   | 16х  | +/-0.256V | 1bit = 0.0078125mV |

  ads.begin(0x4B); // Инициализация модуля ADS1115


  
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  LCD.init(); // инициализация дисплея
  LCD.backlight(); // включение подсветки
    
  pinMode(22, OUTPUT);
  digitalWrite(22, LOW);

  //настройка пинов на чтение (возможно нахуй не нужна будет)
  for (int i = 0; i < 10; i++) {
    pinMode(pinArr[i], INPUT);
    analogWrite(pinArr[i], LOW);
  }

   Serial.println("Initializing SD card...");


  //проверка наличия sd, если нет, принт ошибки
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

  DateTime n = rtc.now();
   
  String logData = String(print_time(n));
//  String logData = String(rtc.now().toString(date)) + "    ";
  LCD.print(n.toString(date));
  
   for (int i = 0; i<10; i ++) {
    changeMux(i);
    delay(50);
    dataArr[i] = ads.computeVolts(ads.readADC_Differential_0_1());
    logData += "    ";
    logData += String(dataArr[i]);
    LCD.setCursor((i%4) * 5,int(i/4)+1); // ставим курсор на 1 символ первой строке
    LCD.print(dataArr[i]); // выводим напряжение на дисплей
   }

   Serial.println(logData);    
 
//  for (int i = 0; i<10; i++) {
//    dataArr[i] = 0;
//    dataArr[i] = analogRead(pinArr[i]);
//    dataArr[i] *= COEF;
//    dataArr[i] *= COEFGALV;
//    //Serial.println("Voltage: " + String(i+1) + "-" + String(dataArr[i]));
//    logData += String(dataArr[i]);
//    logData += "    ";
//    Serial.println(logData);    
//    LCD.setCursor((i%4) * 5,int(i/4)+1); // ставим курсор на 1 символ первой строке
//    LCD.print(dataArr[i]); // выводим напряжение на дисплей
//    
//  }


// Этот кусок пишет на карту, для тестов нахуй не нужен
//  File dataFile = SD.open("log.txt", FILE_WRITE);
//    if (dataFile) {
//    dataFile.println(logData);
//    dataFile.close();
//  }
//  else {
//    Serial.println("error opening log.txt");
//  }
  delay(500);
  LCD.clear(); // очищаем экран дисплея

}
