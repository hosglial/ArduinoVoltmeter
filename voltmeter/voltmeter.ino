#include <String.h>
#include <Wire.h> // библиотека для протокола I2C
#include <LiquidCrystal_I2C.h> // библиотека для LCD 1602
#include <SPI.h>                // Подключаем библиотеку SPI
#include <SD.h>                 // Подключаем библиотеку SD
#include <RTClib.h>
#include <Adafruit_ADS1X15.h>


#define EN 6
#define S0 5
#define S1 4
#define S2 3
#define S3 2


Adafruit_ADS1115 ads;			  // Объект для работы с АЦП
LiquidCrystal_I2C LCD(0x27,20,4); // Объект для работы с дисплеем
Sd2Card card;                     // Указываем переменные SD
SdVolume volume;                  // Указываем переменные SD
SdFile root;                      // Указываем переменные SD
 
const int chipSelect = 10;      //адрес sd карты

RTC_DS3231 rtc;

int controlPin[] = {S0, S1, S2, S3};


String print_time(DateTime timestamp) {
  char message[120];

  sprintf(message, "%d-%d-%d %02d:%02d:%02d", 
		  timestamp.month(),
		  timestamp.day(),
		  timestamp.year(),
		  timestamp.hour(),
		  timestamp.minute(),
		  timestamp.second()
  );
  
  return message;
}

int changeMux(int channel){

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

	// Включение мультиплексора
    pinMode(EN, OUTPUT); 
    digitalWrite(EN, LOW);
	
	// Установка digital портов на выход и запись LOW сигнала на них
	for(int i = 0; i < 4; i ++){
		pinMode(controlPin[i], OUTPUT);
		digitalWrite(controlPin[i], LOW);		
	}


  // Установка коэффициента усиления 2/3
  ads.setGain(GAIN_TWOTHIRDS);

  // ВОЗМОЖНЫЕ ВАРИАНТЫ УСТАНОВКИ КУ:
  // ads.setGain(GAIN_TWOTHIRDS); | 2/3х | +/-6.144V | 1bit = 0.1875mV    |
  // ads.setGain(GAIN_ONE);       | 1х   | +/-4.096V | 1bit = 0.125mV     |
  // ads.setGain(GAIN_TWO);       | 2х   | +/-2.048V | 1bit = 0.0625mV    |
  // ads.setGain(GAIN_FOUR);      | 4х   | +/-1.024V | 1bit = 0.03125mV   |
  // ads.setGain(GAIN_EIGHT);     | 8х   | +/-0.512V | 1bit = 0.015625mV  |
  // ads.setGain(GAIN_SIXTEEN);   | 16х  | +/-0.256V | 1bit = 0.0078125mV |

  if (!ads.begin(0x4B)) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  
 
  rtc.begin(); // включение модуля реального времени
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));// настройка времени

  LCD.init(); // инициализация дисплея
  LCD.backlight(); // включение подсветки
    
  pinMode(22, OUTPUT);
  digitalWrite(22, LOW);

  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);

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

  

// Этот кусок пишет на карту, для тестов нахуй не нужен
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
