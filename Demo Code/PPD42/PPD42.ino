const int sensorPin = 2;  // PPD42 OUT connected here
volatile unsigned long lowStartTime = 0;
volatile unsigned long lowPulseOccupancy = 0;
unsigned long sampleTime_ms = 2000;  // 2 seconds
unsigned long startTime = 0;

float ratio = 0;
float concentration = 0;

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), pulseISR, CHANGE);
  startTime = millis();
}

void loop() {
  if ((millis() - startTime) >= sampleTime_ms) {
    // Calculate ratio and concentration
    ratio = lowPulseOccupancy / (sampleTime_ms * 10.0);  // %
    concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62;

    Serial.print("Concentration = ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf\n");

    // Reset for next cycle
    lowPulseOccupancy = 0;
    startTime = millis();
  }
}

// ISR to calculate LOW pulse duration
void pulseISR() {
  int val = digitalRead(sensorPin);

  if (val == LOW) {
    // Pulse just went LOW
    lowStartTime = micros();
  } else {
    // Pulse just went HIGH
    if (lowStartTime > 0) {
      lowPulseOccupancy += micros() - lowStartTime;
      lowStartTime = 0;
    }
  }
}
