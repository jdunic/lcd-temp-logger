// See: https://arduino-info.wikispaces.com/MultipleTemperatureSensorsToLCD

#include <SPI.h>
#include "SdFat.h"
#include <OneWire.h>
#include <RTClib.h>
#include "DallasTemperature.h"
#include <LiquidCrystal.h>
#include "Wire.h"
#include "Adafruit_LiquidCrystal.h"

SdFat SD;
RTC_PCF8523 rtc;
// Set sd card pin (10 is default on UNO with data logger shield)
int sdCardPin = 10;      

// Temperature probe data pin
#define ONE_WIRE_BUS 2

// Connect via i2c, default address #0 (A0-A2 not jumpered)
Adafruit_LiquidCrystal lcd(0);

// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

DeviceAddress Probe01 = { 0x28, 0x2B, 0x05, 0xA8, 0x08, 0x00, 0x00, 0x12 }; 
DeviceAddress Probe02 = { 0x28, 0xFF, 0x65, 0xB5, 0x85, 0x16, 0x03, 0x68 };
DeviceAddress Probe03 = { 0x28, 0x46, 0xE8, 0xA7, 0x08, 0x00, 0x00, 0x37 };
DeviceAddress Probe04 = { 0x28, 0x88, 0xD2, 0xA7, 0x08, 0x00, 0x00, 0x82 };;


void setup() {

  // set up the LCD's number of rows and columns: 
  lcd.begin(20, 4);

  // light up LEDs
  pinMode(sdCardPin, OUTPUT);

  while (!Serial) {
    delay(2000);  // for Leonardo/Micro/Zero
  }

  Serial.begin(9600);

  // Set up SD card for writing
  if (!SD.begin(sdCardPin)) {
    Serial.println(F("initialization failed!"));
    return;
  } else {
    Serial.println(F("SD card initialized"));
  }

  Serial.println(F("--- SETUP BEGIN ---"));
  // RTC error checks
  if (! rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }
  if (! rtc.initialized()) {
    Serial.println(F("RTC is NOT running!"));
  }

  // Attempting variable file name
  File logfile;
  char filename[16];
  getFilename(filename);

  logfile = SD.open(filename, FILE_WRITE);  // open file for writing

  if (logfile) {
    Serial.print(F("Logging to: "));
    Serial.println(filename);

    Serial.println(F("Opened logfile"));
    
    // Write header
    logfile.println(F("year, month, day, hour, minute, second, probe, temperature"));
    logfile.close();
  }

  // start up the sensor library
  sensors.begin();
  
  Serial.println(F("--- SETUP END ---"));
}


void loop() {
    delay(1000);

    File logfile;
    char filename[16];

    getFilename(filename);

    Serial.println(F("---"));
    Serial.print(F("Number of Devices found on bus = "));
    Serial.println(sensors.getDeviceCount());
    Serial.print(F("Getting temperatures... "));
    Serial.println();
    
    // Command all devices on bus to read temperature
    sensors.requestTemperatures();  

    Serial.print(F("Probe 01 temperature is:   "));
    printTemperature(Probe01);
    Serial.println();

    Serial.print(F("Probe 02 temperature is:   "));
    printTemperature(Probe02);
    Serial.println();

    Serial.print(F("Probe 03 temperature is:   "));
    printTemperature(Probe03);
    Serial.println();

    Serial.print(F("Probe 04 temperature is:   "));
    printTemperature(Probe04);
    Serial.println();

    delay(1000);

    // Print to LCD
    lcd.setCursor(0, 0); //Start at character 0 on line 0
    lcd.print(F("T1: "));
    //displayTemperature(Probe01); 

    //lcd.setCursor(10, 0); 
    //lcd.setCursor(0, 1); 
    //lcd.print(F("T2: "));
    //displayTemperature(Probe02); 

    //lcd.setCursor(0, 2); 
    //lcd.print(F("T3: "));
    //displayTemperature(Probe03);
  
    //lcd.setCursor(0, 3); 
    //lcd.print(F("T4: "));
    //displayTemperature(Probe04); 

    //lcd.setCursor(0,1); //Start at character 0 on line 1
    //lcd.print("T2: ");

    // make a string for assembling the data to log:
    DateTime now = rtc.now();

    char dateTimeString[14];
    sprintf(dateTimeString, "%d,%d,%d,%d,%d,%d,", 
            now.year(), now.month(), now.day(), now.hour(), now.minute(), 
            now.second());
    
    delay(1000);
    
    // Get ready to write to SD card
    //sprintf(filename, "%d%d%d.csv", year, month, day);
    logfile = SD.open(filename, FILE_WRITE);  // open file for writing
    
    Serial.println(F("--- MID LOOP---"));

    if (logfile) {  // if file can be opened, write to it
      logfile.print(dateTimeString);
      writeTemperature(logfile, 1, Probe01);
      logfile.print(dateTimeString);
      writeTemperature(logfile, 2, Probe02);
      logfile.print(dateTimeString);
      writeTemperature(logfile, 3, Probe03);
      logfile.close();
      writeTemperature(logfile, 4, Probe04);
      logfile.close();

      Serial.println(F("Data string for writing: "));
      Serial.print(dateTimeString);
      printTemperature(Probe01);
      Serial.println();

    } else {  // if not, show an error
      Serial.print(F("Error: not able to open "));
      Serial.println(filename);
    }

    delay(1000);  // delay before next write to SD card, adjust as required
}

/*-----( Declare User-written Functions )-----*/
void printTemperature(DeviceAddress deviceAddress) {
  float tempC = sensors.getTempC(deviceAddress);

  if (tempC == -127.00) {
   Serial.print(F("Error getting temperature  "));
  } else {
   Serial.print(F("C: "));
   Serial.print(tempC);
  }
}// End printTemperature

/*
void displayTemperature(DeviceAddress deviceAddress) {
  float tempC = sensors.getTempC(deviceAddress);
   if (tempC == -127.00) { // Measurement failed or no device found
    lcd.print("Temperature Error");
   } else {
    lcd.print(tempC, 2);
   }
}// End printTemperature
*/

void writeTemperature(File logfile, int probeNumber, DeviceAddress deviceAddress) {
  float tempC = sensors.getTempC(deviceAddress);
  char tempString[8]; // Buffer big enough for 7-character float
  dtostrf(tempC, 7, 3, tempString); // Leave room for too large numbers!

  String(tempString).trim();

  logfile.print(F("Probe "));
  logfile.print(probeNumber);
  logfile.print(F(","));
  logfile.println(tempString);
} // End writeTemperature

void getFilename(char* filename) {
  DateTime now = rtc.now();

  sprintf(filename, "%d%d%d.csv", now.year(), now.month(), now.day());
}

//*********( THE END )***********
