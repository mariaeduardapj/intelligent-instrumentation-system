// =================================================================================================
// LIBRARIES
// =================================================================================================

// OLED display communication via I2C
#include <Wire.h>

// Heltec SSD1306 OLED display library
#include "HT_SSD1306Wire.h"

// ESP-NOW communication libraries
#include <WiFi.h>
#include <esp_now.h>

// GC9A01 Round display library
#include <Arduino_GFX_Library.h>


// =================================================================================================
// OLED DISPLAY CONFIGURATION
// =================================================================================================
static SSD1306Wire display(
  0x3c,
  500000,
  SDA_OLED,
  SCL_OLED,
  GEOMETRY_128_64,
  RST_OLED
);


// =================================================================================================
// ROUND DISPLAY CONFIGURATION
// =================================================================================================

// DISPLAY PINS
#define GFX_BL 22
#define SCK_PIN 18
#define MOSI_PIN 23
#define MISO_PIN -1
#define CS_PIN 5
#define DC_PIN 19
#define RST_PIN 21

const int CX = 120;
const int CY = 120;


Arduino_DataBus *bus = new Arduino_ESP32SPI(
  DC_PIN,
  CS_PIN,
  SCK_PIN,
  MOSI_PIN,
  MISO_PIN
);
Arduino_GFX *gfx = new Arduino_GC9A01(
  bus,
  RST_PIN,
  0,
  true
);


// =================================================================================================
// TELEMETRY DATA STRUCTURE
// =================================================================================================

// Structure received from the vehicle ESP through ESP-NOW
typedef struct {
  float speedData;
  int vehicleStatus;

  int vibrationStatus;
  float vibrationData;

  int temperatureStatus;
  float temperatureData;

  int obstacleDistanceData;
} TelemetryData;

// Stores the most recent telemetry packet received
TelemetryData data;

// Controls display refresh rate
unsigned long lastDisplayUpdate = 0;

// Controls refresh rate on the round display for each variable
static int lastSpeed = -1;
static int lastTemperatureStatus = -1;
static int lastVibrationStatus = -1;
static int lastVehicleStatus = -1;

// Variables to delete the old pointer
int lastTipX, lastTipY;
int lastLeftX, lastLeftY;
int lastRightX, lastRightY;
bool firstPointer = true;


// =================================================================================================
// ESP-NOW CALLBACK
// =================================================================================================

// Executed automatically whenever a telemetry packet is received
// Updated callback signature required by ESP32 Core 3.x
void onDataRecv(
  const esp_now_recv_info_t *recvInfo,
  const uint8_t *incomingData,
  int len) {

  // Copy received bytes into the telemetry structure
  memcpy(&data, incomingData, sizeof(data));

  Serial.println("------");

  // Sender MAC address can be accessed if needed
  // const uint8_t *mac_addr = recvInfo->src_addr;

  Serial.print("Speed: ");
  Serial.println(data.speedData);

  Serial.print("Vehicle: ");
  Serial.println(data.vehicleStatus);

  Serial.print("Vibration: ");
  Serial.println(data.vibrationStatus);

  Serial.print("Temperature: ");
  Serial.println(data.temperatureStatus);

  Serial.print("Distance: ");
  Serial.println(data.obstacleDistanceData);
}



// =================================================================================================
// DASHBOARD OLED DRAWING
// =================================================================================================
void drawDashboardOLED() {

  // Clear display buffer
  display.clear();
  // Configure text alignment and font
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  display.drawString(0, 0, "SPEED:");
  display.drawString(70, 0, (String)data.speedData);
  display.drawString(105, 0, "Km");

  display.drawString(0, 12, "DISTANCE:");
  display.drawString(70, 12, (String)data.obstacleDistanceData);
  display.drawString(105, 12, "mm");

  display.drawString(0, 24, "VIBRATION:");
  display.drawString(70, 24, (String)data.vibrationData);
  display.drawString(105, 24, "RMS");

  display.drawString(0, 36, "TEMPERAT:");
  display.drawString(70, 36, (String)data.temperatureData);
  display.drawString(105, 36, "°C");

  display.drawString(8, 51, "VEHICLE STATUS:");
  if (data.vehicleStatus == 0) {
    display.drawString(100, 51, "OK");
  }
  else if (data.vehicleStatus == 1) {
    display.drawString(100, 51, "ATT");
  }
  else {
    display.drawString(100, 51, "CRI");
  }

  // Send buffer contents to the physical OLED display
  display.display();
}


