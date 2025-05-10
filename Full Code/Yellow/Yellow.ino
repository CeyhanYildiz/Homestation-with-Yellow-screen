#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Touchscreen pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Display + UI settings
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define GENERAL_FONT_SIZE 2
#define DATA_FONT_SIZE 4

// Wi-Fi credentials
const char* ssid = "telenet-Ster";
const char* password = "0485541209n";
const char* serverURL = "http://192.168.0.223/sensdata";  // Replace with your ESP8266 IP

// Globals
TFT_eSPI tft = TFT_eSPI();  // Uses config from User_Setup.h
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Sensor Data
float temperature = 0.0f;
float humidity = 0.0f;
unsigned int soundLevel = 0;
float ambiTemperature = 0.0f;
float ambiHumidity = 0.0f;

// Page Navigation
int currentPage = 0;  // 0 = DHT11, 1 = Sound, 2 = AmbiMate

// UI Drawing
void drawBackground() {
  tft.fillScreen(TFT_DARKGREY);
  tft.drawLine(0, 31, SCREEN_WIDTH, 31, TFT_BLACK);
  tft.drawLine(0, 30, SCREEN_WIDTH, 30, TFT_LIGHTGREY);
}

void drawDHT11Page() {
  drawBackground();
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(GENERAL_FONT_SIZE);
  tft.drawString("KY-015_DHT11", 31, 6);
  tft.setTextColor(TFT_LIGHTGREY);
  tft.drawString("KY-015_DHT11", 30, 5);
  tft.fillCircle(300, 15, 6, TFT_GREEN);

  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(DATA_FONT_SIZE);
  tft.drawString("Temp: " + String(temperature, 1) + " C", 30, 60);
  tft.drawString("Hum: " + String(humidity, 1) + " %", 30, 120);
}

void drawMAX4466Page() {
  drawBackground();
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(GENERAL_FONT_SIZE);
  tft.drawString("MAX_4466", 31, 6);
  tft.setTextColor(TFT_LIGHTGREY);
  tft.drawString("MAX_4466", 30, 5);
  tft.fillCircle(300, 15, 6, TFT_GREEN);

  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(DATA_FONT_SIZE);
  tft.drawString("Sound:", 30, 80);
  tft.drawString(String(soundLevel) + " units", 30, 140);
}

void drawAmbiMatePage() {
  drawBackground();
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(GENERAL_FONT_SIZE);
  tft.drawString("AmbiMate MS4", 31, 6);
  tft.setTextColor(TFT_LIGHTGREY);
  tft.drawString("AmbiMate MS4", 30, 5);
  tft.fillCircle(300, 15, 6, TFT_GREEN);

  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(DATA_FONT_SIZE);
  tft.drawString("Temp: " + String(ambiTemperature, 1) + " C", 30, 60);
  tft.drawString("Hum: " + String(ambiHumidity, 1) + " %", 30, 120);
}

// Setup
void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.println("Touch screen to begin...");
  delay(1500);
}

// Fetch Sensor Data from HTTP JSON
void fetchSensorData() {
  HTTPClient http;
  http.begin(serverURL);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();

    // DHT11
    int tempStart = payload.indexOf("\"Temperature\":") + 14;
    int tempEnd = payload.indexOf(",", tempStart);
    temperature = payload.substring(tempStart, tempEnd).toFloat();

    int humStart = payload.indexOf("\"Humidity\":") + 11;
    int humEnd = payload.indexOf(",", humStart);
    humidity = payload.substring(humStart, humEnd).toFloat();

    // Sound
    int soundStart = payload.indexOf("\"SoundLevel\":") + 13;
    int soundEnd = payload.indexOf(",", soundStart);
    soundLevel = payload.substring(soundStart, soundEnd).toInt();

    // AmbiMate Temp
    int ambiTempStart = payload.indexOf("\"AmbiMateTemperature\":") + 22;
    int ambiTempEnd = payload.indexOf(",", ambiTempStart);
    ambiTemperature = payload.substring(ambiTempStart, ambiTempEnd).toFloat();

    // AmbiMate Hum
    int ambiHumStart = payload.indexOf("\"AmbiMateHumidity\":") + 20;
    int ambiHumEnd = payload.indexOf("}", ambiHumStart);
    ambiHumidity = payload.substring(ambiHumStart, ambiHumEnd).toFloat();
  }

  http.end();
}

// Loop
unsigned long lastFetchTime = 0;
const unsigned long fetchInterval = 5000;

void loop() {
  unsigned long currentMillis = millis();

  // Periodic fetch
  if (currentMillis - lastFetchTime >= fetchInterval) {
    fetchSensorData();
    lastFetchTime = currentMillis;

    if (currentPage == 0) {
      drawDHT11Page();
    } else if (currentPage == 1) {
      drawMAX4466Page();
    } else if (currentPage == 2) {
      drawAmbiMatePage();
    }
  }

  // Touch page switch
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point point = touchscreen.getPoint();
    int x = map(point.x, 200, 3700, 1, SCREEN_WIDTH);
    int y = map(point.y, 240, 3800, 1, SCREEN_HEIGHT);

    currentPage = (currentPage + 1) % 3;

    if (currentPage == 0) {
      drawDHT11Page();
    } else if (currentPage == 1) {
      drawMAX4466Page();
    } else if (currentPage == 2) {
      drawAmbiMatePage();
    }

    delay(300);  // Debounce
  }

  delay(50);
}
