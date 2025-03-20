#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"

SCD30 airSensor;

void setup() {
    Serial.begin(115200);
    while (!Serial);  // Wait for Serial Monitor to open

    Wire.begin();

    if (!airSensor.begin()) {
        Serial.println("SCD30 not detected. Check wiring!");
        while (1);
    }

    // Set measurement interval (default is 2s)
    airSensor.setMeasurementInterval(2);

    // Enable automatic self-calibration (ASC)
    airSensor.setAutoSelfCalibration(true);

    Serial.println("SCD30 Initialized.");
}

void loop() {
    if (airSensor.dataAvailable()) {
        Serial.print("CO2: ");
        Serial.print(airSensor.getCO2());
        Serial.print(" ppm | Temp: ");
        Serial.print(airSensor.getTemperature(), 1);
        Serial.print(" °C | Humidity: ");
        Serial.print(airSensor.getHumidity(), 1);
        Serial.println(" %");
    } else {
        Serial.println("Waiting for new data...");
    }

    delay(2000);
}
