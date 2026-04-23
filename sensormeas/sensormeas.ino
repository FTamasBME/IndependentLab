#include <Adafruit_MAX44009.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

Adafruit_MAX44009 max44009;
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

#define LIGHTSENSORPIN A0
#define PWM_LED_1 3
#define PWM_LED_2 5


unsigned long previousMicrosFast = 0;
unsigned long previousMillisTSL = 0;
unsigned long previousMillisLED = 0;

const unsigned long intervalFast = 6250;//6.25 ms 
const unsigned long intervalTSL = 100;//100 ms
const unsigned long intervalLED = 20;//20 ms   

unsigned long sumTEMT = 0;
unsigned long countTEMT = 0;

const float THRESHOLD_PERCENT = 20.0;

float thresholdMAX  = 0.0;
float thresholdTEMT = 0.0;
float thresholdTSL  = 0.0;

float prevLuxMAX  = -1.0;
float prevLuxTEMT = -1.0;
float prevLuxTSL  = -1.0;

const unsigned long pwmCycleDuration = 20;

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

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PWM_LED_1, OUTPUT);
  pinMode(PWM_LED_2, OUTPUT);

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
void checkChange(float currentLux, float &prevLux, float &threshold) {
  if (prevLux < 0.0) {
    prevLux = currentLux;
    return;
  }

  float diff = abs(currentLux - prevLux);
  threshold = prevLux * (THRESHOLD_PERCENT / 100.0);
  
  if (threshold < 5.0) {
    threshold = 10.0; 
  }

  if (diff > threshold) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));      
  }

  prevLux = currentLux; 
}

void TSLRead(void) {
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  float lux = tsl.calculateLux(full, ir);
  Serial.print(F("TSL (100ms): ")); Serial.println(lux, 6);
  
  //checkChange(lux, prevLuxTSL, thresholdTSL);
}

void MAXRead(void) {
  float lux = max44009.readLux();
  Serial.print(F("MAX (6.25ms): "));Serial.println(lux);

  //checkChange(lux, prevLuxMAX, thresholdMAX);
}

void TEMTRead(void) {
  if (countTEMT == 0) return;

  float averageReading = (float)sumTEMT / countTEMT;

  float voltage = averageReading * (3.3 / 1024.0);
  float amps = voltage / 10000.0 * 1000000.0;  
  float lux = amps * 2.0;
  
  Serial.print(F("TEMT (6.25ms Avg: ")); Serial.println(lux);

  checkChange(lux, prevLuxTEMT, thresholdTEMT);

  sumTEMT = 0;
  countTEMT = 0;
}

void FastInterrupt(void){
  MAXRead();
  TEMTRead();
}

void SlowInterrupt(void){
  Serial.println(F("---")); 
  //TEMTRead();
  TSLRead();
}

/*void updatePwmLEDs(unsigned long currentMillis) {
  float timeProgress = (currentMillis % pwmCycleDuration) / (float)pwmCycleDuration;
  
  float angle = timeProgress * 2.0 * PI;
  
  int pwmValue = (sin(angle) + 1.0) * 127.5;
  
  analogWrite(PWM_LED_1, pwmValue);
  analogWrite(PWM_LED_2, pwmValue);
}*/

void updatePwmLEDs(unsigned long currentMillis) {
  
  static int baseBrightness = 200; 
  int flicker = random(-2, 3);   
  
  analogWrite(PWM_LED_1, constrain(baseBrightness + flicker, 0, 255));
  analogWrite(PWM_LED_2, constrain(baseBrightness + flicker, 0, 255));
}

//-------------------------------------------------------------------------------------------------------------------
void loop() {
  unsigned long currentMicros = micros();
  unsigned long currentMillis = millis();

  sumTEMT += analogRead(LIGHTSENSORPIN);
  countTEMT++;

  if (currentMicros - previousMicrosFast >= intervalFast) {
    previousMicrosFast = currentMicros; 
    
    FastInterrupt();//6.25 ms
  }

  if (currentMillis - previousMillisLED >= intervalLED) {
    previousMillisLED = currentMillis;
    updatePwmLEDs(currentMillis);
  }

  if (currentMillis - previousMillisTSL >= intervalTSL) {
    previousMillisTSL = currentMillis;
    
    SlowInterrupt();//100 ms
  }
  
}