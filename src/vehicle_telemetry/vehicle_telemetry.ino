#include <SparkFun_ADXL345.h> // ADXL345 vibration library
#include <Adafruit_VL53L0X.h>
#include <Wire.h> // I2C communication library

#define buzzer 13
#define system_name "Intelligent Vehicle Instrumentation System"
#define system_version "v0.1.0"

String command = ""; // Variable that stores the command typed in the serial monitor.

bool vibrationOn = false; // Controls whether vibration data should be printed
ADXL345 adxl = ADXL345(); // Creates an ADXL345 sensor object

void setup() {
  Serial.begin(115200);
  Serial.println("\n===========================================================");
  Serial.println(system_name " - Version " system_version);
  Serial.println("Developed by https://github.com/mariaeduardapj");
  Serial.println("===========================================================\n");

  Serial.println("COMMAND LIST");
  Serial.println("buzzer.on - Turn on the buzzer;");
  Serial.println("buzzer.off - Turn off the buzzer;");
  Serial.println("vibration.on - Print the current engine vibration information;");
  Serial.println("vibration.off - Stop printing engine vibration information.");

  Wire.begin(14, 27);  // Initializes I2C communication using GPIO 14 (SDA) and GPIO 27 (SCL)
  adxl.powerOn(); // Powers up and initializes the vibration sensor
  adxl.setRangeSetting(2); // Sets the measurement range to ±2g for higher sensitivity (options: ±2g, ±4g, ±8g, ±16g)
}

void loop() {
  int x, y, z; // Variables that store vibration values on each axis
  adxl.readAccel(&x, &y, &z); // Reads the current vibration values from the sensor

  if (Serial.available() > 0){ // If something was read on the serial monitor
    command = Serial.readStringUntil('\n'); // Read until you press enter
    command.trim(); // Remove spaces and line breaks

    if (command == "buzzer.on"){ 
      tone(buzzer, 2000);
    }   
    else if (command  == "buzzer.off"){ 
      noTone(buzzer);
    }

    else if (command == "vibration.on"){
      vibrationOn = true;
    }
    else if (command == "vibration.off"){ 
      vibrationOn = false;
    }

    else { 
      Serial.println("Invalid command");
    }
  }

  if (vibrationOn) { // Prints vibration on the X, Y and Z axis
    Serial.print("X: "); 
    Serial.print(x);
    Serial.print("   Y: "); 
    Serial.print(y);
    Serial.print("   Z: "); 
    Serial.println(z);

    delay(3000); // Waits 3 seconds before the next reading
  }
}
