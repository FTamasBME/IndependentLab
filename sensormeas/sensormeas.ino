#include <Adafruit_MAX44009.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

Adafruit_MAX44009 max44009;
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

#define LIGHTSENSORPIN A0


unsigned long previousMicrosFast = 0;
unsigned long previousMillisTSL = 0;

const unsigned long intervalFast = 6250; 
const unsigned long intervalTSL = 100;   

//-----------------------------------------------------------------------------------------------------

void displayTSLSensorDetails(void) {
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}

void configureTSLSensor(void) {
  tsl.setGain(TSL2591_GAIN_MED);     
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS); 
}
//-----------------------------------------------------------------------------------------------------

void TSLSetup(void){
  if (tsl.begin()) {
    Serial.println(F("Found a TSL2591 sensor"));
  } else {
    Serial.println(F("No TSL2591 sensor found"));
    while (1);
  }
  displayTSLSensorDetails();
  configureTSLSensor();
}

void MAXSetup(void){
  if (!max44009.begin()) {
    Serial.println(F("Couldn't find MAX44009 chip"));
    while (1) delay(10);
  }
  
  max44009.setMode(MAX44009_MODE_MANUAL_CONTINUOUS);
  max44009.setIntegrationTime(MAX44009_INTEGRATION_6_25MS);
  
  max44009.setUpperThreshold(1000.0);
  max44009.setLowerThreshold(100.0);
  max44009.setThresholdTimer(0);
  max44009.enableInterrupt(false);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("Sensor Test Started"));

  // --------------------------------------------------------------------------------MAX44009 Setup----
  MAXSetup();
  //--------------------------------------------------------------------------------SL2591 Setup------
  TSLSetup();
  //--------------------------------------------------------------------------------TEMT6000 Setup----
  pinMode(LIGHTSENSORPIN, INPUT);

  Serial.println(F("Setup complete. Starting continuous read..."));
  delay(1000); 
}

//-------------------------------------------------------------------------------------------------------------------
void TSLRead(void) {
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  Serial.print(F("TSL (100ms): ")); Serial.println(tsl.calculateLux(full, ir), 6);
}

void MAXRead(void) {
  float lux = max44009.readLux();
  Serial.print(F("MAX (6.25ms): "));Serial.println(lux);
}

void TEMTRead(void) {
  float reading = analogRead(LIGHTSENSORPIN);
  float voltage = reading * (3.3 / 1024.0);
  float amps = voltage / 10000.0 * 1000000.0;  
  float lux = amps * 2.0;
  Serial.print(F("TEMT (6.25ms): ")); Serial.println(lux);
}

//-------------------------------------------------------------------------------------------------------------------
void loop() {
  unsigned long currentMicros = micros();
  unsigned long currentMillis = millis();

  if (currentMicros - previousMicrosFast >= intervalFast) {
    previousMicrosFast = currentMicros; 
    
    MAXRead();
    TEMTRead();
  }

  if (currentMillis - previousMillisTSL >= intervalTSL) {
    previousMillisTSL = currentMillis;
    
    Serial.println(F("---")); 
    //TEMTRead();
    TSLRead();
  }
  
}