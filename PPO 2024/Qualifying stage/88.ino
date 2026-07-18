#include <SPI.h>          // библиотека для работы с шиной SPI
#include "nRF24L01.h"     // библиотека радиомодуля
#include "RF24.h"         // ещё библиотека радиомодуля
RF24 radio(9, 53); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб
String str, role = "receiver";
byte key;
int delay_, count = 0;

void setDelay(int &delay_){
  Serial.println("Please set a delay for sending messages");
  while(true){
    if(Serial.available()){
      delay_ = Serial.parseInt();
      if(delay_ != 0){
        Serial.print("The sending of next messages will be delayed for ");
        Serial.print(delay_);
        Serial.println(" ms");
        break;        
      }
    }
  }
}

void keyForming(byte &key, String str){
  if(count == str.length()){
    count = 0;
  }
  key = str[count];
  if(key != 10){
    if(key >= 65 && key <= 90){
       key -= 65;
    }
    else if(key >= 97 && key <= 122){
      key -= 97;
    }
  }
  count++;
}

void keyEntering(String &str){
  Serial.println("Enter a letter to encipher the messages");
  while(true){
    if(Serial.available()){
      str = Serial.readString();
      if((int) str[0] != 10){
        break;
      }
    }
  }
  Serial.print("Your key is ");
  Serial.println(str);
}

void setup() {
  Serial.begin(9600);         // открываем порт для связи с ПК
  Serial.println("Press T to transmit messages or R to receive");
  radio.begin();              // активировать модуль
  radio.setAutoAck(1);        // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();   // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);   // размер пакета, в байтах
  radio.openWritingPipe(address[2]);
  radio.openReadingPipe(1, address[1]);
  radio.setChannel(0x71);

  radio.setPALevel (RF24_PA_MAX);   // уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_2MBPS); // скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  radio.startListening();
  radio.powerUp();        // начать работу
}

void deciphering(byte &symbol, byte key){
  if((symbol >= 65 && symbol <= 90)||(symbol >= 97 && symbol <= 122)){
    if(symbol >= 65 && symbol <= 90){
      if((symbol - 65 - key) < 0){
        symbol = (26 - (key - symbol + 65) + 65);
      }
      else{
        symbol -= key;
      }
    }
    else if(symbol >= 97 && symbol <= 122){
      if((symbol - 97 - key) < 0){
        symbol = (26 - (key - symbol + 97) + 97);
      }
      else{
        symbol -= key;
      }              
    }
  }
}

void waitForConfirmation(){
  Serial.println("Waiting for sending's confirmation");
  radio.startListening();
  byte pipeNo, state = 133;
  int count = 0;
  while(state == 133 && count <= 20000){
    while(radio.available(&pipeNo)){
      radio.read(&state, sizeof(state));
    }
    count++;
  }
  Serial.println(state);
  radio.stopListening();
}

void transmitterSetup(){
  keyEntering(str);
  setDelay(delay_);
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);
  radio.setChannel(0x60);
  radio.stopListening();
}

void receiverSetup(){
  radio.openWritingPipe(address[2]);
  radio.openReadingPipe(1, address[1]);
  radio.setChannel(0x71);
  radio.startListening(); // начинаем слушать эфир, мы приёмный модуль  
}

void loop() {
  if(role == "receiver"){
    if(Serial.available()){
      byte change;
      change = Serial.read();
      if(change == 84 && role == "receiver"){
        role = "transmitter";
        Serial.println("This channel is now a transmitter");
        transmitterSetup();
      }
      else{
        byte pipeNo;
        byte message[2];
        while(radio.available(&pipeNo)){
          radio.read(message, sizeof(message));
          deciphering(message[0], message[1]);
          if(message[0] == 10){
            Serial.print('\n');
          }
          else{
            Serial.print((char) message[0]);
          }
        }
      }
    }
    else{
      byte pipeNo;
      byte message[2];
      while(radio.available(&pipeNo)){
        radio.read(message, sizeof(message));
        deciphering(message[0], message[1]);
        if(message[0] == 10){
          Serial.print('\n');
        }
        else{
          Serial.print((char) message[0]);
        }
      }
    }
  }
  else if(role == "transmitter"){
    byte pipeNo;
    if(Serial.available()){
      byte message;
      message = Serial.read();
      if(message == 82 && role == "transmitter"){
        Serial.println("This channel is now a receiver");
        role = "receiver";
        receiverSetup();
      }
      else{
        byte messageArray[2];
        keyForming(key, str);
        if((message >= 65 && message <= 90)||(message >= 97 && message <= 122)){
          if(message >= 65 && message <= 90){
            message = ((message + key - 65) % 26 + 65);
          }
          else if(message >= 97 && message <= 122){
            message = ((message + key - 97) % 26 + 97);
          }
        }
        messageArray[0] = message;
        messageArray[1] = key;
        delay(delay_);
        radio.write(messageArray, sizeof(messageArray));
        Serial.print("Sent message: ");
        Serial.println(message);
        waitForConfirmation();
      }
    }
  }
}
