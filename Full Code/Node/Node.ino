#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

// Replace with your network credentials
const char* ssid = "504F94A12106";  // Your WiFi SSID
const char* password = "";          // Your WiFi Password

// IP and port for sending and receiving UDP packets
const char* udpIP = "192.168.1.77";  // IP to send UDP packets to (Miniserver IP or ESP32 IP)
const int udpPort = 50000;           // Port to send UDP packets to

WiFiUDP udp;  // Initialize WiFiUDP object

// Sensor Data (for demonstration purposes)
float sens1 = 21.0f;
float sens2 = 18.2f;
float sens3 = 25.5f;
float sens4 = 9.8f;

void setup() {
    Serial.begin(115200);  // Start Serial Monitor
    delay(1000);

    Serial.println();
    Serial.println("Connecting to WiFi...");

    WiFi.begin(ssid, password);  // Start connecting to WiFi

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected!");
    
    // Print the ESP8266 IP address once connected to Wi-Fi
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());  // Print the local IP address

    // Start listening for UDP packets on the specified port
    udp.begin(udpPort);  // Bind to UDP port
}

void loop() {
    // Send UDP packets with sensor data
    sendUDP("Sens1: " + String(sens1));
    delay(1000);  // Wait for a second before sending another packet

    sendUDP("Sens2: " + String(sens2));
    delay(1000);  // Wait for a second before sending another packet

    sendUDP("Sens3: " + String(sens3));
    delay(1000);  // Wait for a second before sending another packet

    sendUDP("Sens4: " + String(sens4));
    delay(1000);  // Wait for a second before sending another packet

    // Listen for incoming UDP packets
    receiveUDP();
}

// Function to send a UDP message
void sendUDP(String message) {
    udp.beginPacket(udpIP, udpPort);  // Start UDP packet to the specified IP and port
    udp.print(message);               // Write message to the packet
    udp.endPacket();                  // End the UDP packet
    Serial.print("Sent: ");
    Serial.println(message);
}

// Function to receive incoming UDP packets
void receiveUDP() {
    int packetSize = udp.parsePacket();  // Check if there is a packet
    if (packetSize) {
        char packetBuffer[packetSize];  // Create a buffer to store incoming data
        udp.read(packetBuffer, packetSize);  // Read the incoming data into the buffer
        Serial.print("Received: ");
        Serial.println(packetBuffer);  // Print received message
    }
}
