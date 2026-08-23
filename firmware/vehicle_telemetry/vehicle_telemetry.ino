// =================================================================================================
// SYSTEM CONFIGURATIONS
// =================================================================================================

#include <SparkFun_ADXL345.h> // ADXL345 vibration library
#include <Adafruit_VL53L0X.h> // VL53L0X distance library
#include <Wire.h> // I2C communication library
#include <DHT.h> // DHT11 temperature library
#include <esp_now.h> // ESP comunication library
#include <WiFi.h> // WIFI library for ESP comunication
#include <WiFiUdp.h>


#define buzzer 13
#define carbrake 25
#define accelerator 26
#define temperature 33
#define DHTTYPE DHT11
#define system_name "Intelligent Vehicle Instrumentation System"
#define system_version "v1.0.0"
#define STATUS_NORMAL 0
#define STATUS_ATTENTION 1
#define STATUS_CRITICAL 2

String command = ""; // Variable that stores the command typed in the serial monitor
int warning_distance = 150; // Variable that stores the distance warning value

// =================================================================================================
// DEBUG FLAGS
// =================================================================================================

bool debugVibration = false; 
bool debugDistance = false; 
bool debugSpeed = false;
bool debugTemperature = false;

// =================================================================================================
// VIBRATION SYSTEM
// =================================================================================================

ADXL345 adxl = ADXL345(); // Creates an ADXL345 sensor object

int xAnt = 0; // Previous acceleration reading on the X, Y and Z axis
int yAnt = 0;
int zAnt = 0;

float vibrationRMS = 0;
int vibrationStatus = 0;

int attentionLimit = 3;
int criticalLimit = 10;

long sumSquares = 0; // Accumulates the squared vibration variations for RMS calculation
int sampleCount = 0; // Counts the number of vibration samples collected for the current RMS calculation

// =================================================================================================
// DISTANCE SYSTEM
// =================================================================================================

Adafruit_VL53L0X lox = Adafruit_VL53L0X(); // Creates an VL53L0X sensor object
int obstacleDistance = 0;
bool parkingAlertActive = false;
unsigned long lastBeep = 0; // Timestamp of the last buzzer state change
bool beepState = false; // Current buzzer state 
int beepInterval = 1000; // Time interval between buzzer toggles in milliseconds

// =================================================================================================
// SPEEDOMETER SYSTEM
// =================================================================================================

float speed = 0.0f; // Speedometer state and vehicle dynamics parameters used for speed simulation
const float acceleration = 20.0f;   
const float cbrake = 40.0f;         
const float friction = 5.0f;         
const float max_speed = 200.0f;

int gear = 0; // Current selected gear
int engineFreq = 0; // Engine sound frequency used by the buzzer
int vibration = 0; // Current vibration intensity level

// =================================================================================================
// TEMPERATURE SYSTEM
// =================================================================================================
DHT dht(temperature, DHTTYPE);

float motorTemperature = 0;
int temperatureStatus = 0;

// =================================================================================================
// TIMERS
// =================================================================================================
unsigned long lastUpdate = 0; 
unsigned long lastSpeedPrint = 0; 
unsigned long lastDistancePrint = 0;
unsigned long lastTemperaturePrint = 0;
unsigned long lastVibrationSample = 0;
unsigned long lastTelemetrySend = 0;

// =================================================================================================
// VEHICLE STATUS
// =================================================================================================
int vehicleStatus = STATUS_NORMAL;

// =================================================================================================
// TELEMETRY SYSTEM
// =================================================================================================

// Structure used to package all vehicle telemetry data before transmission
typedef struct {
  float speedData;           // Current vehicle speed (km/h)
  int vehicleStatus;         // Overall vehicle status (NORMAL, ATTENTION or CRITICAL)
  int vibrationStatus;       // Current vibration alarm status
  float vibrationData;       // Measured vibration RMS value
  int temperatureStatus;     // Current temperature alarm status
  float temperatureData;     // Measured engine temperature (°C)
  int obstacleDistanceData;  // Measured obstacle distance (mm)
} TelemetryData;

TelemetryData telemetry; // Structure instance used to store and send telemetry packets

// MAC address of the dashboard ESP32 receiver
uint8_t receiverMAC[] = {
  0x4C, 0xEB, 0xD6, 0x7B, 0xD5, 0xA0
};



// Callback function automatically executed after each ESP-NOW transmission
void OnDataSent(
  const wifi_tx_info_t *info,
  esp_now_send_status_t status)
{
  Serial.print("Send Status: ");

  if(status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Success");
  } else {
    Serial.println("Fail");
  }
}

// =================================================================================================
// WiFi Setup
// =================================================================================================
// Wi-Fi credentials
const char* ssid = ""; // Network name 
const char* password = ""; // Network password 

