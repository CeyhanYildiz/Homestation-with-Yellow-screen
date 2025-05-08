#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


// WiFi info
const char* wifiName = "504F94A12106";
const char* wifiPassword = "";

// UDP settings (optional)
const char* udpIP = "192.168.1.77";
const int udpPort = 50006;

// DHT Sensor setup
#define DHTPIN D3      // D3 is GPIO0 on NodeMCU
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Networking
WiFiUDP udp;
ESP8266WebServer server(80);

// Variables to hold sensor data
float temperature = 0.0;
float humidity = 0.0;

// Sound sensor setup
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
const int AMP_PIN = A0;       // Preamp output pin connected to A0
unsigned int sample;
unsigned int peakToPeak = 0;

// Timer for printing IP
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 10000; // 10 seconds

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println("Connecting to WiFi...");
  WiFi.begin(wifiName, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println("IP address: " + WiFi.localIP().toString());

  udp.begin(udpPort);

  dht.begin();

  // Setup web server route
  server.on("/sensdata", HTTP_GET, []() {
    String json = "{";
    json += "\"Temperature\":" + String(temperature, 1) + ",";
    json += "\"Humidity\":" + String(humidity, 1) + ",";
    json += "\"SoundLevel\":" + String(peakToPeak);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Read DHT sensor
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    temperature = temp;
    humidity = hum;
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }

  // Measure sound peak-to-peak
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(AMP_PIN);
    if (sample < 1024) {
      if (sample > signalMax) {
        signalMax = sample;
      } else if (sample < signalMin) {
        signalMin = sample;
      }
    }
  }
  peakToPeak = signalMax - signalMin;
  //Serial.print("Sound Level (peakToPeak): ");
  //Serial.println(peakToPeak);

  // Optional: Send UDP packet
  String message = "Temp: " + String(temperature) + " C, Hum: " + String(humidity) + " %, Sound: " + String(peakToPeak);
  udp.beginPacket(udpIP, udpPort);
  udp.print(message);
  udp.endPacket();

  // Handle web server
  server.handleClient();

  // Print IP every 10 seconds
  if (millis() - lastPrintTime >= printInterval) {
    Serial.println("Current IP: " + WiFi.localIP().toString());
    lastPrintTime = millis();
  }

  delay(2000); // Delay to match DHT sensor update speed
}
