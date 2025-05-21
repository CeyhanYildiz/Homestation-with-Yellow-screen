# homescreen

Esp32 with a build in tft display. main display of the project

<img src="../../Photos/YellowScreen2.jpg" alt="yellow screen" width="200">

## display

As mentioned above has the esp32 a custom build in tft display. It allows for a quick overvieuw of the sensor data that is being received over wifi.

The homescreen test [code](../../Demo%20Code/YellowScreen/homescreen/homescreen.ino) uses the touch functionality of the display to toggle between the different sensor data values. In the full [code](./Yellow.ino) however the display toggles every 5 seconds between two sensor values. This design choice was made since the touch funcionality of the tft display got slightly damaged over time. When building the project using new components this shouldn't be a problem however.

## wifi

All sensor data in the project is received over wifi, this can be achieved using the wifi functionality on the esp32 chip ( wroom-eu ). The data of the sensor's is available over the loxone server and is parsed by the homescreen module itself.

```c
// --- Fetch Sensor Data ---
void fetchSensorData() {
  HTTPClient http;
  http.begin(serverURL);

  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();

    // Parse JSON manually
    temperature = payload.substring(payload.indexOf("\"Temperature\":") + 14, payload.indexOf(",", payload.indexOf("\"Temperature\":"))).toFloat();
    humidity = payload.substring(payload.indexOf("\"Humidity\":") + 11, payload.indexOf(",", payload.indexOf("\"Humidity\":"))).toFloat();
    soundLevel = payload.substring(payload.indexOf("\"SoundLevel\":") + 13, payload.indexOf(",", payload.indexOf("\"SoundLevel\":"))).toInt();
    dustConcentration = payload.substring(payload.indexOf("\"DustConcentration\":") + 21, payload.indexOf(",", payload.indexOf("\"DustConcentration\":"))).toFloat();
    ambiTemp = payload.substring(payload.indexOf("\"AmbiTemp\":") + 11, payload.indexOf(",", payload.indexOf("\"AmbiTemp\":"))).toFloat();
    ambiHumidity = payload.substring(payload.indexOf("\"AmbiHumidity\":") + 15, payload.indexOf(",", payload.indexOf("\"AmbiHumidity\":"))).toFloat();
    ambiLight = payload.substring(payload.indexOf("\"AmbiLight\":") + 12, payload.indexOf(",", payload.indexOf("\"AmbiLight\":"))).toInt();
    ambiAudio = payload.substring(payload.indexOf("\"AmbiAudio\":") + 12, payload.indexOf(",", payload.indexOf("\"AmbiAudio\":"))).toInt();
    ambiCO2 = payload.substring(payload.indexOf("\"AmbiCO2\":") + 11, payload.indexOf(",", payload.indexOf("\"AmbiCO2\":"))).toInt();
    ambiVOC = payload.substring(payload.indexOf("\"AmbiVOC\":") + 11, payload.indexOf(",", payload.indexOf("\"AmbiVOC\":"))).toInt();
    ambiBattery = payload.substring(payload.indexOf("\"AmbiBattery\":") + 15).toFloat();
  }

  http.end();
}
```



