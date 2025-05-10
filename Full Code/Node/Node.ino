#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SoftwareSerial.h> // For debugging if needed (not used here since ESP8266 has one hardware serial)

#define AMBIMATE_I2C_ADDR  0x2A  // I2C address of the AmbiMate sensor
#define SCAN_REGISTER      0xC0  // Register to initiate data scan
#define SCAN_FULL_SET      0xFF  // Value to scan all sensor registers

// WiFi credentials
const char* WIFI_SSID = "telenet-Ster";
const char* WIFI_PASSWORD = "0485541209n";

// UDP target
const char* UDP_IP = "192.168.1.77";
const int UDP_PORT = 50006;

// DHT sensor setup
#define DHT_PIN D3
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Sound sensor setup
const int AMP_PIN = A0;
const int SAMPLE_WINDOW = 50; // milliseconds

// Networking
WiFiUDP udp;
ESP8266WebServer server(80);

// Sensor data variables
float temperature = 0.0;
float humidity = 0.0;
unsigned int soundLevel = 0;

// Timer for IP logging
unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 10000;

void setup() {
  // Serial communication
  Serial.begin(115200);
  delay(100);

  // WiFi setup
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.println("IP address: " + WiFi.localIP().toString());

  udp.begin(UDP_PORT);
  dht.begin();

  // AmbiMate sensor initialization
  Wire.begin();
  Serial.println("AmbiMate MS4 Sensor Initialization...");

  // Web server endpoint for sensor data
  server.on("/sensdata", HTTP_GET, []() {
    String json = "{";
    json += "\"Temperature\":" + String(temperature, 1) + ",";
    json += "\"Humidity\":" + String(humidity, 1) + ",";
    json += "\"SoundLevel\":" + String(soundLevel) + ",";
    json += "\"AmbiMateTemperature\":" + String(getAmbiMateTemperature(), 1) + ",";
    json += "\"AmbiMateHumidity\":" + String(getAmbiMateHumidity(), 1) + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  readTemperatureHumidity();
  readSoundLevel();
  sendUDPMessage();
  server.handleClient();

  if (millis() - lastPrintTime >= PRINT_INTERVAL) {
    Serial.println("Current IP: " + WiFi.localIP().toString());
    lastPrintTime = millis();
  }

  delay(2000);
}

// Read temperature and humidity from DHT11
void readTemperatureHumidity() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    temperature = temp;
    humidity = hum;
  }
}

// Read sound level from analog pin
void readSoundLevel() {
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  while (millis() - startMillis < SAMPLE_WINDOW) {
    unsigned int sample = analogRead(AMP_PIN);
    if (sample < 1024) {
      if (sample > signalMax) signalMax = sample;
      if (sample < signalMin) signalMin = sample;
    }
  }

  soundLevel = signalMax - signalMin;
}

// Send a UDP message with current sensor values including AmbiMate data
void sendUDPMessage() {
  float ambiTemp = getAmbiMateTemperature();
  float ambiHum = getAmbiMateHumidity();

  String message = "Temp: " + String(temperature) + " C, "
                 + "Hum: " + String(humidity) + " %, "
                 + "Sound: " + String(soundLevel) + ", "
                 + "AmbiMate Temp: " + String(ambiTemp) + " C, "
                 + "AmbiMate Hum: " + String(ambiHum) + " %";

  udp.beginPacket(UDP_IP, UDP_PORT);
  udp.print(message);
  udp.endPacket();
}

// Function to get AmbiMate temperature
float getAmbiMateTemperature() {
  Wire.beginTransmission(AMBIMATE_I2C_ADDR);
  Wire.write(SCAN_REGISTER);
  Wire.write(SCAN_FULL_SET);
  Wire.endTransmission();
  
  delay(500); // Wait for sensor to update

  // Read sensor data from registers (0x00 - 0x0E)
  Wire.beginTransmission(AMBIMATE_I2C_ADDR);
  Wire.write(0x00);  // Start reading from register 0x00
  Wire.endTransmission();
  Wire.requestFrom(AMBIMATE_I2C_ADDR, 15);  // Read 15 bytes of sensor data

  if (Wire.available() < 15) {
    Serial.println("Error: Not enough data received.");
    return 0.0;
  }

  uint8_t data[15];
  for (int i = 0; i < 15; i++) {
    data[i] = Wire.read();
  }

  // Extract AmbiMate temperature
  return ((data[1] << 8) | data[0]) / 10.0;  // Temperature in °C
}

// Function to get AmbiMate humidity
float getAmbiMateHumidity() {
  Wire.beginTransmission(AMBIMATE_I2C_ADDR);
  Wire.write(SCAN_REGISTER);
  Wire.write(SCAN_FULL_SET);
  Wire.endTransmission();
  
  delay(500); // Wait for sensor to update

  // Read sensor data from registers (0x00 - 0x0E)
  Wire.beginTransmission(AMBIMATE_I2C_ADDR);
  Wire.write(0x00);  // Start reading from register 0x00
  Wire.endTransmission();
  Wire.requestFrom(AMBIMATE_I2C_ADDR, 15);  // Read 15 bytes of sensor data

  if (Wire.available() < 15) {
    Serial.println("Error: Not enough data received.");
    return 0.0;
  }

  uint8_t data[15];
  for (int i = 0; i < 15; i++) {
    data[i] = Wire.read();
  }

  // Extract AmbiMate humidity
  return ((data[3] << 8) | data[2]) / 10.0;     // Humidity in %
}
