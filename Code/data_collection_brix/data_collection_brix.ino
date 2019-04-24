
/*
 * This program reads modbus data and stores in microSD card
 * 
 */


 #include <RunningAverage.h>
 #include <ModbusMaster.h>
 #include <SD.h>
 #include "RTClib.h"
 #include <Wire.h>


  ModbusMaster node;    
  String dataString ="";
  RTC_PCF8523 rtc;
  File myFile;
  String filename = "";
  RunningAverage sensor1RA(20);
  RunningAverage sensor2RA(20);
  int samples = 0;
  
void setup()
{
  
  rtc.begin();
  pinMode(53, OUTPUT);
  SD.begin(53);
  sensor1RA.clear();
  sensor2RA.clear();
  


  DateTime now = rtc.now();

  Serial1.begin(9600);
  Serial.begin(4800);
  delay(100);  
  node.begin(1, Serial1);  // Modbus RTU
  delay(100);
  Serial.println("Connected "); 
  Serial.println("Modbus RTU Master Online");

//  dataString = String("Timestamp")+ "," + String("Sensor 1 Measured Pressure") +","+ String("Sensor1 Druck n. Lagekor") + "," + String("Sensor1 Sensor Pressure") + "," + String("Sensor1 temp")
//   + "," + String("Sensor 2 Measured Pressure") +","+ String("Sensor2 Druck n. Lagekor") + "," + String("Sensor2 Sensor Pressure") + "," + String("Sensor2 temp");
//
  filename = String("data") + String(now.month()) + String(now.day()) + String(".csv");

  if (!SD.exists(filename))
  {
    createHeader();
  }
  
}

void createHeader(){
  DateTime now = rtc.now();
//   dataString = String("Timestamp")+ "," + String("Sensor 1 Measured Pressure") +","+ String("Sensor1 Druck n. Lagekor") + "," + String("Sensor1 Sensor Pressure") + "," + String("Sensor1 temp")
//   + "," + String("Sensor 2 Measured Pressure") +","+ String("Sensor2 Druck n. Lagekor") + "," + String("Sensor2 Sensor Pressure") + "," + String("Sensor2 temp");

  dataString = String("Timestamp")+ "," + String("Sensor 1 Measured Pressure") +","+  String("Sensor1 temp") + "," + String("Sensor 2 Measured Pressure") + "," + String("Sensor2 temp");

  filename = String("data") + String(now.month()) + String(now.day()) + String(".csv");
  saveData();
}



union ifloat 
{
  uint8_t bytes[4];
  float val;
};

float getData(int reg)
{

  node.readHoldingRegisters(reg, 2); 

  signed long node1 = node.getResponseBuffer(0);
  word node2 = node.getResponseBuffer(1);

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


void loop()
{
  
  DateTime now = rtc.now();
  filename = String("data") + String(now.month()) + String(now.day()) + String(".csv");

  if (!SD.exists(filename))
  {
    createHeader();
  }
//  unsigned long seconds = 1000L; //Notice the L 
//  unsigned long minutes = seconds * 60;

  int Mdelay = 3000;   // microdelay

  Serial.print("Sensor 1 Measured Pressure: ");
  float f_1_1 = getData(256);
  Serial.println(f_1_1,20 );

//  Serial.print("Sensor1 Druck n. Lagekor: ");
//  float f_1_2 = getData(258);
//  Serial.println(f_1_2,20 );
//
//  Serial.print("Sensor1 Sensor Pressure:  ");
//  float f_1_3 = getData(260);
//  Serial.println(f_1_3,20 );

  Serial.print("Sensor1 temp: ");
  float f_1_temp = getData(262);
  Serial.println(f_1_temp, 20);

  
  Serial.print("Sensor 2 Measured Pressure: ");
  float f_2_1 = getData(264);
  Serial.println(f_2_1,20 );

//  Serial.print("Sensor1 Druck n. Lagekor: ");
//  float f_2_2 = getData(266);
//  Serial.println(f_2_2,20 );
//
//  Serial.print("Sensor1 Sensor Pressure:  ");
//  float f_2_3 = getData(268);
//  Serial.println(f_2_3,20 );
  
  Serial.print("Sensor2 temp: ");
  float f_2_temp = getData(270);
  Serial.println(f_2_temp, 20);

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

  sensor1RA.addValue(f_1_1);
  sensor2RA.addValue(f_2_1);
  Serial.println(samples);



  samples++;
  if (samples == 20)
  {
    samples = 0;
//    dataString = str + "," + String(sensor1RA.getAverage(),20) + "," + String(f_1_2, 20) + "," + String(f_1_3, 20)+ ","+ String(f_1_temp, 20) + ","
//      + String(sensor2RA.getAverage(),20) + "," + String(f_2_2, 20) + "," + String(f_2_3, 20)+ ","+ String(f_2_temp, 20);
    
    dataString = str + "," + String(sensor1RA.getAverage(),20)+ ","+ String(f_1_temp, 20) + "," + String(sensor2RA.getAverage(),20) + ","+ String(f_2_temp, 20);
    
    if (sensor1RA.getAverage() < 2.5 && sensor2RA.getAverage() < 2.5){
      saveData();
    }
    
    sensor1RA.clear();
    sensor2RA.clear();
  Serial.println(sensor1RA.getAverage(),20);
  Serial.println(sensor2RA.getAverage(),20);
  }
  
  


//  dataString = str + "," + String(f_1_1, 20) + "," + String(f_1_2, 20) + "," + String(f_1_3, 20)+ ","+ String(f_1_temp, 20) + ","
//  + String(f_2_1, 20) + "," + String(f_2_2, 20) + "," + String(f_2_3, 20)+ ","+ String(f_2_temp, 20);
//  saveData();


  delay(Mdelay);
//  delay(minutes);

  Serial.println();

  
}

void saveData(){
  // now append new data fil
  myFile = SD.open(filename, FILE_WRITE);

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
