#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Display + UI settings
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define GENERAL_FONT_SIZE 2
#define DATA_FONT_SIZE 4

// Wi-Fi credentials
const char* ssid = "xxxxxxxxx";
const char* password = "xxxxxxxxx";
const char* serverURL = "http://192.168.0.223/sensdata";  // Update IP if needed

// Globals
TFT_eSPI tft = TFT_eSPI();  // Uses config from User_Setup.h

// Sensor Data
float temperature = 0.0f;
float humidity = 0.0f;
unsigned int soundLevel = 0;
float dustConcentration = 0.0f;
float ambiTemp = 0.0f, ambiHumidity = 0.0f, ambiBattery = 0.0f;
uint16_t ambiLight = 0, ambiCO2 = 0, ambiVOC = 0;
uint8_t ambiAudio = 0;

// Page Navigation
int currentPage = 0;
const int totalPages = 8;  // 0-2: KY Sensors, 3-7: AmbiMate values

// --- UI Drawing Functions ---
void drawBackground(const char* title) {
  tft.fillScreen(TFT_DARKGREY);
  tft.drawLine(0, 31, SCREEN_WIDTH, 31, TFT_BLACK);
  tft.drawLine(0, 30, SCREEN_WIDTH, 30, TFT_LIGHTGREY);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(GENERAL_FONT_SIZE);
  tft.drawString(title, 31, 6);
  tft.setTextColor(TFT_LIGHTGREY);
  tft.drawString(title, 30, 5);
  tft.fillCircle(300, 15, 6, TFT_GREEN);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(DATA_FONT_SIZE);
}

// KY Sensors
void drawDHT11Page() {
  drawBackground("KY-015_DHT11");
  tft.drawString("Temp: " + String(temperature, 1) + " C", 30, 60);
  tft.drawString("Hum: " + String(humidity, 1) + " %", 30, 120);
}

void drawMAX4466Page() {
  drawBackground("MAX_4466");
  tft.drawString("Sound:", 30, 80);
  tft.drawString(String(soundLevel) + " units", 30, 140);
}

void drawDustPage() {
  drawBackground("PPD42 Dust");
  tft.drawString("Dust:", 30, 80);
  tft.drawString(String(dustConcentration, 1) + " pcs", 30, 140);
}

// AmbiMate pages
void drawAmbiTempPage() {
  drawBackground("AmbiMate Temp");
  tft.drawString(String(ambiTemp, 1) + " C", 30, 100);
}

void drawAmbiHumidityPage() {
  drawBackground("AmbiMate Humidity");
  tft.drawString(String(ambiHumidity, 1) + " %", 30, 100);
}

void drawAmbiLightPage() {
  drawBackground("AmbiMate Light");
  tft.drawString(String(ambiLight) + " Lux", 30, 100);
}

void drawAmbiAudioPage() {
  drawBackground("AmbiMate Audio");
  tft.drawString(String(ambiAudio) + " dB", 30, 100);
}

void drawAmbiCO2VOCBatteryPage() {
  drawBackground("AmbiMate Env");
  tft.setTextSize(GENERAL_FONT_SIZE);
  tft.drawString("CO2: " + String(ambiCO2) + " ppm", 30, 60);
  tft.drawString("VOC: " + String(ambiVOC) + " ppb", 30, 100);
  tft.drawString("Bat: " + String(ambiBattery, 2) + " V", 30, 140);
}

// --- Setup ---
void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.println("Starting...");

  delay(1500);
}

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

// --- Loop ---
unsigned long lastFetchTime = 0;
const unsigned long fetchInterval = 1000;

unsigned long lastPageSwitchTime = 0;
const unsigned long pageSwitchInterval = 7500;

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastFetchTime >= fetchInterval) {
    fetchSensorData();

    switch (currentPage) {
      case 0: drawDHT11Page(); break;
      case 1: drawMAX4466Page(); break;
      case 2: drawDustPage(); break;
      case 3: drawAmbiTempPage(); break;
      case 4: drawAmbiHumidityPage(); break;
      case 5: drawAmbiLightPage(); break;
      case 6: drawAmbiAudioPage(); break;
      case 7: drawAmbiCO2VOCBatteryPage(); break;
    }

    lastFetchTime = currentMillis;
  }

  if (currentMillis - lastPageSwitchTime >= pageSwitchInterval) {
    currentPage = (currentPage + 1) % totalPages;
    lastPageSwitchTime = currentMillis;
  }

  delay(50);
}
