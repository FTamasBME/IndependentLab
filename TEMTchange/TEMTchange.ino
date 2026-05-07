#include <Adafruit_MAX44009.h>
#include <Wire.h>

Adafruit_MAX44009 max44009;

#define LIGHTSENSORPIN A0
#define PWM_LED_L 3
#define PWM_LED_R 5

const unsigned long stepTime = 6250; 
unsigned long lastStepMicros = 0;
bool ledsOn = false;

float totalTEMT = 0;
float ambientTEMT = 0;
float maxLedContribution = 0; 

float filteredContribution = 0;        
const float filterWeight = 0.15;         
float lastPrintedContribution = -999.0; 
const float JUMP_THRESHOLD = 6;

float avgNoise = 0.5;          // Az aktuális zajszint becslése
const float noiseWeight = 0.001; // Milyen gyorsan kövesse a zajszint változását
float dynamicThreshold = 6.0;  // Ez fog változni futás közben
const float SENSITIVITY = 1.1; // Szorzó: hányszorosa legyen a küszöb a zajnak
const float MIN_THRESHOLD = 0.5; // Abszolút minimum küszöb

void setup() {
  Serial.begin(115200);
  if (!max44009.begin()) { while (1); }

  max44009.setMode(MAX44009_MODE_MANUAL_CONTINUOUS);
  max44009.setIntegrationTime(MAX44009_INTEGRATION_6_25MS);

  pinMode(PWM_LED_L, OUTPUT);
  pinMode(PWM_LED_R, OUTPUT);
  pinMode(LIGHTSENSORPIN, INPUT);

  delay(1000); 
  lastStepMicros = micros();
}

float smoothAnalogRead(int pin) {
  long sum = 0;
  for(int i=0; i<16; i++) { sum += analogRead(pin); }
  return sum / 16.0;
}

void loop() {
  unsigned long currentMicros = micros();

  if (currentMicros - lastStepMicros >= stepTime) {
    lastStepMicros = currentMicros;

    float measuredTEMT = smoothAnalogRead(LIGHTSENSORPIN);

    if (ledsOn) {
      totalTEMT = measuredTEMT;
      digitalWrite(PWM_LED_L, LOW);
      digitalWrite(PWM_LED_R, LOW);
      ledsOn = false;
    }
    else {
      ambientTEMT = measuredTEMT;
      digitalWrite(PWM_LED_L, HIGH);
      digitalWrite(PWM_LED_R, HIGH);
      ledsOn = true;

      float rawContribution = totalTEMT - ambientTEMT;
      if (rawContribution < 0) rawContribution = 0;

      filteredContribution = (rawContribution * filterWeight) + (filteredContribution * (1.0 - filterWeight));

      float currentJitter = abs(rawContribution - filteredContribution);

      avgNoise = (currentJitter * noiseWeight) + (avgNoise * (1.0 - noiseWeight));

      dynamicThreshold = (avgNoise * SENSITIVITY) + MIN_THRESHOLD;

      if (dynamicThreshold > 30) dynamicThreshold = 30;

      showEventDynamic(currentMicros, rawContribution); 
      //showEvent(currentMicros, rawContribution); 
      //dataLogger(currentMicros, rawContribution);
    }
  }
}

void showEventDynamic(unsigned long currentMicros, float rawContribution) {
  if (abs(filteredContribution - lastPrintedContribution) >= dynamicThreshold) {
    
    Serial.print("Time:");Serial.print(currentMicros);
    Serial.print("|Theshold:"); Serial.print(dynamicThreshold, 1); // Kiírjuk az aktuális küszöböt
    Serial.print("|Noise:"); Serial.print(avgNoise, 2);      // Kiírjuk az átlagos zajt
    Serial.print("|Event");
    
    lastPrintedContribution = filteredContribution; 
    
    Serial.print("|Filtered:");
    Serial.print(filteredContribution, 1);
    Serial.print("|Raw:");
    Serial.print(rawContribution, 1);
    Serial.print("|Ambient:");
    Serial.println(ambientTEMT, 1);
  }
}

void showEvent(unsigned long currentMicros, float rawContribution) {
  if (abs(filteredContribution - lastPrintedContribution) >= JUMP_THRESHOLD) {
    
    Serial.print(currentMicros);
    Serial.print("|");
    Serial.print(filterWeight, 3);
    Serial.print("|");
    Serial.print(JUMP_THRESHOLD, 1);
    Serial.print("|");
    Serial.print(1); 
    
    lastPrintedContribution = filteredContribution; 
    
    Serial.print("|");
    Serial.print(filteredContribution, 1);
    Serial.print("|");
    Serial.print(rawContribution, 1);
    Serial.print("|");
    Serial.println(ambientTEMT, 1);
  }
}

void dataLogger(unsigned long currentMicros, float rawContribution) {
  Serial.print(currentMicros);
  Serial.print("|");
  Serial.print(filterWeight, 3);
  Serial.print("|");
  Serial.print(JUMP_THRESHOLD, 1);
  Serial.print("|");

  if (abs(filteredContribution - lastPrintedContribution) >= dynamicThreshold) {
    Serial.print(1); 
    lastPrintedContribution = filteredContribution; 
  } else {
    Serial.print(0);
  }

  Serial.print("|");
  Serial.print(filteredContribution, 1);
  Serial.print("|");
  Serial.print(rawContribution, 1);
  Serial.print("|");
  Serial.println(ambientTEMT, 1);
}