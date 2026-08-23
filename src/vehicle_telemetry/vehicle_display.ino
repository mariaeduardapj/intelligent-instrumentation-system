#include <WiFi.h>
#include <esp_now.h>

typedef struct {
  float speed;
  int vehicleStatus;
  int vibrationStatus;
  int temperatureStatus;
  int obstacleDistance;
} TelemetryData;

TelemetryData data;

void onReceive(
  const esp_now_recv_info_t *info,
  const uint8_t *incomingData,
  int len)
{
  memcpy(&data, incomingData, sizeof(data));

  Serial.println("------");

  Serial.print("Speed: ");
  Serial.println(data.speed);

  Serial.print("Vehicle: ");
  Serial.println(data.vehicleStatus);

  Serial.print("Vibration: ");
  Serial.println(data.vibrationStatus);

  Serial.print("Temperature: ");
  Serial.println(data.temperatureStatus);

  Serial.print("Distance: ");
  Serial.println(data.obstacleDistance);
}

void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Error");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  Serial.println("Receiver Ready");
}

void loop() {
}