
#include <RunningAverage.h>
#include <SD.h>
#include "RTClib.h"
#include <Wire.h>

#include <ModbusMaster.h>

#define MAX485_DE   17
#define MAX485_RE_NEG   16

String dataString ="";
RTC_PCF8523 rtc;
File myFile;
String filename = "";
RunningAverage redoxRA(20);
RunningAverage temperatureRA(20);

int samples = 0;

ModbusMaster node;

void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}




void setup() {

  rtc.begin();
  pinMode(53, OUTPUT);
  SD.begin(53);
  redoxRA.clear();
  temperatureRA.clear();
  

  DateTime now = rtc.now();
  
  
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);

  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  
  delay(100);

  Serial1.begin(19200);
  node.begin(1, Serial1);
  delay(100);
  Serial.begin(9600);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  Serial.println("Goodnight moon!");
  
  delay(100);
  
  filename = String("data") + String(now.month()) + 
  String(now.day()) + String(".csv");
  
  if (!SD.exists(filename))
  {
    createHeader();
  }


}

void createHeader(){
  DateTime now = rtc.now();
  
  dataString = String("Timestamp")+","+String("Redox value")+
  ","+String("Temperature");
  
  filename = String("data") + String(now.month()) + 
  String(now.day()) + String(".csv");

  saveData();
}

union ifloat 
{
  uint8_t bytes[4];
  float val;
};

float getData(int reg)
{
  node.readHoldingRegisters(reg,10);
  
  signed long node1 = node.getResponseBuffer(3);
  word node2 = node.getResponseBuffer(2);

  byte msb3 = node1;
  byte msb4 = node1 >> 8;
  byte msb1 = node2;
  byte msb2 = node2 >> 8;
  ifloat f;
  f.bytes[3] = msb4;
  f.bytes[2] = msb3;
  f.bytes[1] = msb2;
  f.bytes[0] = msb1;
  
  
  node.clearResponseBuffer();
  
  return f.val;
}

void loop() { // run over and over

  DateTime now = rtc.now();
  
  filename = String("data") + String(now.month()) + 
  String(now.day()) + String(".csv");
  
  if (!SD.exists(filename))
  {
    createHeader();
  }
  
  int MDelay = 3000;
  
  Serial.print("Redox: ");
  float f_1 = getData(2089);
  Serial.println(f_1,20);
  
  Serial.print("Temperature: ");
  float f_2 = getData(2409);
  Serial.println(f_2, 20);
  
  
  String y = String(now.year());
  String mo = String(now.month());
  String d = String(now.day());

  String h = String(now.hour());
  String m = String(now.minute());
  String s = String(now.second());

  Serial.print(h);
  Serial.print(m);
  Serial.print(s);
  Serial.println();
  
  String str = y + "/" + mo + "/" + d + " " + h + ":" + m + ":" + s;
  
  redoxRA.addValue(f_1);
  temperatureRA.addValue(f_2);
  Serial.println(samples);
  samples++;
  
  if (samples == 300)
  {
    samples = 0;
    dataString = str+","+String(redoxRA.getAverage(),20)+
    ","+String(temperatureRA.getAverage(),20);
    
    
    if (redoxRA.getAverage() < 3000 && temperatureRA.getAverage() < 100){
      saveData();
    }
    
    redoxRA.clear();
    temperatureRA.clear();
    Serial.println(redoxRA.getAverage(),20);
    Serial.println(temperatureRA.getAverage(),20);
  }
  
  delay(MDelay);
  Serial.println();

}

  void saveData(){

  myFile = SD.open(filename, FILE_WRITE);
  
  Serial.println(myFile);
  Serial.println(filename);
  
  if (SD.exists(filename))
  {
    if (myFile){
      myFile.println(dataString);
      myFile.close(); // close the file
    }
  }
  else
  {
    Serial.println("Error writing to file !");
  }
  
}
  

  
  