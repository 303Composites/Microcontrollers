/*
   -- Light_Controls_blake --
   
   This source code of graphical user interface 
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 3.1.13 or later version 
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/                   
     - for ANDROID 4.15.01 or later version;
     - for iOS 1.12.1 or later version;
    
   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.    
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE

#include <BLEDevice.h>

// RemoteXY connection settings 
#define REMOTEXY_BLUETOOTH_NAME "Night_Light"


#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 135 bytes
  { 255,2,0,1,0,128,0,19,0,0,0,0,31,1,106,200,1,1,5,0,
  2,29,17,44,22,1,2,26,31,31,79,78,0,79,70,70,0,4,15,72,
  69,13,160,2,26,129,9,54,87,12,64,17,76,105,103,104,116,32,83,101,
  110,115,105,116,105,118,105,116,121,32,0,72,29,127,47,47,108,166,140,1,
  26,0,0,0,0,0,0,200,66,0,0,0,0,94,0,0,200,65,0,0,
  72,66,135,0,0,72,66,0,0,200,66,36,0,0,0,0,0,0,192,65,
  129,32,112,39,12,64,17,66,97,116,116,101,114,121,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t NIGHT_LIGHT_SWITCH; // =1 if switch ON and =0 if OFF
  int8_t lightSens; // from -100 to 100

    // output variables
  int8_t voltage; // from 0 to 100

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

#include <Arduino.h>

#define TEMT6000 A1 //TEMT6000 Photolight sensor
#define VBATT A2 //ADC pin for reading battery voltage
#define PIN_NIGHT_LIGHT_SWITCH D4 //D5 can also be used as an additional LED power pin
int lightCorrFactor = 1; //intial correction factor aka none
float vBatt = 0.0;
float lightAverage = 0;
bool manual_flag = false;
bool manual_reset_flag = false;

unsigned long startMillis; // set the intial time
unsigned long currentMillis; // get the current time
const unsigned long period = 5000;  // the value is a number of milliseconds aka the non-blocking delay

void setup() 
{
  //Included by default
  RemoteXY_Init (); 

  //Enable for debugging
  Serial.begin(115200);
  
  //ESP32C6 pin setup
  pinMode (PIN_NIGHT_LIGHT_SWITCH, OUTPUT); //Switch for night light
  pinMode (VBATT, INPUT); //Configures ADC for battery voltage reading
  pinMode (TEMT6000, INPUT); // TEMT6000 light sensor reading on pin A1

  //Start the timer
  startMillis = millis();  //initial start time
  
  //Intializes the slider
  RemoteXY.lightSens = 0;                  // sets slider to the middle
  RemoteXY.NIGHT_LIGHT_SWITCH = 0;         // Default to automatic mode

}


void loop() 
{ 
  RemoteXY_Handler();
  currentMillis = millis(); // Set current time
  // Battery voltage calculations
  for (int i = 0; i < 16; i++) {
    vBatt += analogReadMilliVolts(VBATT);
  }
  float Vbattf = 2 * vBatt / 16 / 1000.0;
  
  // Control night light switch in manual mode
  if (RemoteXY.NIGHT_LIGHT_SWITCH == 1) {
    digitalWrite(PIN_NIGHT_LIGHT_SWITCH, HIGH);
    manual_flag = true;
    if(currentMillis - startMillis >= period){
      Serial.println("Manual Mode: Nightlight ON");
      RemoteXY.voltage = map(Vbattf * 1000, 280, 390, 0, 100);
      startMillis = currentMillis;
    }
  } 
  else if (currentMillis - startMillis >= period) { // Automatic mode: check time interval
    // Light sensor calculations
    float volts = analogRead(TEMT6000) * 3.3 / 4096.0; // 12-bit ADC resolution
    float amps = volts / 10000.0; 
    float microamps = amps * 1e6;
    // Apply correction factor based on slider value
    float correctionFactor = 1.0 + (RemoteXY.lightSens / 100.0);
    float lux = microamps * 2.0 * correctionFactor;
    
    //Debugging statements for fine tuning room
    Serial.print("Current lux value:");
    Serial.println(lux);

    RemoteXY.voltage = map(Vbattf * 1000, 280, 390, 0, 100);
    // Automatic control based on lux value
    if (lux < 3 && manual_flag == false) {
      digitalWrite(PIN_NIGHT_LIGHT_SWITCH, HIGH);
      RemoteXY.NIGHT_LIGHT_SWITCH = 1;
      manual_flag = false;
      manual_reset_flag = true;
      Serial.println("Automatic Mode: Nightlight ON");

    } 
    else if (lux > 5) {
      digitalWrite(PIN_NIGHT_LIGHT_SWITCH, LOW);
      RemoteXY.NIGHT_LIGHT_SWITCH = 0;
      manual_flag = false;
      manual_reset_flag = true;
      Serial.println("Automatic Mode: Nightlight OFF");
    }

    if (manual_reset_flag == true){
      RemoteXY.NIGHT_LIGHT_SWITCH = 0;
      Serial.println("Preventing the block");
    }
    // Update the startMillis for the next interval
    startMillis = currentMillis;
    Serial.println("Reset Time");
  }
}