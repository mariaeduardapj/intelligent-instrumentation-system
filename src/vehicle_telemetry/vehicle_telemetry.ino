#include <SparkFun_ADXL345.h> // ADXL345 vibration library
#include <Adafruit_VL53L0X.h> // VL53L0X distance library
#include <Wire.h> // I2C communication library

#define buzzer 13
#define carbrake 25
#define accelerator 26
#define system_name "Intelligent Vehicle Instrumentation System"
#define system_version "v0.2.1"

String command = ""; // Variable that stores the command typed in the serial monitor
int warning_distance = 150; // Variable that stores the distance warning value

bool vibrationOn = false; // Controls whether vibration data should be printed
ADXL345 adxl = ADXL345(); // Creates an ADXL345 sensor object

bool distancemeterOn = false; // Controls whether distance data should be printed
Adafruit_VL53L0X lox = Adafruit_VL53L0X(); // Creates an VL53L0X sensor object
unsigned long lastBeep = 0; // Timestamp of the last buzzer state change
bool beepState = false; // Current buzzer state 
int beepInterval = 1000; // Time interval between buzzer toggles in milliseconds

unsigned long lastUpdate = 0; // Timestamp of the last main loop update
unsigned long lastDistancePrint = 0; // Timestamp of the last distance measurement print
unsigned long lastVibratPrint = 0; // Timestamp of the last vibration data print

void setup() {
  Serial.begin(115200);
  Serial.println("\n===========================================================");
  Serial.println(system_name " - Version " system_version);
  Serial.println("Developed by https://github.com/mariaeduardapj");
  Serial.println("===========================================================\n");

  Serial.println("COMMAND LIST");
  Serial.println("vibration.on - Print the current engine vibration information;");
  Serial.println("vibration.off - Stop printing engine vibration information.");
  Serial.println("distancemeter.on - Print the current information captured by the laser distance meter;");
  Serial.println("distancemeter.off - Stop printing laser distance meter information;");
  Serial.println("warning.distance x - Define a maximum distance x (in mm) for the buzzer to sound (maximum 1200 mm);");
  Serial.println();

  Wire.begin(14, 27);  // Initializes I2C communication using GPIO 14 (SDA) and GPIO 27 (SCL)
  adxl.powerOn(); // Powers up and initializes the vibration sensor
  adxl.setRangeSetting(2); // Sets the measurement range to ±2g for higher sensitivity (options: ±2g, ±4g, ±8g, ±16g)
  if (!lox.begin()) { // Check if the VL53L0X distance sensor starts correctly
   Serial.println("VL53L0X not found");
   while(1);
  }

  lastUpdate = millis(); // Initialize the update timer with the current system time

  pinMode(carbrake, INPUT_PULLUP); // Configure the brake pedal input with internal pull-up resistor enabled
  pinMode(accelerator, INPUT_PULLUP); // Configure the accelerator pedal input with internal pull-up resistor enabled
}

void loop() {
  int x, y, z; // Variables that store vibration values on each axis
  adxl.readAccel(&x, &y, &z); // Reads the current vibration values from the sensor

  VL53L0X_RangingMeasurementData_t measure; // Create a structure to store the distance measurement data
  lox.rangingTest(&measure, false); // Perform a distance measurement and store the result in 'measure'

  if (Serial.available() > 0){ // If something was read on the serial monitor
    command = Serial.readStringUntil('\n'); // Read until you press enter
    command.trim(); // Remove spaces and line breaks

    if (command == "vibration.on"){
      vibrationOn = true;
    }
    else if (command == "vibration.off"){ 
      vibrationOn = false;
    }

    else if (command == "distancemeter.on"){
      distancemeterOn = true;
    }
    else if (command == "distancemeter.off"){ 
      distancemeterOn = false;
      noTone(buzzer);
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

    else { 
      Serial.println("Invalid command");
    }
  }

  if (vibrationOn) { // Prints vibration on the X, Y and Z axis
    if (millis() - lastVibratPrint >= 3000) { // Print the vibration measured in millimeters every 3 seconds
      lastVibratPrint = millis(); 
      Serial.print("X: "); 
      Serial.print(x);
      Serial.print("   Y: "); 
      Serial.print(y);
      Serial.print("   Z: "); 
    }
  }

  if (distancemeterOn) {
    if (measure.RangeStatus != 4) { // Verify that the measurement is valid
      if (millis() - lastDistancePrint >= 500){ // Print the distance measured in millimeters every half second
        lastDistancePrint = millis();
        Serial.print("Distance: ");
        Serial.print(measure.RangeMilliMeter); 
        Serial.println(" mm");
      }
      if (warning_distance > 0 && measure.RangeMilliMeter <= warning_distance) {  // Check if the measured distance is below the warning threshold
        float ratio = // Calculate the distance ratio relative to the warning limit
          (float)measure.RangeMilliMeter /
          (float)warning_distance;
          if (ratio > 0.75)  // Adjust the buzzer interval according to proximity
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
        noTone(buzzer);
        beepState = false; 
      }
    }
    else {
      noTone(buzzer);
      beepState = false;  
      Serial.println("Out of reach");
    }
  }
}