// Python receiver IP and port
const char* receiverIP = "";  // IP running Python software
const int receiverPort = 5005;

WiFiUDP udp;


void setup() {
  // =================================================================================================
  // INITIAL SETTINGS - SERIAL, HEADER AND TIMER
  // =================================================================================================
  Serial.begin(115200);
  Serial.println("\n===========================================================");
  Serial.println(system_name " - Version " system_version);
  Serial.println("Developed by https://github.com/mariaeduardapj");
  Serial.println("===========================================================\n");

  Serial.println("COMMAND LIST");
  Serial.println("debug.vibration.on - Print the current engine vibration information;");
  Serial.println("debug.vibration.off - Stop printing engine vibration information.");
  Serial.println("debug.distance.on - Print the current information captured by the laser distance meter;");
  Serial.println("debug.distance.off - Stop printing laser distance meter information;");
  Serial.println("debug.speed.on - Print the current speed;");
  Serial.println("debug.speed.off - Stop printing the current speed;");
  Serial.println("debug.temperature.on - Print the current engine temperature;");
  Serial.println("debug.temperature.off - Stop print the current engine temperature.");
  Serial.println("warning.distance x - Define a maximum distance x (in mm) for the buzzer to sound (maximum 1200 mm);");
  Serial.println();

  lastUpdate = millis(); // Initialize the update timer with the current system time

  // =================================================================================================
  // INITIAL SETTINGS - SENSORS
  // =================================================================================================
  
  // VIBRATION SETTINGS
  Wire.begin(14, 27);  // Initializes I2C communication using GPIO 14 (SDA) and GPIO 27 (SCL)
  adxl.powerOn(); // Powers up and initializes the vibration sensor
  adxl.setRangeSetting(2); // Sets the measurement range to ±2g for higher sensitivity (options: ±2g, ±4g, ±8g, ±16g)
  if (!lox.begin()) { // Check if the VL53L0X distance sensor starts correctly
   Serial.println("VL53L0X not found");
   while(1);
  }
  int x0, y0, z0;
  adxl.readAccel(&x0, &y0, &z0);
  xAnt = x0;
  yAnt = y0;
  zAnt = z0;

  // SPEED BUTTON SETTINGS
  pinMode(carbrake, INPUT_PULLUP); // Configure the brake pedal input with internal pull-up resistor enabled
  pinMode(accelerator, INPUT_PULLUP); // Configure the accelerator pedal input with internal pull-up resistor enabled

  // TEMPERATURE SENSOR SETTING
  dht.begin();

  // =================================================================================================
  // INITIAL SETTINGS - TELEMETRY
  // =================================================================================================

  // Configure ESP32 Wi-Fi in Station mode (required by ESP-NOW)
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW communication protocol
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Error");
    return;
  }

  // Create and configure the receiver device information
  esp_now_peer_info_t peerInfo = {};

  // Copy the receiver MAC address into the peer configuration structure
  memcpy(peerInfo.peer_addr, receiverMAC, 6);

  // Use the current Wi-Fi channel
  peerInfo.channel = 0;

  // Disable encryption for simplicity
  peerInfo.encrypt = false;

  // Register the receiver as a valid ESP-NOW peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
  }

  // Register callback function to monitor transmission results
  esp_now_register_send_cb(OnDataSent);

  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected!");

  udp.begin(receiverPort);
}


void handleCommands() {
  if (Serial.available() > 0){ // If something was read on the serial monitor
    command = Serial.readStringUntil('\n'); // Read until you press enter
    command.trim(); // Remove spaces and line breaks

    if (command == "debug.vibration.on"){
      debugVibration = true;
    }
    else if (command == "debug.vibration.off"){ 
      debugVibration = false;
    }

    else if (command == "debug.distance.on"){
      debugDistance = true;
    }
    else if (command == "debug.distance.off"){ 
      debugDistance = false;
    }
    else if (command.startsWith("warning.distance")) { // Check if the received command sets the warning distance
    String value = command.substring(17); // Extract the distance value from the command string
    warning_distance = value.toInt(); // Convert the received value to an integer
      if (warning_distance > 0 && warning_distance <= 1200) { // Validate that the distance is within the allowed range
        Serial.print("Warning distance set to ");
        Serial.print(warning_distance);
        Serial.println(" mm");
      }
      else {
        warning_distance = 150; // Restore the default warning distance if the value is invalid
        Serial.println("Invalid value.");
      }
    }

    else if (command == "debug.speed.on") {
      debugSpeed = true;
    }
    else if (command == "debug.speed.off"){
      debugSpeed = false;
    }

    else if (command == "debug.temperature.on") {
      debugTemperature = true;
    }
    else if (command == "debug.temperature.off"){
      debugTemperature = false;

    }

    else { 
      Serial.println("Invalid command");
    }
  }
}


