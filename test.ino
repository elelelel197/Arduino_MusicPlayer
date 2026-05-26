#include <Arduino.h>
#include <TM1637Display.h>

const int clkSensPin = 2;  // sensor pin number for clock in paper code (LM393#1)
const int codeSensPin = 5;  // sensor pin number for note code in paper code (LM393#2)
int codeWordLength = 6;

// Display
const int clkPin = 3;
const int dioPin = 4;
TM1637Display display(clkPin, dioPin);

// motor 2 (tunnel B)
const int in3 = 7;
const int in4 = 8;
const int enB = 9;

//speaker
const int speaker = 10;

// motor 1 (tunnel A)
const int enA = 11;
const int in1 = 12;
const int in2 = 13;

// buttons
const int btn0 = A0;
const int btn1 = A1;
const int btn2 = A2;
const int btn3 = A3;
const int btn4 = A4;

// motor default speed (0~255)
const int speedA = 0; 
const int speedB = 0;

// ========== TEST STATE VARIABLES ==========
boolean testActive = false;
int currentMotorSpeed = 0;  // 0, 50, or -50
int currentSensorDisplay = 1;  // 1 or 2 (for LM393#1 or LM393#2)
boolean speakerActive = false;

// Button state tracking - detect rising edge (LOW -> HIGH)
int lastButtonState[5] = {LOW, LOW, LOW, LOW, LOW};
unsigned long lastDebounceTime[5] = {0, 0, 0, 0, 0};
const unsigned long debounceDelay = 50;  // 50ms debounce for noise filtering

void setup() {
  Serial.begin(9600);
  
  // Setup button pins
  pinMode(btn0, INPUT);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(btn3, INPUT);
  pinMode(btn4, INPUT);
  
  // Setup sensor pins
  pinMode(clkSensPin, INPUT);
  pinMode(codeSensPin, INPUT);
  
  // Setup motor pins
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  // Setup speaker pin
  pinMode(speaker, OUTPUT);
  
  // Setup display
  display.setBrightness(0x0f);
  display.clear();
  
  Serial.println("System initialized. Ready for testing.");
}

void loop() {
  handleButtonInput();
}

// ========== BUTTON INPUT HANDLING ==========
void handleButtonInput() {
  // Button A0: Start test
  if (checkButtonPress(btn0, 0)) {
    testActive = true;
    currentMotorSpeed = 0;
    currentSensorDisplay = 1;
    speakerActive = false;
    display.showNumberDec(0);
    Serial.println("TEST STARTED - Display shows 0");
  }
  
  // Button A1: Set motor speed (50 or -50)
  if (testActive && checkButtonPress(btn1, 1)) {
    if (currentMotorSpeed == 0 || currentMotorSpeed == -100) {
      currentMotorSpeed = 100;
      setMotorSpeed(100, -100);
      display.showNumberDec(100);
      Serial.println("Motor speed set to 100");
    } else {
      currentMotorSpeed = -100;
      setMotorSpeed(-100, 100);
      display.showNumberDec(-100);
      Serial.println("Motor speed set to -100");
    }
  }
  
  // Button A2: Display sensor value (toggle between sensor 1 and 2)
  if (testActive && checkButtonPress(btn2, 2)) {
    if (currentSensorDisplay == 1) {
      currentSensorDisplay = 2;
    } else {
      currentSensorDisplay = 1;
    }
    displaySensorValue();
  }
  
  // Button A3: Play speaker sound
  if (testActive && checkButtonPress(btn3, 3)) {
    speakerActive = !speakerActive;
    if (speakerActive) {
      tone(speaker, 440, 200);  // Play 440 Hz for 200ms
      Serial.println("Speaker playing sound");
    } else {
      noTone(speaker);
      Serial.println("Speaker stopped");
    }
  }
  
  // Button A4: Stop all
  if (checkButtonPress(btn4, 4)) {
    testActive = false;
    currentMotorSpeed = 0;
    currentSensorDisplay = 1;
    speakerActive = false;
    
    // Stop motors
    setMotorSpeed(0, 0);
    
    // Stop speaker
    noTone(speaker);
    
    // Clear display
    display.clear();
    
    Serial.println("ALL SYSTEMS STOPPED");
  }
}

// ========== BUTTON PRESS DETECTION ==========
// Returns true only on rising edge (button pressed, LOW -> HIGH transition)
// Only triggers once per physical press - holding won't retrigger
boolean checkButtonPress(int buttonPin, int buttonIndex) {
  int currentButtonState = digitalRead(buttonPin);
  unsigned long currentTime = millis();
  
  // Debounce: check if state has been stable for debounceDelay
  if (currentButtonState != lastButtonState[buttonIndex]) {
    lastDebounceTime[buttonIndex] = currentTime;
  }
  
  // Only consider state change valid after debounceDelay
  if ((currentTime - lastDebounceTime[buttonIndex]) > debounceDelay) {
    // Detect rising edge: previous state was LOW, current state is HIGH
    if (lastButtonState[buttonIndex] == LOW && currentButtonState == HIGH) {
      lastButtonState[buttonIndex] = currentButtonState;
      return true;  // Button pressed
    }
    // Update state for next check
    lastButtonState[buttonIndex] = currentButtonState;
  }
  
  return false;
}

// ========== MOTOR CONTROL ==========
void setMotorSpeed(int speedA_val, int speedB_val) {
  // Motor A
  if (speedA_val > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(enA, speedA_val);
  } else if (speedA_val < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(enA, abs(speedA_val));
  } else {
    analogWrite(enA, 0);
  }
  
  // Motor B
  if (speedB_val > 0) {
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, speedB_val);
  } else if (speedB_val < 0) {
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    analogWrite(enB, abs(speedB_val));
  } else {
    analogWrite(enB, 0);
  }
}

// ========== SENSOR DISPLAY ==========
void displaySensorValue() {
  int sensorValue;
  
  if (currentSensorDisplay == 1) {
    sensorValue = digitalRead(clkSensPin);
    Serial.print("LM393 Sensor #1 value: ");
    Serial.println(sensorValue);
  } else {
    sensorValue = digitalRead(codeSensPin);
    Serial.print("LM393 Sensor #2 value: ");
    Serial.println(sensorValue);
  }
  
  // Display the sensor value on the 7-segment display
  display.showNumberDec(sensorValue * 1000 + currentSensorDisplay);  // Show sensor # and value
}
