#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>

// WiFi credentials
const char* ssid = "telenet-Ster";
const char* password = "0485541209n";

// UDP settings
const char* udpIP = "192.168.1.77";  // target IP (optional)
const int udpPort = 50000;

// Sensor data
float sens1 = 21.0f;
float sens2 = 18.2f;
float sens3 = 25.5f;
float sens4 = 9.8f;

// Network objects
WiFiUDP udp;
ESP8266WebServer server(80);  // HTTP server on port 80

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nConnecting to WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start UDP
  udp.begin(udpPort);

  // Setup HTTP endpoint
  server.on("/sensdata", HTTP_GET, []() {
    String json = "{";
    json += "\"sens1\": " + String(sens1) + ",";
    json += "\"sens2\": " + String(sens2) + ",";
    json += "\"sens3\": " + String(sens3) + ",";
    json += "\"sens4\": " + String(sens4);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Send sensor data over UDP
  sendUDP("Sens1: " + String(sens1)); delay(1000);
  sendUDP("Sens2: " + String(sens2)); delay(1000);
  sendUDP("Sens3: " + String(sens3)); delay(1000);
  sendUDP("Sens4: " + String(sens4)); delay(1000);

  // Check for incoming HTTP requests
  server.handleClient();
  receiveUDP();
}

void sendUDP(String message) {
  udp.beginPacket(udpIP, udpPort);
  udp.print(message);
  udp.endPacket();
  Serial.print("Sent: ");
  Serial.println(message);
}

void receiveUDP() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[packetSize + 1];
    udp.read(packetBuffer, packetSize);
    packetBuffer[packetSize] = 0;
    Serial.print("Received: ");
    Serial.println(packetBuffer);
  }
}