void updateVibration() {
  if (millis() - lastVibrationSample >= 5) { // Collect one vibration sample every 5 milliseconds
    lastVibrationSample = millis(); // Update the timestamp of the last sample
    int x, y, z;
    adxl.readAccel(&x, &y, &z); // Read the current acceleration values from the sensor

    int dx = x - xAnt; // Calculate the variation on each axis to remove the constant gravity component
    int dy = y - yAnt;
    int dz = z - zAnt;

    sumSquares +=
      (long)dx*dx +
      (long)dy*dy +
      (long)dz*dz; // Accumulate squared variations for RMS computation

    xAnt = x; // Store the current readings for the next variation calculation
    yAnt = y;
    zAnt = z;

    sampleCount++;  // Increment the number of collected samples
  }

  if(sampleCount >= 100) { // Calculate vibration RMS after collecting 100 samples
    vibrationRMS =
      sqrt(sumSquares / (100.0 * 3.0)); // Compute the RMS vibration index
    sumSquares = 0; // Reset the accumulator for the next measurement cycle
    sampleCount = 0; // Reset the sample counter

    if(vibrationRMS < attentionLimit) {
      vibrationStatus = STATUS_NORMAL;
    } else if(vibrationRMS < criticalLimit) {
      vibrationStatus = STATUS_ATTENTION;
    } else {
      vibrationStatus = STATUS_CRITICAL;
    }

    if(debugVibration){
      Serial.print("Vibration: ");
      Serial.print(vibrationRMS);
      Serial.print(" RMS ");
      Serial.println(vibrationStatus);
    }
  }
}


void updateDistance() {
  VL53L0X_RangingMeasurementData_t measure; // Create a structure to store the distance measurement data
  lox.rangingTest(&measure, false); // Perform a distance measurement and store the result in 'measure'
  
  if (measure.RangeStatus != 4) { // Verify that the measurement is valid
    if (millis() - lastDistancePrint >= 500){ // Print the distance measured in millimeters every half second
    obstacleDistance = measure.RangeMilliMeter;
    lastDistancePrint = millis();
    if(debugDistance){
      Serial.print("Distance: ");
      Serial.print(obstacleDistance);
      Serial.println(" mm");
    }
    }
    if (warning_distance > 0 && measure.RangeMilliMeter <= warning_distance) { // Check if the measured distance is below the warning threshold
      parkingAlertActive = true;
      float ratio = // Calculate the distance ratio relative to the warning limit
        (float)measure.RangeMilliMeter /
        (float)warning_distance;
        if (ratio > 0.75) // Adjust the buzzer interval according to proximity
            beepInterval = 1000;
        else if (ratio > 0.50)
            beepInterval = 500;
        else if (ratio > 0.25)
            beepInterval = 250;
        else
            beepInterval = 100;
        
        unsigned long now = millis(); // Get the current system time
        if (now - lastBeep >= beepInterval){ // Toggle the buzzer when the interval expires
            lastBeep = now; // Update the last toggle timestamp
            if (beepState){ // Turn the buzzer off if it is currently on
              noTone(buzzer);
              beepState = false;
            } else {
              tone(buzzer, 1800);
              beepState = true;
            }
        }
    } else {
      parkingAlertActive = false;
      beepState = false;
    }
  }
  else {
    noTone(buzzer);
    beepState = false;

    if (debugDistance) {
      Serial.println("Out of reach");
    }
  }

  if (!parkingAlertActive) { // Generate engine sound while the parking alert is inactive
    if (speed > 0) {
        tone(buzzer, engineFreq + vibration);
    } else {
        noTone(buzzer);
    }
  }
}


