#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// WiFi credentials
const char* wifiName = "telenet-Ster";
const char* wifiPassword = "0485541209n";

// UDP settings
const char* udpIP = "192.168.1.77";
const int udpPort = 50006;

// DHT setup
#define DHTPIN D3 // GPIO0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Sound sensor setup
const int sampleWindow = 50; // milliseconds
const int AMP_PIN = A0;
unsigned int sample;
unsigned int peakToPeak = 0;

// PPD42 Dust Sensor
const int dustSensorPin = D4; // GPIO2
volatile unsigned long lowStartTime = 0;
volatile unsigned long lowPulseOccupancy = 0;
unsigned long sampleTime_ms = 2000;
unsigned long dustStartTime = 0;
float dustRatio = 0;
float dustConcentration = 0;

// Sensor values
float temperature = 0.0;
float humidity = 0.0;

// Networking
WiFiUDP udp;
ESP8266WebServer server(80);

// Timer
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 10000;

void IRAM_ATTR pulseISR() {
  int val = digitalRead(dustSensorPin);
  if (val == LOW) {
    lowStartTime = micros();
  } else {
    if (lowStartTime > 0) {
      lowPulseOccupancy += micros() - lowStartTime;
      lowStartTime = 0;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // WiFi setup
  WiFi.begin(wifiName, wifiPassword);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // Sensors and networking
  dht.begin();
  udp.begin(udpPort);
  pinMode(dustSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(dustSensorPin), pulseISR, CHANGE);
  dustStartTime = millis();

  server.on("/sensdata", HTTP_GET, []() {
    String json = "{";
    json += "\"Temperature\":" + String(temperature, 1) + ",";
    json += "\"Humidity\":" + String(humidity, 1) + ",";
    json += "\"SoundLevel\":" + String(peakToPeak) + ",";
    json += "\"DustConcentration\":" + String(dustConcentration, 2);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // === DHT Sensor ===
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (!isnan(temp) && !isnan(hum)) {
    temperature = temp;
    humidity = hum;
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }

  // === Sound Sensor ===
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(AMP_PIN);
    if (sample < 1024) {
      if (sample > signalMax) signalMax = sample;
      if (sample < signalMin) signalMin = sample;
    }
  }
  peakToPeak = signalMax - signalMin;

  // === Dust Sensor ===
  if ((millis() - dustStartTime) >= sampleTime_ms) {
    dustRatio = lowPulseOccupancy / (sampleTime_ms * 10.0); // %
    dustConcentration = 1.1 * pow(dustRatio, 3) - 3.8 * pow(dustRatio, 2) + 520 * dustRatio + 0.62;
    lowPulseOccupancy = 0;
    dustStartTime = millis();

    Serial.print("Dust Concentration: ");
    Serial.print(dustConcentration);
    Serial.println(" pcs/0.01cf");
  }

  // === UDP Send ===
  String message = "Temp: " + String(temperature) + " C, Hum: " + String(humidity) + 
                   " %, Sound: " + String(peakToPeak) + ", Dust: " + String(dustConcentration, 2);
  udp.beginPacket(udpIP, udpPort);
  udp.print(message);
  udp.endPacket();

  // === Web Server ===
  server.handleClient();

  // === Optional IP print ===
  if (millis() - lastPrintTime >= printInterval) {
    Serial.println("IP: " + WiFi.localIP().toString());
    lastPrintTime = millis();
  }

  delay(1000); // adjust for sensor stability, keep it short
}
