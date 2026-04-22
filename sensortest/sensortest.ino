/*
 * Full test sketch for MAX44009 Ambient Light Sensor
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code for
 * Adafruit Industries. MIT license, check license.txt for more information
 *
 * Displays configuration and continuous lux readings.
 */

#include <Adafruit_MAX44009.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

Adafruit_MAX44009 max44009;
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

#define LIGHTSENSORPIN A0

//-----------------------------------------------------------------------------------------------------

/**************************************************************************/
void displayTSLSensorDetails(void)
{
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
  delay(500);
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2591
*/
/**************************************************************************/
void configureTSLSensor(void)
{
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Display the gain and integration time for reference sake */  
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print  (F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC); 
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}
//-----------------------------------------------------------------------------------------------------

void setup() {
   
  Serial.begin(9600);
  while (!Serial)
    delay(10);

  Serial.println(F("MAX44009 Full Test"));
  Serial.println(F("=================="));

  if (!max44009.begin()) {
    Serial.println(F("Couldn't find MAX44009 chip"));
    while (1)
      delay(10);
  }

  Serial.println(F("MAX44009 found!"));

  // === Mode Configuration ===
  Serial.println(F("\n--- Mode Configuration ---"));

  max44009.setMode(MAX44009_MODE_CONTINUOUS);
  // Options: MAX44009_MODE_DEFAULT (auto, 800ms cycle, lowest power)
  //          MAX44009_MODE_CONTINUOUS (auto, fast updates)
  //          MAX44009_MODE_MANUAL (manual gain/time, 800ms cycle)
  //          MAX44009_MODE_MANUAL_CONTINUOUS (manual gain/time, fast updates)
  Serial.print(F("Mode: "));
  switch (max44009.getMode()) {
    case MAX44009_MODE_DEFAULT:
      Serial.println(F("Default (auto, 800ms cycle)"));
      break;
    case MAX44009_MODE_CONTINUOUS:
      Serial.println(F("Continuous (auto, fast updates)"));
      break;
    case MAX44009_MODE_MANUAL:
      Serial.println(F("Manual (800ms cycle)"));
      break;
    case MAX44009_MODE_MANUAL_CONTINUOUS:
      Serial.println(F("Manual Continuous (fast updates)"));
      break;
  }

  // === Integration Time ===
  // Only configurable in MANUAL modes; skip if using auto-ranging
  if (max44009.getMode() == MAX44009_MODE_MANUAL ||
      max44009.getMode() == MAX44009_MODE_MANUAL_CONTINUOUS) {
    Serial.println(F("\n--- Integration Time ---"));

    max44009.setIntegrationTime(MAX44009_INTEGRATION_100MS);
    // Options: MAX44009_INTEGRATION_800MS  (best low-light sensitivity)
    //          MAX44009_INTEGRATION_400MS
    //          MAX44009_INTEGRATION_200MS
    //          MAX44009_INTEGRATION_100MS  (default, best high-brightness)
    //          MAX44009_INTEGRATION_50MS   (manual mode only)
    //          MAX44009_INTEGRATION_25MS   (manual mode only)
    //          MAX44009_INTEGRATION_12_5MS (manual mode only)
    //          MAX44009_INTEGRATION_6_25MS (manual mode only)
    Serial.print(F("Integration Time: "));
    switch (max44009.getIntegrationTime()) {
      case MAX44009_INTEGRATION_800MS:
        Serial.println(F("800ms"));
        break;
      case MAX44009_INTEGRATION_400MS:
        Serial.println(F("400ms"));
        break;
      case MAX44009_INTEGRATION_200MS:
        Serial.println(F("200ms"));
        break;
      case MAX44009_INTEGRATION_100MS:
        Serial.println(F("100ms"));
        break;
      case MAX44009_INTEGRATION_50MS:
        Serial.println(F("50ms"));
        break;
      case MAX44009_INTEGRATION_25MS:
        Serial.println(F("25ms"));
        break;
      case MAX44009_INTEGRATION_12_5MS:
        Serial.println(F("12.5ms"));
        break;
      case MAX44009_INTEGRATION_6_25MS:
        Serial.println(F("6.25ms"));
        break;
    }

    // === Current Division Ratio ===
    Serial.println(F("\n--- Current Division Ratio ---"));

    max44009.setCurrentDivisionRatio(false);
    // false = full photodiode current to ADC (normal)
    // true  = 1/8 current to ADC (for very bright environments)
    Serial.print(F("CDR: "));
    Serial.println(max44009.getCurrentDivisionRatio() ? F("1/8 (divided)")
                                                      : F("Full current"));

  } else {
    Serial.println(F("\n(Integration time and CDR are auto-managed)"));
  }

  // === Interrupt Configuration ===
  Serial.println(F("\n--- Interrupt Configuration ---"));

  max44009.setUpperThreshold(1000.0);
  // Range: 0.045 to 188,000 lux (quantized to exponent/mantissa encoding)
  Serial.print(F("Upper Threshold: "));
  Serial.print(max44009.getUpperThreshold());
  Serial.println(F(" lux"));

  max44009.setLowerThreshold(100.0);
  // Range: 0.045 to 188,000 lux
  Serial.print(F("Lower Threshold: "));
  Serial.print(max44009.getLowerThreshold());
  Serial.println(F(" lux"));

  max44009.setThresholdTimer(0);
  // Timer value * 100ms = delay before interrupt fires
  // 0 = immediate, 255 = 25.5 seconds (default)
  Serial.print(F("Threshold Timer: "));
  Serial.print(max44009.getThresholdTimer());
  Serial.println(F(" (x100ms)"));

  max44009.enableInterrupt(false);
  // true = INT pin asserts low when lux outside threshold window
  // Note: INT pin is open-drain, needs external pull-up
  // Note: reading getInterruptStatus() clears the interrupt
  Serial.print(F("Interrupt: "));
  Serial.println(max44009.isInterruptEnabled() ? F("Enabled") : F("Disabled"));

  // === Continuous Lux Readings ===
  Serial.println(F("\n--- Lux Readings ---"));
  //-------------------------------------------------------------------------------------------------------------------
  if (tsl.begin()) 
  {
    Serial.println(F("Found a TSL2591 sensor"));
  } 
  else 
  {
    Serial.println(F("No sensor found ... check your wiring?"));
    while (1);
  }
    
  /* Display some basic information on this sensor */
  displayTSLSensorDetails();
  
  /* Configure the sensor */
  configureTSLSensor();
  //-------------------------------------------------------------------------------------------------------------------
  pinMode(LIGHTSENSORPIN,  INPUT);
  //-------------------------------------------------------------------------------------------------------------------
  
  delay(5000); // Let sensor stabilize
}
  //-------------------------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------------------------
void MAXRead(void)
  {
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  //Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
  //Serial.print(F("IR: ")); Serial.print(ir);  Serial.print(F("  "));
  //Serial.print(F("Full: ")); Serial.print(full); Serial.print(F("  "));
  //Serial.print(F("Visible: ")); Serial.print(full - ir); Serial.print(F("  "));
  Serial.print(F("MAX: ")); Serial.println(tsl.calculateLux(full, ir), 6);
  }

void STLRead(void)
  {
    float lux = max44009.readLux();

  Serial.print(F("STL: "));
  if (isnan(lux)) {
    Serial.println(F("OVERRANGE"));
  } else {
    Serial.println(lux);
  }

  // Check register status (note: reading this clears the interrupt!)
  bool regStatus = max44009.getInterruptStatus();
  if (regStatus) {
    Serial.println(F("IRQ Fired"));
  }
  }

void TEMTRead(void){
    float reading = analogRead(LIGHTSENSORPIN); //Read light level
    Serial.print(F("TEMT: "));Serial.println(reading);                    //Display reading in serial monitor
  }
void loop() {
  Serial.println("----------------------");
  //-------------------------------------------------------------------------------------------------------------------
  STLRead();
  //-------------------------------------------------------------------------------------------------------------------
  MAXRead();
  //-------------------------------------------------------------------------------------------------------------------
  TEMTRead();
  //-------------------------------------------------------------------------------------------------------------------
  Serial.println("----------------------");
  delay(10);
}