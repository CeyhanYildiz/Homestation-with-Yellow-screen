#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// === WiFi credentials ===
const char* wifiName = "telenet-Ster";
const char* wifiPassword = "0485541209n";

// === UDP settings ===
const char* udpIP = "192.168.1.77";
const int udpPort = 50006;

// === DHT setup ===
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// === Sound sensor setup ===
const int sampleWindow = 50; // milliseconds
const int AMP_PIN = A0;
unsigned int sample;
unsigned int peakToPeak = 0;

// === Dust Sensor ===
const int dustSensorPin = D4;
volatile unsigned long lowStartTime = 0;
volatile unsigned long lowPulseOccupancy = 0;
unsigned long sampleTime_ms = 2000;
unsigned long dustStartTime = 0;
float dustRatio = 0;
float dustConcentration = 0;

// === AmbiMate I2C Setup ===
#define AMBIMATE_I2C_ADDR  0x2A
#define SCAN_REGISTER      0xC0
#define SCAN_FULL_SET      0xFF

float ambiTemp = 0.0, ambiHumidity = 0.0, ambiBatteryVoltage = 0.0;
uint16_t ambiLight = 0, ambiCO2 = 0, ambiVOC = 0;
uint8_t ambiAudio = 0;

// === Networking ===
WiFiUDP udp;
ESP8266WebServer server(80);

// === General ===
float temperature = 0.0;
float humidity = 0.0;
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
  Wire.begin();

  // WiFi setup
  WiFi.begin(wifiName, wifiPassword);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

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
    json += "\"DustConcentration\":" + String(dustConcentration, 2) + ",";
    json += "\"AmbiTemp\":" + String(ambiTemp, 1) + ",";
    json += "\"AmbiHumidity\":" + String(ambiHumidity, 1) + ",";
    json += "\"AmbiLight\":" + String(ambiLight) + ",";
    json += "\"AmbiAudio\":" + String(ambiAudio) + ",";
    json += "\"AmbiCO2\":" + String(ambiCO2) + ",";
    json += "\"AmbiVOC\":" + String(ambiVOC) + ",";
    json += "\"AmbiBattery\":" + String(ambiBatteryVoltage, 2);
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
    dustRatio = lowPulseOccupancy / (sampleTime_ms * 10.0);
    dustConcentration = 1.1 * pow(dustRatio, 3) - 3.8 * pow(dustRatio, 2) + 520 * dustRatio + 0.62;
    lowPulseOccupancy = 0;
    dustStartTime = millis();
  }

  // === AmbiMate Sensor Read ===
  Wire.beginTransmission(AMBIMATE_I2C_ADDR);
  Wire.write(SCAN_REGISTER);
  Wire.write(SCAN_FULL_SET);
  Wire.endTransmission();
  delay(500);
  Wire.beginTransmission(AMBIMATE_I2C_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(AMBIMATE_I2C_ADDR, 15);
  if (Wire.available() >= 15) {
    uint8_t data[15];
    for (int i = 0; i < 15; i++) data[i] = Wire.read();
    ambiTemp = ((data[1] << 8) | data[0]) / 10.0;
    ambiHumidity = ((data[3] << 8) | data[2]) / 10.0;
    ambiLight = (data[5] << 8) | data[4];
    ambiAudio = data[6];
    ambiCO2 = (data[8] << 8) | data[7];
    ambiVOC = (data[10] << 8) | data[9];
    ambiBatteryVoltage = ((data[14] << 8) | data[13]) / 1024.0 * (3.3 / 0.330);
  }

  // === UDP Send ===
  String message = "Temp: " + String(temperature) + " C, Hum: " + String(humidity) +
                   " %, Sound: " + String(peakToPeak) + ", Dust: " + String(dustConcentration, 2) +
                   ", AmbiTemp: " + String(ambiTemp) + " C, AmbiHum: " + String(ambiHumidity) +
                   " %, Light: " + String(ambiLight) + " Lux, Audio: " + String(ambiAudio) +
                   " dB, CO2: " + String(ambiCO2) + " PPM, VOC: " + String(ambiVOC) +
                   " PPB, Battery: " + String(ambiBatteryVoltage, 2) + " V";
  udp.beginPacket(udpIP, udpPort);
  udp.print(message);
  udp.endPacket();

  server.handleClient();

  if (millis() - lastPrintTime >= printInterval) {
    Serial.println("IP: " + WiFi.localIP().toString());
    lastPrintTime = millis();
  }

  delay(1000);
}
