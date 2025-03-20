#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

// Replace with your network credentials
const char* ssid = "504F94A12106";      // Your WiFi SSID
const char* password = "";  // Your WiFi Password

// IP and port for sending and receiving UDP packets
const char* udpIP = "192.168.1.77";  // IP to send UDP packets to (Miniserver IP)
const int udpPort = 50006;           // Port to send UDP packets to

WiFiUDP udp;  // Initialize WiFiUDP object

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
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());  // Print local IP address

    // Start listening for UDP packets on the specified port
    udp.begin(udpPort);  // Bind to UDP port
}

void loop() {
    // Send UDP packets to the specified IP and port
    sendUDP("Temp: 21");
    delay(1000);  // Wait for a second before sending another packet

    sendUDP("Test: 9999");
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
