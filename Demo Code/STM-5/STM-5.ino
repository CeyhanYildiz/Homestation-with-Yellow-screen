#include <Wire.h>

#define AMBIMATE_I2C_ADDR  0x2A  // I2C address of the AmbiMate sensor
#define SCAN_REGISTER      0xC0  // Register to initiate data scan
#define SCAN_FULL_SET      0xFF  // Value to scan all sensor registers

void setup() {
    Serial.begin(115200);
    while (!Serial);  // Wait for Serial Monitor to open

    Wire.begin();  // Initialize I2C

    Serial.println("AmbiMate MS4 Sensor Initialization...");
}

void loop() {
    // Request a full data scan
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
        return;
    }

    uint8_t data[15];
    for (int i = 0; i < 15; i++) {
        data[i] = Wire.read();
    }

    // Extract sensor values
    float temperature = ((data[1] << 8) | data[0]) / 10.0;  // Temperature in °C
    float humidity = ((data[3] << 8) | data[2]) / 10.0;     // Humidity in %
    uint16_t light = (data[5] << 8) | data[4];             // Light in Lux
    uint8_t audio = data[6];                               // Audio in dB
    uint16_t eCO2 = (data[8] << 8) | data[7];             // eCO2 in PPM
    uint16_t VOC = (data[10] << 8) | data[9];             // VOC in PPB
    float batteryVoltage = ((data[14] << 8) | data[13]) / 1024.0 * (3.3 / 0.330);  // Battery Voltage

    // Display data
    Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Light: "); Serial.print(light); Serial.println(" Lux");
    Serial.print("Audio: "); Serial.print(audio); Serial.println(" dB");
    Serial.print("eCO2: "); Serial.print(eCO2); Serial.println(" PPM");
    Serial.print("VOC: "); Serial.print(VOC); Serial.println(" PPB");
    Serial.print("Battery Voltage: "); Serial.print(batteryVoltage); Serial.println(" V");
    
    Serial.println("----------------------");
    
    delay(2000);  // Wait before next reading
}
