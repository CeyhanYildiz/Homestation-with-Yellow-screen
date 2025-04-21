#include <SoftwareSerial.h>
SoftwareSerial mySerial(D2, D1); // RX, TX — match your wiring

void setup() {
  Serial.begin(115200); // For debug via USB
  mySerial.begin(9600);
  Serial.println("ESP8266 Ready");
}

void loop() {
  mySerial.println("Hello from ESP8266");
  delay(1000);

  if (mySerial.available()) {
    String msg = mySerial.readStringUntil('\n');
    Serial.println("ESP32 replied: " + msg);
  }
}
