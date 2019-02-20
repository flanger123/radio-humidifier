#define BLYNK_PRINT Serial

#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <DHT.h>

#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

byte transmit_data[5]; // массив, хранящий передаваемые данные
byte latest_data[5]; // массив, хранящий последние переданные данные
boolean flag; // флажок отправки данных
boolean start_count=0;
  
  int high_hum=60;
  int low_hum=52;
  int high_temp=21;
  int low_temp=18;
  int aver_temp = 19;

  unsigned long timing=0;
  
  
  float h=0;
  float t=0;
  
  char auth[] = "bfcd0d1b62f44838aa6990140035952f";
  char ssid[] = "Skynet";
  char pass[] = "1601351601";


#include <SoftwareSerial.h>
SoftwareSerial EspSerial(7, 8); // RX, TX
#define ESP8266_BAUD 19200

ESP8266 wifi(&EspSerial);

#define DHTPIN 2          // What digital pin we're connected to
#define DHTTYPE DHT21   // DHT 21, AM2301

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;


///////////////////////////////////////////
////////                  /////////////
///////ОТПРАВКА В ЭФИР/////
/////////             ///////////
////////////////////////////


void sendSensor()
{
  //Serial.println("0");
  h = dht.readHumidity();
  t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }


  transmit_data[0] = h;
  transmit_data[1] = t;
  transmit_data[2] = 0;
  transmit_data[3] = low_hum;
  transmit_data[4] = high_hum;
  for (int i = 0; i < 5; i++) { // в цикле от 0 до числа каналов
    if (transmit_data[i] != latest_data[i]) { // если есть изменения в transmit_data
      flag = 1; // поднять флаг отправки по радио
      latest_data[i] = transmit_data[i]; // запомнить последнее изменение
    }
  }

  if (flag == 1) {
    radio.powerUp(); // включить передатчик
    radio.write(&transmit_data, sizeof(transmit_data)); // отправить по радио
  //    Serial.println(h);
    //    Serial.println(t);
      
            aver_temp=t;
   
      if (t<low_temp || t>high_temp ) 
            {
              if(start_count==0){
                  start_count=1;
                  timing = millis();
                      Serial.print("start bad temp: ");
                     // Serial.println(t);
            }
        else{
               if (millis() - timing > 60000){
                  
                  if (t<low_temp)aver_temp=5;
                  else aver_temp=100;
                  
                  start_count=0;
                  timing = millis();
                   Serial.print("end bad temp: ");
                //   Serial.println(t);
                  }
               }
            }
            
      if(start_count==1&&t>=low_temp && t<=high_temp ){
                      start_count=0; timing = millis(); 
                      Serial.print("вроде наладилось:  ");
                     //  Serial.println(t);
                      }
       
            Blynk.virtualWrite(V5, h);
            Blynk.virtualWrite(V6, t);
            Blynk.virtualWrite(V11, aver_temp);
        
    flag = 0; //опустить флаг
    radio.powerDown(); // выключить передатчик
  }

            Blynk.virtualWrite(V7, low_hum);
            Blynk.virtualWrite(V8, high_hum);         
            Blynk.virtualWrite(V9, low_temp);
            Blynk.virtualWrite(V10, high_temp);

             
}


///////////////////////////////////////////
////////                  /////////////
///////SETUP/////
/////////             ///////////
////////////////////////////

void setup()
{
  // Debug console
  Serial.begin(9600);

  // Set ESP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  delay(10);


Serial.println("wifi to connect");
  Blynk.begin(auth, wifi, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, wifi, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, wifi, ssid, pass, IPAddress(192,168,1,100), 8080);
Serial.println("my connected");
  dht.begin();
  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах
  radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
  radio.setChannel(0x5a);  //выбираем канал (в котором нет шумов!)
  radio.setPALevel (RF24_PA_MIN); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик

  Blynk.virtualWrite(V7, low_hum);
  Blynk.virtualWrite(V8, high_hum);
  Blynk.virtualWrite(V9, low_temp);
  Blynk.virtualWrite(V10, high_temp);


  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);
 
  

}
///////////////////////////////////////////
////////                  /////////////
///////     LOOP            /////
/////////             ///////////
////////////////////////////

void loop()
{
  Blynk.run();
  timer.run();

}


//////////////////////////
//////////////////
////////PINS/////
//////////
//////////



BLYNK_WRITE(V7)
{
  low_hum = param.asInt(); // assigning incoming value from pin V1 to a variable

  Serial.print("V7 hum_low value is: ");
  Serial.println(low_hum);
  
}

BLYNK_WRITE(V8)
{
  high_hum = param.asInt(); // assigning incoming value from pin V1 to a variable

  Serial.print("V8 hum_high value is: ");
  Serial.println(high_hum);
  
}

BLYNK_WRITE(V9)
{
  low_temp = param.asInt(); // assigning incoming value from pin V1 to a variable

  Serial.print("low temp value is: ");
  Serial.println(low_temp);
  
}

BLYNK_WRITE(V10)
{
  high_temp = param.asInt(); // assigning incoming value from pin V1 to a variable

  Serial.print("hight_temp value is: ");
  Serial.println(high_temp);
  
}
