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

// =================================================================================================
// DISPLAY CONFIGURATION
// =================================================================================================

// Creates the OLED display object
// Parameters:
// I2C Address = 0x3C
// Frequency = 500 kHz
// SDA_OLED and SCL_OLED are defined by the Heltec board package
// Resolution = 128x64 pixels
// RST_OLED = display reset pin
static SSD1306Wire display(
  0x3c,
  500000,
  SDA_OLED,
  SCL_OLED,
  GEOMETRY_128_64,
  RST_OLED
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

// =================================================================================================
// ESP-NOW CALLBACK
// =================================================================================================

// Executed automatically whenever a telemetry packet is received
// Updated callback signature required by ESP32 Core 3.x
void onDataRecv(
  const esp_now_recv_info_t *recvInfo,
  const uint8_t *incomingData,
  int len
) {

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
}

// =================================================================================================
// DASHBOARD DRAWING
// =================================================================================================

// Draws the vehicle dashboard on the OLED display
void drawDashboard() {

  // Clear display buffer
  display.clear();

  // Configure text alignment and font
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  // -----------------------------------------------------------------------------------------------
  // SPEED
  // -----------------------------------------------------------------------------------------------

  display.drawString(0, 0, "SPEED:");
  display.drawString(70, 0, (String)data.speedData);
  display.drawString(105, 0, "Km");

  // -----------------------------------------------------------------------------------------------
  // DISTANCE SENSOR
  // -----------------------------------------------------------------------------------------------

  display.drawString(0, 12, "DISTANCE:");
  display.drawString(70, 12, (String)data.obstacleDistanceData);
  display.drawString(105, 12, "mm");

  // -----------------------------------------------------------------------------------------------
  // VIBRATION
  // -----------------------------------------------------------------------------------------------

  display.drawString(0, 24, "VIBRATION:");
  display.drawString(70, 24, (String)data.vibrationData);
  display.drawString(105, 24, "RMS");

  // -----------------------------------------------------------------------------------------------
  // TEMPERATURE
  // -----------------------------------------------------------------------------------------------

  display.drawString(0, 36, "TEMPERAT:");
  display.drawString(70, 36, (String)data.temperatureData);
  display.drawString(105, 36, "C");

  // -----------------------------------------------------------------------------------------------
  // VEHICLE STATUS
  // -----------------------------------------------------------------------------------------------

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
// MAIN LOOP
// =================================================================================================

void loop() {

  // Update display every 200 milliseconds
  if (millis() - lastDisplayUpdate >= 200) {

    lastDisplayUpdate = millis();

    drawDashboard();
  }
}