#define buzzer 13
#define system_name "Intelligent Vehicle Instrumentation System"
#define system_version "v0.0.1"

String command = ""; // Variable that stores the command typed in the serial monitor.

void setup() {
  Serial.begin(115200);
  Serial.println("\n===========================================================");
  Serial.println(system_name " - Version " system_version);
  Serial.println("Developed by https://github.com/mariaeduardapj");
  Serial.println("===========================================================\n");

  Serial.println("COMMAND LIST");
  Serial.println("buzzer.on - Turn on the buzzer;");
  Serial.println("buzzer.off - Turn off the buzzer;");
}

void loop() {
  if (Serial.available() > 0){ // If something was read on the serial monitor
    command = Serial.readStringUntil('\n'); // Read until you press enter
    command.trim(); // Remove spaces and line breaks

    if (command == "buzzer.on"){ // If the command is buzzer.on, turn on the buzzer
      tone(buzzer, 2000);
    }   
    else if (command  == "buzzer.off"){ // If the command is buzzer.off, turn off the buzzer
      noTone(buzzer);
    }

    else { // If it's not one of the commands in the pre-made list, then the command is invalid.
      Serial.println("Invalid command");
    }
  }
}
