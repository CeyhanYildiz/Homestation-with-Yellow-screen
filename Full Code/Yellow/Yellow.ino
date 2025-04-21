// Includes
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

// Display + UI settings
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define GENERAL_FONT_SIZE 2
#define DATA_FONT_SIZE 4
#define MAXSENSCOUNT 4

// Wi-Fi credentials
const char* ssid = "telenet-Ster";
const char* password = "0485541209n";
const char* serverURL = "http://192.168.0.223/sensdata";  // Replace with your ESP8266's IP and the endpoint

// Globals
TFT_eSPI tft = TFT_eSPI();  // Uses config from User_Setup.h
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Sensor Data
int sensCount = 0;
String names[MAXSENSCOUNT] = { "sens1", "sens2", "sens3", "sens4" };
float sensValue[MAXSENSCOUNT] = { 0.0f, 0.0f, 0.0f, 0.0f };

// --- UI Drawing Functions ---
void drawBackground() {
  tft.fillScreen(TFT_DARKGREY);
  tft.drawLine(0, 31, SCREEN_WIDTH, 31, TFT_BLACK);
  tft.drawLine(0, 30, SCREEN_WIDTH, 30, TFT_LIGHTGREY);
}

void drawFrame(String name, float *value) {
  drawBackground();

  // Title
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(GENERAL_FONT_SIZE);
  tft.drawString(name, 31, 6);
  tft.setTextColor(TFT_LIGHTGREY);
  tft.drawString(name, 30, 5);

  if (value != nullptr) {
    // Green light for valid data
    tft.fillCircle(300, 15, 6, TFT_GREEN);

    // Draw value
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(DATA_FONT_SIZE);
    tft.drawFloat(*value, 1, 60, 100);  // 1 decimal place
    tft.setTextColor(TFT_LIGHTGREY);
    tft.drawFloat(*value, 1, 59, 99);
  } else {
    // Red light for no data
    tft.fillCircle(300, 15, 6, TFT_RED);

    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(DATA_FONT_SIZE);
    tft.drawString("no data", 60, 100);
    tft.setTextColor(TFT_LIGHTGREY);
    tft.drawString("no data", 59, 99);
  }
}

// --- Setup ---
void setup() {
  // Serial.begin(115200);  // Removed for faster operation

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Init touchscreen SPI
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  // Init screen
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.println("Touch screen to begin...");

  delay(1500);
  drawFrame(names[sensCount], &sensValue[sensCount]);
}

// --- Fetch Sensor Data from ESP8266 ---
void fetchSensorData() {
  HTTPClient http;
  http.begin(serverURL);

  int httpCode = http.GET(); // Send GET request

  if (httpCode > 0) {
    String payload = http.getString();

    // Assuming the payload is in this format: {"sens1": 21.0, "sens2": 15.5, ...}
    // Parse the payload (you can use JSON parsing library for better handling)
    sensValue[0] = payload.substring(payload.indexOf("sens1") + 7, payload.indexOf("sens1") + 11).toFloat();
    sensValue[1] = payload.substring(payload.indexOf("sens2") + 7, payload.indexOf("sens2") + 11).toFloat();
    sensValue[2] = payload.substring(payload.indexOf("sens3") + 7, payload.indexOf("sens3") + 11).toFloat();
    sensValue[3] = payload.substring(payload.indexOf("sens4") + 7, payload.indexOf("sens4") + 11).toFloat();
  }

  http.end(); // Close the connection
}
unsigned long lastFetchTime = 0;  // Time tracker for fetch requests
const unsigned long fetchInterval = 5000;  // 5 seconds interval between fetch requests

void loop() {
  unsigned long currentMillis = millis();  // Get the current time in milliseconds

  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point point = touchscreen.getPoint();

    // Map touch point
    int x = map(point.x, 200, 3700, 1, SCREEN_WIDTH);
    int y = map(point.y, 240, 3800, 1, SCREEN_HEIGHT);

    // Navigate sensor list
    if (y > SCREEN_HEIGHT / 2) {
      --sensCount;
      if (sensCount < 0) sensCount = MAXSENSCOUNT - 1;
    } else {
      ++sensCount;
      if (sensCount == MAXSENSCOUNT) sensCount = 0;
    }

    // Fetch new sensor data only if it's been enough time since the last fetch
    if (currentMillis - lastFetchTime >= fetchInterval) {
      fetchSensorData();
      lastFetchTime = currentMillis;  // Update the last fetch time
    }

    // Draw frame with updated sensor value
    drawFrame(names[sensCount], &sensValue[sensCount]);
  }

  delay(50);  // Small delay to improve touchscreen detection response time
}
