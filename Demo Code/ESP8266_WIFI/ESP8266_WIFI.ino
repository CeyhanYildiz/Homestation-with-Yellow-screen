#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// Replace with your network credentials
const char* ssid = "504F94A12106";      // Your WiFi SSID
const char* password = "";  // Your WiFi Password

// IP and port for sending and receiving UDP packets
const char* udpIP = "192.168.1.77";  // IP to send UDP packets to (Miniserver IP)
const int udpPort = 50006;           // Port to send UDP packets to

WiFiUDP udp;  // Initialize WiFiUDP object

#define DHTPIN 3      // Use Digital Pin 3 (D3)
#define DHTTYPE DHT11 // KY-015 uses DHT11 sensor

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT11 sensor

void setup() {
    Serial.begin(9600);  // Start Serial Monitor at 9600 baud rate
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
    
    // Initialize the DHT sensor
    dht.begin();
}

void loop() {
    // Read the ALS-PT19 light sensor value (analog)
    int sensorValue = analogRead(A0);
    
    // Print the raw sensor value to the serial monitor
    Serial.print("ALS-PT19 Raw Value: ");
    Serial.println(sensorValue);

    // Scale the raw sensor value to a 0-100 range (percentage)
    int scaledValue = map(sensorValue, 0, 1023, 0, 100);
    
    // Print the scaled light intensity (percentage) to the serial monitor
    Serial.print("Scaled Light Intensity (0-100): ");
    Serial.println(scaledValue);

    // Create a message for light intensity
    String lightMessage = "Light Intensity: " + String(scaledValue) + "%";
    
    // Send the light intensity via UDP
    sendUDP(lightMessage);

    // Read temperature and humidity from the DHT11 sensor
    float temp = dht.readTemperature(); // Temperature in Celsius
    float hum = dht.readHumidity();    // Humidity in percentage

    if (isnan(temp) || isnan(hum)) {
        Serial.println("Failed to read from DHT sensor!");
    } else {
        // Print temperature and humidity to the serial monitor
        Serial.print("Temperature: ");
        Serial.print(temp);
        Serial.print("°C  Humidity: ");
        Serial.print(hum);
        Serial.println("%");

        // Create a message for temperature and humidity
        String tempHumidityMessage = "Temperature: " + String(temp) + "°C, Humidity: " + String(hum) + "%";
        
        // Send the temperature and humidity data via UDP
        sendUDP(tempHumidityMessage);
    }

    delay(2000);  // Delay to avoid too frequent sensor reads

    // Listen for incoming UDP packets (you can implement logic if needed)
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
