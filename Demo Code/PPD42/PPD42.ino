// Define the pin for the dust sensor
const int sensorPin = 2;  // Connect the OUT pin of PPD42 to pin 2 on Nano 33 BLE
volatile int pulseCount = 0;
unsigned long lastTime = 0;
float dustDensity = 0.0;

// This function will be triggered when a pulse is detected
void pulseCounter() {
  pulseCount++;
}

void setup() {
  Serial.begin(9600);

  // Set the sensor pin as input
  pinMode(sensorPin, INPUT);

  // Attach an interrupt to the sensor pin to count the pulses
  attachInterrupt(digitalPinToInterrupt(sensorPin), pulseCounter, FALLING); // Trigger on falling edge
  
  // Allow the sensor to stabilize (typically around 30 seconds)
  delay(30000);  // Wait for 30 seconds for the sensor to warm up
}

void loop() {
  // Read the pulse count over a fixed interval (e.g., 1 second)
  unsigned long currentTime = millis();
  
  // Every second, calculate dust density
  if (currentTime - lastTime >= 1000) {
    lastTime = currentTime;
    
    // Calculate the dust concentration (PPM)
    // The formula varies based on the manufacturer. This is a general estimation.
    dustDensity = pulseCount / 10.0;  // Divide by a factor to adjust, e.g., 10
    
    // Print out the result
    Serial.print("Dust Concentration: ");
    Serial.print(dustDensity);
    Serial.println(" ug/m3");
    
    // Reset the pulse count for the next cycle
    pulseCount = 0;
  }
}