void updateSpeed() {
  // Calculate elapsed time since the last update
  unsigned long now = millis();
  float deltaTime = (now - lastUpdate) / 1000.0f;
  lastUpdate = now;

  // Read brake and accelerator pedal states
  bool stateBrake = digitalRead(carbrake);
  bool stateAccelerator = digitalRead(accelerator);
  
  if (stateBrake == LOW) { // Apply braking force
    speed -= cbrake * deltaTime;
  }
  if (stateAccelerator == LOW) {  // Apply acceleration force
    speed += acceleration * deltaTime;
  }
  if (stateBrake == HIGH && stateAccelerator == HIGH) { // Apply natural deceleration when no pedal is pressed
    speed -= friction * deltaTime;
  }

  if (speed < 0.0f){  // Keep speed within valid limits
    speed = 0.0f;
  }
  if (speed > max_speed) {
    speed = max_speed;
  }
  if(debugSpeed) {
    if (millis() - lastSpeedPrint >= 500) {  // Print the current speed every 500 ms
      lastSpeedPrint = millis();
      Serial.print("Speed: ");
      Serial.print(speed);
      Serial.println(" km/h");
    }
  }
  speed = constrain(speed, 0.0f, max_speed); // Ensure speed remains within the configured range
  if (speed < 20) // Determine the current gear based on vehicle speed
      gear = 1;
  else if (speed < 50)
      gear = 2;
  else if (speed < 90)
      gear = 3;
  else if (speed < 130)
      gear = 4;
  else
      gear = 5;
  switch (gear) // Calculate engine sound frequency according to speed and gear
  {
    case 1: engineFreq = map(speed, 0, 20, 120, 450); break;
    case 2: engineFreq = map(speed, 20, 50, 250, 650); break;
    case 3: engineFreq = map(speed, 50, 90, 450, 900); break;
    case 4: engineFreq = map(speed, 90, 130, 700, 1300); break;
    case 5: engineFreq = map(speed, 130, 200, 1000, 1800); break;
  }
  vibration += 15; // Add a small frequency variation to simulate engine vibration
  if (vibration > 100) vibration = 0;

  if (!parkingAlertActive) { // Update the buzzer with the current engine sound
    if (speed > 0) {
        tone(buzzer, engineFreq + vibration);
    }
    else {
    }
  }
  if(speed < 50) {
      attentionLimit = 3; // Low-speed vibration threshold
      criticalLimit = 10; // Low-speed critical vibration threshold
  } else if(speed < 100) {
      attentionLimit = 6; // Medium-speed vibration threshold
      criticalLimit = 20; // Medium-speed critical vibration threshold
  } else {
      attentionLimit = 12; // High-speed vibration threshold
      criticalLimit = 40; // High-speed critical vibration threshold
  }
}


void updateTemperature() {
  // Read temperature as Celsius
  float t = dht.readTemperature();

  // Check if any reads failed and exit early 
  if (isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  motorTemperature = t;
  if (millis() - lastTemperaturePrint >= 3000){ // Update motor temperature every 3 seconds
    lastTemperaturePrint = millis();
    if(t < 80){
      temperatureStatus = STATUS_NORMAL;
      } else if(t < 100) {
          temperatureStatus = STATUS_ATTENTION;
      } else { 
        temperatureStatus = STATUS_CRITICAL; 
      }
    if(debugTemperature) {
      Serial.print(F("Temperature: "));
      Serial.print(motorTemperature);
      Serial.print(F("°C "));
      Serial.println(temperatureStatus);
    }
        
  }
}


void updateVehicleStatus() {
  vehicleStatus = STATUS_NORMAL;

  if(vibrationStatus == STATUS_ATTENTION ||
    temperatureStatus == STATUS_ATTENTION)
  {
      vehicleStatus = STATUS_ATTENTION;
  }

  if(vibrationStatus == STATUS_CRITICAL ||
    temperatureStatus == STATUS_CRITICAL)
  {
      vehicleStatus = STATUS_CRITICAL;
  }
}


void sendTelemetry() {
    // Send binary via ESP-NOW (keep existing)
    telemetry.speedData = speed;
    telemetry.vehicleStatus = vehicleStatus;
    telemetry.vibrationStatus = vibrationStatus;
    telemetry.vibrationData = vibrationRMS;
    telemetry.temperatureStatus = temperatureStatus;
    telemetry.temperatureData = motorTemperature;
    telemetry.obstacleDistanceData = obstacleDistance;
    
    esp_now_send(receiverMAC, (uint8_t*)&telemetry, sizeof(telemetry));
    
    String json = "{";
    json += "\"speed\":" + String(speed) + ",";
    json += "\"vehicleStatus\":" + String(vehicleStatus) + ",";
    json += "\"vibrationStatus\":" + String(vibrationStatus) + ",";
    json += "\"vibrationData\":" + String(vibrationRMS) + ",";
    json += "\"temperatureStatus\":" + String(temperatureStatus) + ",";
    json += "\"temperatureData\":" + String(motorTemperature) + ",";
    json += "\"distance\":" + String(obstacleDistance);
    json += "}";
    
    // Envia via UDP
    udp.beginPacket(receiverIP, receiverPort);
    udp.print(json);
    udp.endPacket();
}


void loop() {
 
  handleCommands();

  updateVibration();
  updateDistance();
  updateSpeed();
  updateTemperature();
 
  updateVehicleStatus();

  // Send telemetry data every 100 milliseconds
  if(millis() - lastTelemetrySend >= 100) {
    lastTelemetrySend = millis();
    sendTelemetry();
  }
}






