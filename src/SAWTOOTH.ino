#include <DueTimer.h>

const int dacPin = DAC1;  // DAC1 = A13

const float pi = 3.14159;
const float sawFreq = 1.0;     // תדר שן מסור
const float sineFreq = 50.0;   // תדר רעש
const int dacMax = 4095;       // 12-bit

volatile float currentTime = 0.0;  // זמן כולל במשתנה גלובלי
const float dt = 0.001;            // 1ms = 1kHz

void generateSignal() {
  currentTime += dt;

  // אות שן מסור
  float saw = fmod(currentTime * sawFreq, 1.0);

  // רעש סינוס
  float noise = 0.05 * sin(2 * pi * sineFreq * currentTime);

  // חיבור אות
  float signal = saw + noise;

  // חיתוך והמרה ל־DAC
  signal = constrain(signal, 0.0, 1.0);
  int dacValue = signal * dacMax;
  analogWrite(dacPin, dacValue);
}

void setup() {
  analogWriteResolution(12);  // רזולוציה של 12 ביט

  // הפעלת טיימר 3 שיקרא את generateSignal כל 1000 מיקרו־שניות
  Timer3.attachInterrupt(generateSignal);
  Timer3.start(1000);  // 1000 מיקרו־שניות = 1kHz
}

void loop() {
 
}
