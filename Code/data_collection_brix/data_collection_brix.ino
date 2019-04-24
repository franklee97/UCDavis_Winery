/*
   This code runs on Arduino Mega and collects data from two Brix sensor via one RS232
   using Modbus protocol. It then saves the data to a csv file. The polling rate
   can be changed.
   The following modules are necessary to run this code:
   RS 232 Shifter
   RTC chip
   SD Card module

*/

#include <RunningAverage.h>   // Necessary library to find running average of the values to prevent outliers
#include <SD.h>               // Library for SD card module functions
#include "RTClib.h"           // Access RTC functions
#include <Wire.h>             // Communicate with I2C devices
#include <ModbusMaster.h>     // Use for modbus protocol functions



// Instantiate
ModbusMaster node;            // Instantiate Modbus node
RTC_PCF8523 rtc;              // Instantiate rtc module
File myFile;                  // Instantiate File
RunningAverage sensor1RA(20); // Instantiate RunningAverage
RunningAverage sensor2RA(20);


// Initialize
String dataString = "";     // Initialize data string
String filename = "";       // Initialize filename
int samples = 0;            // Initialize sample

void setup()
{

  rtc.begin();        // Begin rtc module
  pinMode(53, OUTPUT);// Set pin 53 to OUTPUT for SD card communication
  SD.begin(53);       // Begin SD module
  // Clear any values in runningAverage
  sensor1RA.clear();
  sensor2RA.clear();


  // the DateTime class now is the current time given by rtc module
  DateTime now = rtc.now();

  Serial1.begin(9600);      // Modbus serial with baud rate 9600
  Serial.begin(4800);       // Serial for debugging
  delay(100);
  node.begin(1, Serial1);  // Modbus RTU
  delay(100);
  Serial.println("Connected ");
  Serial.println("Modbus RTU Master Online");

  // Define csv filename format
  filename = String("data") + String(now.month()) + String(now.day()) + String(".csv");

  // If the filename doesn't exist, create the header for CSV file. Header is the first row of CSV file
  if (!SD.exists(filename))
  {
    createHeader();
  }

}

/*
   Function to create the header of CSV file.
   It also updates the global filename value with current date
*/
void createHeader() {
  DateTime now = rtc.now();

  dataString = String("Timestamp") + "," + String("Sensor 1 Measured Pressure") + "," +  String("Sensor1 temp") + "," + String("Sensor 2 Measured Pressure") + "," + String("Sensor2 temp");

  filename = String("data") + String(now.month()) + String(now.day()) + String(".csv");
  saveData();
}


/*
   Necessary to convert float to decimal
*/
union ifloat
{
  uint8_t bytes[4];
  float val;
};

/*
   @param reg : the modbus register the user wishes to retrive data from
   This function reads the data on the modbus register as well as convert the value
   from float to decimal.
*/
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
  // update global filename
  filename = String("data") + String(now.month()) + String(now.day()) + String(".csv");

  if (!SD.exists(filename))
  {
    createHeader();
  }


  int Mdelay = 3000;   // microdelay

  Serial.print("Sensor 1 Measured Pressure: "); // Debug
  float f_1_1 = getData(256);
  Serial.println(f_1_1, 20 );     // Debug

  Serial.print("Sensor1 temp: "); // Debug
  float f_1_temp = getData(262);
  Serial.println(f_1_temp, 20);   // Debug


  Serial.print("Sensor 2 Measured Pressure: "); // Debug
  float f_2_1 = getData(264);
  Serial.println(f_2_1, 20 );   // Debug

  Serial.print("Sensor2 temp: "); // Debug
  float f_2_temp = getData(270);
  Serial.println(f_2_temp, 20);   // Debug


  // Retrieve time data
  String y = String(now.year());
  String mo = String(now.month());
  String d = String(now.day());

  String h = String(now.hour());
  String m = String(now.minute());
  String s = String(now.second());

  // Debug
  Serial.print(h);
  Serial.print(m);
  Serial.print(s);
  Serial.println();


  // Concatenate time data
  String str = y + "/" + mo + "/" + d + " " + h + ":" + m + ":" + s;


  // Push to runningAverage
  sensor1RA.addValue(f_1_1);
  sensor2RA.addValue(f_2_1);
  Serial.println(samples);    // Debug



  samples++;
  if (samples == 20)      // Every minute, record the value given from runningAverage
  {
    samples = 0;

    dataString = str + "," + String(sensor1RA.getAverage(), 20) + "," + String(f_1_temp, 20) + "," + String(sensor2RA.getAverage(), 20) + "," + String(f_2_temp, 20);

    // Manually discard outliers
    if (sensor1RA.getAverage() < 2.5 && sensor2RA.getAverage() < 2.5) {
      saveData();
    }

    sensor1RA.clear();
    sensor2RA.clear();
    // Debug
    Serial.println(sensor1RA.getAverage(), 20);
    Serial.println(sensor2RA.getAverage(), 20);
  }


  delay(Mdelay);

  Serial.println();


}


/*
   This function opens the csv file and adds a row of data
*/
void saveData() {
  myFile = SD.open(filename, FILE_WRITE);

  if (SD.exists(filename))
  {
    if (myFile) {
      myFile.println(dataString);
      myFile.close(); // close the file
    }
  }
  else
  {
    Serial.println("Error writing to file !");
  }

}
