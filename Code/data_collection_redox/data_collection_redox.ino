/*
   This code runs on Arduino Mega and collects data from a redox sensor via RS485
   using Modbus protocol. It then saves the data to a csv file. The polling rate
   can be changed.
   The following modules are necessary to run this code:
   RS485 to TTL
   RTC chip
   SD Card module

*/


#include <RunningAverage.h>   // Necessary library to find running average of the values to prevent outliers
#include <SD.h>               // Library for SD card module functions
#include "RTClib.h"           // Access RTC functions
#include <Wire.h>             // Communicate with I2C devices
#include <ModbusMaster.h>     // Use for modbus protocol functions


// Rx/Tx is hooked up to Serial1 and Data Enable, Receive Enable are hooked up as follows:
#define MAX485_DE   17
#define MAX485_RE_NEG   16


// Instantiate
RTC_PCF8523 rtc;            // Instantiate rtc module
File myFile;                // Instantiate File
RunningAverage redoxRA(20);       // Instantiate RunningAverage
RunningAverage temperatureRA(20);
ModbusMaster node;          // Instantiate Modbus node


// Initialize
String dataString = "";     // Initialize data string
String filename = "";       // Initialize filename
int samples = 0;            // Initialize sample


// Necessary enable functions before and after transmiaaion
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

  rtc.begin();        // Begin rtc module
  pinMode(53, OUTPUT);// Set pin 53 to OUTPUT for SD card communication
  SD.begin(53);       // Begin SD module
  redoxRA.clear();    // Clear any values in the RunningAverage
  temperatureRA.clear();


  DateTime now = rtc.now();     // the DateTime class now is the current time given by rtc module

  // Set pinmodes for enable lines
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);


  // Manually write low to the enable lines
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);


  delay(100);


  Serial1.begin(19200);       // Modbus serial with baud rate 19200
  node.begin(1, Serial1);     // Begin modbus in Serial1
  delay(100);
  Serial.begin(9600);         // Serial for debugging

  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);


  // Hello world
  Serial.println("Goodnight moon!");

  delay(100);


  // Define csv filename format
  filename = String("data") + String(now.month()) +
             String(now.day()) + String(".csv");



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

  dataString = String("Timestamp") + "," + String("Redox value") +
               "," + String("Temperature");

  filename = String("data") + String(now.month()) +
             String(now.day()) + String(".csv");

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
  node.readHoldingRegisters(reg, 10);

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
  // update global filename
  filename = String("data") + String(now.month()) +
             String(now.day()) + String(".csv");


  // same as in setup()
  if (!SD.exists(filename))
  {
    createHeader();
  }

  int MDelay = 3000;

  // Debug
  Serial.print("Redox: ");
  float f_1 = getData(2089);
  Serial.println(f_1, 20);

  Serial.print("Temperature: ");
  float f_2 = getData(2409);
  Serial.println(f_2, 20);


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
  redoxRA.addValue(f_1);
  temperatureRA.addValue(f_2);

  Serial.println(samples);      // debug
  samples++;

  if (samples == 300)     // Every 15 minutes, record the value given from runningAverage
  {
    samples = 0;
    dataString = str + "," + String(redoxRA.getAverage(), 20) +
                 "," + String(temperatureRA.getAverage(), 20);

    // Manually discard outliers
    if (redoxRA.getAverage() < 3000 && temperatureRA.getAverage() < 100) {
      saveData();
    }

    redoxRA.clear();
    temperatureRA.clear();

    // Debug
    Serial.println(redoxRA.getAverage(), 20);
    Serial.println(temperatureRA.getAverage(), 20);
  }

  delay(MDelay);
  Serial.println();

}


/*
   This function opens the csv file and adds a row of data
*/
void saveData() {

  myFile = SD.open(filename, FILE_WRITE);

  Serial.println(myFile);
  Serial.println(filename);

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
