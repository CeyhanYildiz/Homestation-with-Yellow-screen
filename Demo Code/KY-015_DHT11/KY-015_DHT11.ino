#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN D3      // D3 is GPIO0 on NodeMCU
#define DHTTYPE DHT11  // We're using a DHT11 sensor

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200); // ESP8266 prefers 115200 baud rate
  Serial.println();
  Serial.println("DHT11 sensor test (NodeMCU ESP8266)");

  dht.begin();
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("°C  Humidity: ");
  Serial.print(hum);
  Serial.println("%");

  delay(500);
}
