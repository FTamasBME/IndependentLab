#include <Adafruit_MAX44009.h>
#include <Wire.h>

Adafruit_MAX44009 max44009;

#define LIGHTSENSORPIN A0
#define PWM_LED_1 3
#define PWM_LED_2 5

const unsigned long stepTime = 6250; 
unsigned long lastStepMicros = 0;
bool ledsOn = false;

float totalTEMT = 0;
float ambientTEMT = 0;
float maxLedContribution = 0; 

// --- SZŰRÉS ÉS KÜSZÖB ---
float filteredContribution = 0;         // A simított jel
const float filterWeight = 0.2;         // Szűrő erőssége (0.0-1.0). Minél kisebb, annál simább, de lassabb.
float lastPrintedContribution = -999.0; 
const float JUMP_THRESHOLD = 15.0;      // Most már lemehetünk 50-re!

void setup() {
  Serial.begin(115200);
  if (!max44009.begin()) { while (1); }

  max44009.setMode(MAX44009_MODE_MANUAL_CONTINUOUS);
  max44009.setIntegrationTime(MAX44009_INTEGRATION_6_25MS);

  pinMode(PWM_LED_1, OUTPUT);
  pinMode(PWM_LED_2, OUTPUT);
  pinMode(LIGHTSENSORPIN, INPUT);

  delay(1000); 
  lastStepMicros = micros();
}

// Gyors függvény a zajmentesebb analóg olvasáshoz
float smoothAnalogRead(int pin) {
  long sum = 0;
  for(int i=0; i<4; i++) { sum += analogRead(pin); }
  return sum / 4.0;
}

void loop() {
  unsigned long currentMicros = micros();

  if (currentMicros - lastStepMicros >= stepTime) {
    lastStepMicros = currentMicros;

    float measuredTEMT = smoothAnalogRead(LIGHTSENSORPIN); // Többszörös mintavétel

    if (ledsOn) {
      totalTEMT = measuredTEMT;
      digitalWrite(PWM_LED_1, LOW);
      digitalWrite(PWM_LED_2, LOW);
      ledsOn = false;
    }
    else {
      ambientTEMT = measuredTEMT;
      digitalWrite(PWM_LED_1, HIGH);
      digitalWrite(PWM_LED_2, HIGH);
      ledsOn = true;

      processWithFilter();
    }
  }
}

void processWithFilter() {
  float rawContribution = totalTEMT - ambientTEMT;
  if (rawContribution < 0) rawContribution = 0;

  // --- EMA SZŰRŐ ALKALMAZÁSA ---
  // filtered = (új_adat * súly) + (régi_szűrt_adat * (1-súly))
  filteredContribution = (rawContribution * filterWeight) + (filteredContribution * (1.0 - filterWeight));

  // Rekord frissítése a szűrt érték alapján
  if (filteredContribution > maxLedContribution) {
    maxLedContribution = filteredContribution;
  }

  // Ugrás vizsgálata a SZŰRT értéken
  if (abs(filteredContribution - lastPrintedContribution) >= JUMP_THRESHOLD) {
    
    Serial.print("ESEMÉNY!");

    lastPrintedContribution = filteredContribution;
  }
}