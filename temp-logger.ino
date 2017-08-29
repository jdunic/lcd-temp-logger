// See: https://arduino-info.wikispaces.com/MultipleTemperatureSensorsToLCD

#include <SD.h>
#include <OneWire.h>
#include <RTClib.h>
#include "DallasTemperature.h"
#include "LiquidCrystal.h"

File logfile;
RTC_PCF8523 rtc;
// library functions won't work to set SD pin
int sdCardPin = 10;
char filename[16] = {'\0'};
float temp1;
LiquidCrystal lcd(6, 7, 8, 9, 11, 12); // Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7) 

#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

DeviceAddress Probe01 = { 0x28, 0x2B, 0x05, 0xA8, 0x08, 0x00, 0x00, 0x12 }; 
DeviceAddress Probe02 = { 0x28, 0xFF, 0x65, 0xB5, 0x85, 0x16, 0x03, 0x68 };

void setup() {

  // light up LEDs
  pinMode(sdCardPin, OUTPUT);

  while (!Serial) {
    delay(1000);  // for Leonardo/Micro/Zero
  }
  
  Serial.begin(9600);

  // RTC error checks
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
  }

  // Set up SD card for writing
  if (!SD.begin(sdCardPin)) {
    Serial.println("initialization failed!");
    return;
  } else {
    Serial.println("SD card initialized");
  }

  // Attempting variable file name
  DateTime now = rtc.now();
  int year = now.year();
  int month = now.month();
  int day = now.day();
  sprintf(filename, "%d%d%d.csv", year, month, day);

  // I have no idea why this line makes shit work :|
  if (SD.exists(filename)) { Serial.println("File exists!");}
  
  if (!SD.exists(filename)) {
    // only open a new file if it doesn't exist
    logfile = SD.open(filename, FILE_WRITE);
    Serial.print(filename);
    Serial.println(' - New file created');
  }
  logfile = SD.open(filename, FILE_WRITE);  // open file for writing

  if (logfile) {
    Serial.print("Logging to: ");
    Serial.println(filename);

    Serial.println("Opened logfile");
    
    // Write header
    logfile.println("year, month, day, hour, minute, second, probe, temperature");
    logfile.close();
  }

  // start up the sensor library
  sensors.begin();

  lcd.begin(16, 2); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display

}


void loop() {
    // Get temperature organised
    delay(1000);
    Serial.println();
    Serial.print("Number of Devices found on bus = ");
    Serial.println(sensors.getDeviceCount());
    Serial.print("Getting temperatures... ");
    Serial.println();
    
    // Command all devices on bus to read temperature
    sensors.requestTemperatures();  

    Serial.print("Probe 01 temperature is:   ");
    printTemperature(Probe01);
    Serial.println();

    Serial.print("Probe 02 temperature is:   ");
    printTemperature(Probe02);
    Serial.println();

    // Print some times
    DateTime now = rtc.now();

    int year = now.year();
    int month = now.month();
    int day = now.day();
    int hour = now.hour();
    int minute = now.minute();
    int second = now.second();

    // make a string for assembling the data to log:
    String dataString = String(year) + ", " + String(month) + ", " + String(day) + ", " + String(hour) + ", " + String(minute) + ", " + String(second) + ", ";
    
    // Get ready to write to SD card
    //sprintf(filename, "%d%d%d.csv", year, month, day);
    logfile = SD.open(filename, FILE_WRITE);  // open file for writing

    Serial.println(filename);
    
    if (logfile) {  // if file can be opened, write to it
      Serial.print(filename);
      Serial.println(" file opened for writing");
      
      logfile.print(dataString);
      logfile.print("Probe 01, ");
      writeTemperature(Probe01);
      logfile.print(dataString);
      logfile.print("Probe 02, ");
      writeTemperature(Probe02);
      logfile.close();

      Serial.println("Data string for writing: ");
      Serial.print(dataString);
      printTemperature(Probe01);
      Serial.println();

    } else {  // if not, show an error
      Serial.print("Error: not able to open ");
      Serial.println(filename);
    }

    Serial.println("end of loop test");

    // Print to LCD

    lcd.setCursor(0,0); //Start at character 0 on line 0
    lcd.print("Hello World");
    Serial.println("T1: ");
    //displayTemperature(Probe01);  
  
    //lcd.setCursor(0,1); //Start at character 0 on line 1
    //lcd.print("T2: ");
    //displayTemperature(Probe02); 

    delay(2000);  // delay before next write to SD card, adjust as required

}

/*-----( Declare User-written Functions )-----*/
void printTemperature(DeviceAddress deviceAddress)
{

float tempC = sensors.getTempC(deviceAddress);

   if (tempC == -127.00) 
   {
   Serial.print("Error getting temperature  ");
   } 
   else
   {
   Serial.print("C: ");
   Serial.print(tempC);
   }
}// End printTemperature

void displayTemperature(DeviceAddress deviceAddress)
{

float tempC = sensors.getTempC(deviceAddress);

   if (tempC == -127.00) // Measurement failed or no device found
   {
    lcd.print("Temperature Error");
   } 
   else
   {
   lcd.print(tempC);
   }
}// End printTemperature

void writeTemperature(DeviceAddress deviceAddress)
{

float tempC = sensors.getTempC(deviceAddress);
char tempString[8]; // Buffer big enough for 7-character float
  dtostrf(tempC, 7, 3, tempString); // Leave room for too large numbers!

  String(tempString).trim();

  logfile.println(tempString);

}// End writeTemperature

//*********( THE END )***********
