#include <Arduino.h>

void sendCommand(byte cmd[], int len);
bool readResponse(byte response[], int len);
int parseCO2(byte response[]);

void setup() {
    Serial.begin(115200);   // Serial Monitor
    Serial1.begin(9600);    // MH-Z19C Serial
    delay(1000);
    Serial.println("MH-Z19C CO2 Sensor Initialized...");
}

void loop() {
    byte cmdReadCO2[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    byte response[9];

    sendCommand(cmdReadCO2, 9);

    if (readResponse(response, 9)) {
        int co2 = parseCO2(response);
        
        Serial.print("CO2 Concentration: ");
        Serial.print(co2);
        Serial.println(" ppm");

        // Determine air quality and location
        if (co2 <= 450) {
            Serial.println("🌿 Likely Outdoor Air (Fresh and Clean)");
        } else if (co2 > 450 && co2 <= 1000) {
            Serial.println("🏠 Typical Indoor Air (Well-Ventilated)");
        } else if (co2 > 1000 && co2 <= 2000) {
            Serial.println("😷 Poor Ventilation! Open a Window!");
        } else if (co2 > 2000 && co2 <= 5000) {
            Serial.println("⚠️ Stale Air! Possible Headaches and Drowsiness!");
        } else {
            Serial.println("🚨 Dangerous CO2 Levels! Leave the Area Immediately!");
        }
    } else {
        Serial.println("❌ Failed to read CO2 data");
    }

    delay(2000);
}

void sendCommand(byte cmd[], int len) {
    Serial1.write(cmd, len);
}

bool readResponse(byte response[], int len) {
    unsigned long startTime = millis();
    while (Serial1.available() < len) {
        if (millis() - startTime > 1000) {
            return false;  // Timeout
        }
    }
    for (int i = 0; i < len; i++) {
        response[i] = Serial1.read();
    }
    return true;
}

int parseCO2(byte response[]) {
    if (response[0] != 0xFF || response[1] != 0x86) {
        return -1;  // Invalid response
    }
    int high = response[2];
    int low = response[3];
    return (high << 8) + low;
}