// =================================================================================================
// DASHBOARD ROUND DRAWING
// =================================================================================================
void drawStaticDashboardRound() { // Static drawings
    // Draw rim
    gfx->drawCircle(CX, CY, 100, RGB565_WHITE);
    gfx->drawCircle(CX, CY, 101, RGB565_WHITE);
    gfx->drawCircle(CX, CY, 102, RGB565_WHITE);

    // Scale markings
    for(int i = 0; i <= 200; i += 20) {
      float angle = (-135 + (270.0 * i / 200));
      float rad = angle * PI / 180.0;
      int x1 = CX + cos(rad) * 90;
      int y1 = CY + sin(rad) * 90;
      int x2 = CX + cos(rad) * 100;
      int y2 = CY + sin(rad) * 100;
      gfx->drawLine(x1,y1,x2,y2,RGB565_WHITE);

      int xt = CX + cos(rad) * 75;
      int yt = CY + sin(rad) * 75;
      gfx->setTextColor(RGB565_WHITE);
      gfx->setTextSize(1);
      String value = String(i);

      gfx->setCursor(xt - (value.length() * 3),yt - 4);

      gfx->print(value);
    }
}


void drawDashboardRound() { // Update variables only when they change
  if((int)data.speedData != lastSpeed) {  
    lastSpeed = (int)data.speedData;

    // Erase old pointer
    if(!firstPointer) {
      gfx->fillTriangle(lastTipX, lastTipY,lastLeftX, lastLeftY,lastRightX, lastRightY,RGB565_BLACK);
    }

    // Calculates the triangular pointer position based on the current speed
    float angle = -135 + (270.0 * data.speedData / 200.0);
    float rad = angle * PI / 180.0;

    // Center base of the pointer
    int baseX = CX + cos(rad) * 50;
    int baseY = CY + sin(rad) * 50;

    // Tip of the triangle
    int tipX = CX + cos(rad) * 65;
    int tipY = CY + sin(rad) * 65;

    // Triangle width
    float perp = rad + PI/2;
    int leftX  = baseX + cos(perp) * 5;
    int leftY  = baseY + sin(perp) * 5;
    int rightX = baseX - cos(perp) * 5;
    int rightY = baseY - sin(perp) * 5;

    // Draw Pointer
    gfx->fillTriangle(tipX, tipY, leftX, leftY, rightX, rightY, RGB565_GREEN);

    // Update variables to clear pointer
    lastTipX = tipX;
    lastTipY = tipY;
    lastLeftX = leftX;
    lastLeftY = leftY;
    lastRightX = rightX;
    lastRightY = rightY;
    firstPointer = false;

    // Clears the speed region so that the pointer and the old speed do not overlap.
    gfx->fillRect(75,90,93,65,RGB565_BLACK);

    // Current speed data
    gfx->setTextColor(RGB565_GREEN);
    gfx->setTextSize(5);
    if (data.speedData < 10) {
      gfx->setCursor(108,95);
    } else if (data.speedData < 100) {
      gfx->setCursor(95,95);
    } else {
      gfx->setCursor(78,95);
    }
    String spd = String((int)data.speedData);
    gfx->print(spd);
    gfx->setTextSize(2);
    gfx->setCursor(99,135);
    gfx->print("km/h");
  }

  if((int)data.vehicleStatus != lastVehicleStatus) { // Creates a border with a color according to the car's condition.
    lastVehicleStatus = (int)data.vehicleStatus;
    
    uint16_t statusColor;

    if(data.vehicleStatus == 0) {
    statusColor = RGB565_GREEN;
    } else if(data.vehicleStatus == 1) {
    statusColor = RGB565_YELLOW;
    } else {
    statusColor = RGB565_RED;
    }

    gfx->drawCircle(CX,CY,110,statusColor);
  }

  if((int)data.temperatureStatus != lastTemperatureStatus) { // Create an icon with a color that matches the temperature state
    lastTemperatureStatus = (int)data.temperatureStatus;

    gfx->fillRect(40,90,30,35,RGB565_BLACK);

    if (data.temperatureStatus == 0) {
      gfx->fillCircle(50,115,4,RGB565_GREY);
      gfx->fillRect(50,95,2,25,RGB565_GREY);
      gfx->setTextSize(1);
      gfx->setTextColor(RGB565_GREY);
      gfx->setCursor(55,100);
      gfx->print("°C");
    } else if (data.temperatureStatus == 1) {
      gfx->fillCircle(50,115,4,RGB565_YELLOW);
      gfx->fillRect(50,95,2,25,RGB565_YELLOW);
      gfx->setTextSize(1);
      gfx->setTextColor(RGB565_YELLOW);
      gfx->setCursor(55,100);
      gfx->print("°C");
    } else {
      gfx->fillCircle(50,115,4,RGB565_RED);
      gfx->fillRect(50,95,2,25,RGB565_RED);
      gfx->setTextSize(1);
      gfx->setTextColor(RGB565_RED);
      gfx->setCursor(55,100);
      gfx->print("°C");
    }
  }

  if((int)data.vibrationStatus != lastVibrationStatus) {  // Create an icon with a color that matches the vibration state
    lastVibrationStatus = (int)data.vibrationStatus;

    gfx->fillRect(35,130,40,15,RGB565_BLACK);

    if (data.vibrationStatus == 0) {
      gfx->drawCircle(55, 135, 2, RGB565_GREY);
      gfx->drawCircle(55, 135, 6, RGB565_GREY);
      gfx->drawCircle(55, 135, 10, RGB565_GREY);
      gfx->setTextSize(1);
      gfx->setTextColor(RGB565_GREY);
      gfx->setCursor(68,124);
      gfx->print("R");
      gfx->setCursor(68,132);
      gfx->print("M");
      gfx->setCursor(68,140);
      gfx->print("S");
    } else if (data.vibrationStatus == 1) {
      gfx->drawCircle(55, 135, 2, RGB565_YELLOW);
      gfx->drawCircle(55, 135, 6, RGB565_YELLOW);
      gfx->drawCircle(55, 135, 10, RGB565_YELLOW);
      gfx->setTextSize(1);
      gfx->setTextColor(RGB565_YELLOW);
      gfx->setCursor(68,124);
      gfx->print("R");
      gfx->setCursor(68,132);
      gfx->print("M");
      gfx->setCursor(68,140);
      gfx->print("S");
    } else {
      gfx->drawCircle(55, 135, 2, RGB565_RED);
      gfx->drawCircle(55, 135, 6, RGB565_RED);
      gfx->drawCircle(55, 135, 10, RGB565_RED);
      gfx->setTextSize(1);
      gfx->setTextColor(RGB565_RED);
      gfx->setCursor(68,124);
      gfx->print("R");
      gfx->setCursor(68,132);
      gfx->print("M");
      gfx->setCursor(68,140);
      gfx->print("S");
    }
  }
}


// =================================================================================================
// INITIALIZATION
// =================================================================================================
void setup() {

  // Initialize serial communication
  Serial.begin(115200);

  // Configure ESP32 as Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Error");
    return;
  }

  // Register callback for incoming packets
  esp_now_register_recv_cb(onDataRecv);

  Serial.println("Receiver Ready");

  // Initialize OLED display
  display.init();
  // Select default font
  display.setFont(ArialMT_Plain_10);

  // Initialize Round display
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
  if (!gfx->begin()) {
  Serial.println("Display failed!");
  }
  gfx->fillScreen(RGB565_BLACK);
  drawStaticDashboardRound();
}

// =================================================================================================
// MAIN LOOP
// =================================================================================================
void loop() {

  // Update display every 200 milliseconds
  if (millis() - lastDisplayUpdate >= 200) {

    lastDisplayUpdate = millis();

    drawDashboardOLED();

    drawDashboardRound();
  }
}