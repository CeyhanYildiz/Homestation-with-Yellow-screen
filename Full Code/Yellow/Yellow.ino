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
const char* ssid = "telenet-Ster";
const char* password = "0485541209n";
const char* serverURL = "http://192.168.0.223/sensdata";  // Update if ESP8266 IP is different

// Globals
TFT_eSPI tft = TFT_eSPI();  // Uses config from User_Setup.h

// Sensor Data
float temperature = 0.0f;
float humidity = 0.0f;
unsigned int soundLevel = 0;
float dustConcentration = 0.0f;

// Page Navigation
int currentPage = 0;  // 0 = DHT11, 1 = Sound, 2 = Dust

// --- UI Drawing Functions ---
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

void drawDustPage() {
  drawBackground();
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(GENERAL_FONT_SIZE);
  tft.drawString("PPD42 Dust", 31, 6);
  tft.setTextColor(TFT_LIGHTGREY);
  tft.drawString("PPD42 Dust", 30, 5);
  tft.fillCircle(300, 15, 6, TFT_GREEN);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(DATA_FONT_SIZE);
  tft.drawString("Dust:", 30, 80);
  tft.drawString(String(dustConcentration, 1) + " pcs", 30, 140);
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

  int httpCode = http.GET(); // Send GET request

  if (httpCode > 0) {
    String payload = http.getString();

    // Temperature
    int tempStart = payload.indexOf("\"Temperature\":") + 14;
    int tempEnd = payload.indexOf(",", tempStart);
    temperature = payload.substring(tempStart, tempEnd).toFloat();

    // Humidity
    int humStart = payload.indexOf("\"Humidity\":") + 11;
    int humEnd = payload.indexOf(",", humStart);
    humidity = payload.substring(humStart, humEnd).toFloat();

    // Sound Level
    int soundStart = payload.indexOf("\"SoundLevel\":") + 13;
    int soundEnd = payload.indexOf(",", soundStart);
    if (soundEnd == -1) soundEnd = payload.indexOf("}", soundStart);  // fallback
    soundLevel = payload.substring(soundStart, soundEnd).toInt();

    // Dust Concentration
    int dustStart = payload.indexOf("\"DustConcentration\":") + 21;
    int dustEnd = payload.indexOf("}", dustStart);
    dustConcentration = payload.substring(dustStart, dustEnd).toFloat();
  }

  http.end();
}

// --- Loop ---
unsigned long lastFetchTime = 0;
const unsigned long fetchInterval = 1000;  // every 1s

unsigned long lastPageSwitchTime = 0;
const unsigned long pageSwitchInterval = 7500;

void loop() {
  unsigned long currentMillis = millis();

  // Fetch sensor data and update current page
  if (currentMillis - lastFetchTime >= fetchInterval) {
    fetchSensorData();

    // Refresh current page
    switch (currentPage) {
      case 0: drawDHT11Page(); break;
      case 1: drawMAX4466Page(); break;
      case 2: drawDustPage(); break;
    }

    lastFetchTime = currentMillis;
  }

  // Page switching logic
  if (currentMillis - lastPageSwitchTime >= pageSwitchInterval) {
    currentPage = (currentPage + 1) % 3;
    lastPageSwitchTime = currentMillis;
  }

  delay(50);
}
