# node

Esp82 based wifi module to communicate with the loxone server

<img src="../../Photos/esp.jpg" alt="node" width="200">

## data collection

This module is the most important part of the whole project. It is used to collect all sensor data and form a general message to send towards the loxone server over udp.

The active sensors within the final prototype of this project are:
 - [ky-015_dht11](../../Demo%20Code/KY-015_DHT11/KY-015_DHT11.md)
 - [max_4466](../../Demo%20Code/MAX_4466/MAX_4466.md)
 - [ppd42](../../Demo%20Code/PPD42/ppd42.md)
 - [stm-5](../../Demo%20Code/STM-5/STM-5.md)
 - [mh_z19c](../../Demo%20Code/MH_Z19C/mh_z19c.md)

## wifi

All sensor data in the project is send over wifi, this can be achieved using the wifi functionality on the esp82 chip. After the data is send to the loxone server over udp it will be available for all other device including the homsecreen module.

```c
//in void loop()
// === UDP Send ===
  String message = "Temp: " + String(temperature) + " C, Hum: " + String(humidity) +
                   " %, Sound: " + String(peakToPeak) + ", Dust: " + String(dustConcentration, 2) +
                   ", AmbiTemp: " + String(ambiTemp) + " C, AmbiHum: " + String(ambiHumidity) +
                   " %, Light: " + String(ambiLight) + " Lux, Audio: " + String(ambiAudio) +
                   " dB, CO2: " + String( ( ambiCO2 + co2 ) / 2 ) + " PPM, VOC: " + String(ambiVOC) +
                   " PPB, Battery: " + String(ambiBatteryVoltage, 2) + " V";
  udp.beginPacket(udpIP, udpPort);
  udp.print(message);
  udp.endPacket();

  server.handleClient();
```

