/*   Данный скетч делает следующее: передатчик (TX) отправляет массив
     данных, который генерируется согласно показаниям с кнопки и с
     двух потенциомтеров. Приёмник (RX) получает массив, и записывает
     данные на реле, сервомашинку и генерирует ШИМ сигнал на транзистор.
    by AlexGyver 2016
*/

#include <Wire.h> 
#include <avr/wdt.h>

//LiquidCrystal_I2C lcd(0x27,16,2);



#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

int butt=0;


RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно


byte recieved_data[5]; // массив принятых данных
byte sensor_button = 5; // попробуем замкнуть кнопку
byte fan = 3; //куллер
byte water = 6;//опрос датчика воды
unsigned long timing=0; 
float h = 0;//влажность
float t=0;
boolean nabor = 1; // увлажнитель включен

int delay1=100;
int delay2=1000;
int delay3=20000; /// сколько крутит куллер

int low_hum=52;
int high_hum=60;


byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб
///////////////////////////////////////////
////////                  /////////////а
///////SETUP/////
/////////             ///////////
////////////////////////////

void setup() {
 
        wdt_reset();
      wdt_enable (WDTO_8S); //собака сбрасывается через 8 секунд
      wdt_reset();
  
  Serial.begin(9600); //открываем порт для связи с ПК

  pinMode(sensor_button, OUTPUT); // настроить пин сенсорной кнопки как выход
  pinMode(fan, INPUT); //проверка куллера
  pinMode(water, INPUT); //проверка воды

  
        radio.begin(); //активировать модуль
        radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
        radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
        radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
        radio.setPayloadSize(32);     //размер пакета, в байтах

        radio.openReadingPipe(1, address[0]);     //хотим слушать трубу 0
        radio.setChannel(0x5a);  //выбираем канал (в котором нет шумов!)

        radio.setPALevel (RF24_PA_MIN); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
        radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
        //должна быть одинакова на приёмнике и передатчике!
        //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
      wdt_reset();
delay(3000);

///включаем увлажнитель
      digitalWrite(sensor_button, HIGH);
      delay (delay1);
      digitalWrite(sensor_button, LOW);
      delay (delay2);
      digitalWrite(sensor_button, HIGH);
      delay (delay1);
      digitalWrite(sensor_button, LOW);
      delay(delay2);
      Serial.println("увлажнитель включен");
            wdt_reset();
}


///////////////////////////////////////////
////////                  /////////////
///////     LOOP            /////
/////////             ///////////
////////////////////////////

void loop() {
    // Serial.println(h);
        wdt_reset();
        byte pipeNo;
        while ( radio.available(&pipeNo)) {  // слушаем эфир со всех труб
        radio.read( &recieved_data, sizeof(recieved_data) );         // чиатем входящий сигнал
        h = recieved_data[0]; // извлекаем влажность
        t = recieved_data[1]; // извлекаем влажность
        butt = recieved_data[2]; // извлекаем кнопку
     low_hum = recieved_data[3];
       high_hum = recieved_data[4];
        wdt_reset();
        Serial.print("Humidity: ");
        Serial.print(h,2);
        Serial.print(" %\t");
        Serial.print("Temperature: ");
        Serial.print(t,2);
        Serial.println(" *C");
        Serial.print("Fan: ");
        Serial.println(digitalRead(fan));
        Serial.print("Water: ");
        Serial.println(digitalRead(water));
        Serial.print("humlow: ");
        Serial.println(low_hum);
        Serial.print("humhigh: ");
        Serial.println(high_hum);
        
   if( h < low_hum && nabor==0 && digitalRead(water) == LOW ){
      Serial.println("влажность низкая, включаемся");
      turn_sensor2();
      nabor=1;
   //   Serial.println("влажность низкая, включаемся");
      //     delay_wdt(delay3);
      if(digitalRead(fan) == HIGH){turn_sensor1();  Serial.println("куллер не работает");   }
      }
      
    
   

   if( h > high_hum && nabor==1 && digitalRead(water) == LOW){
      Serial.println("влажность высокая, отключаемся");
      turn_sensor1();
      nabor=0;
      
      
   //   delay_wdt(delay3); 
      if(digitalRead(fan) == LOW){turn_sensor1();  Serial.println("куллер все еще работает");   }
      
      
   }

            if(digitalRead(water) == HIGH){
               Serial.println("воды нет, отдыхаем");
               delay_wdt(delay2);
                
             }
      

     if(butt==1){
        digitalWrite(sensor_button, HIGH);
        delay (delay1);
        digitalWrite(sensor_button, LOW);
        delay(delay2);
                }

   }



if (millis() - timing > 60000){  
                timing=millis(); 
           //     if(nabor==1 && digitalRead(fan) == HIGH && digitalRead(water) == LOW)
             //   {
               //  Serial.println("походу кто-то меня вЫключил");     
                 //nabor=0;
                // }
                
                if(nabor==0 && digitalRead(fan) == LOW && digitalRead(water) == LOW)
                {
                 Serial.println("походу кто-то меня включил");     
                 nabor=1;
                 }
                  }
                
 
}

void delay_wdt(unsigned long time) 
{
  unsigned long ms_timer;
 
  wdt_reset();
  ms_timer = millis() + time;
  while (millis() < ms_timer) wdt_reset();
    
  
}


void turn_sensor1()
{
      digitalWrite(sensor_button, HIGH);
      delay_wdt(delay1);
      digitalWrite(sensor_button, LOW);
      delay_wdt(delay3);
      timing=millis(); 
}


void turn_sensor2()
{
      digitalWrite(sensor_button, HIGH);
      delay_wdt(delay1);
      digitalWrite(sensor_button, LOW);
      delay_wdt(delay2);
      digitalWrite(sensor_button, HIGH);
      delay_wdt(delay1);
      digitalWrite(sensor_button, LOW);
      delay_wdt(delay3);
      timing=millis(); 
}
