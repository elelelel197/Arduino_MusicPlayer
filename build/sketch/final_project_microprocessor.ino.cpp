#include <Arduino.h>
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
#include <SevenSegmentTM1637.h>

#define clkSensPin 2  // sensor pin number for clock in paper code

#define codeSensPin 5  // sensor pin number for note code in paper code

#define mtr1fwPin 11  // motor 1 forward pin
#define mtr1bwPin 12  // motor 1 backward pin
#define mtr1pwmPin 13 // motor 1 PWM pin

#define mtr2fwPin 7 // motor 2 forward pin
#define mtr2bwPin 8 // motor 2 backward pin
#define mtr2pwmPin 9 // motor 2 PWM pin

#define spkrPin 10 // speaker pin

#define sevsegClkPin 3 // 7-segment display clock pin
#define sevsegDioPin 4 // 7-segment display data pin

#define btnPlayPin A0 // play button pin
#define btnPausePin A1 // pause button pin
#define btnRestartPin A2 // restart button pin
#define btnReadPin A3 // read button pin
#define btnSetTrackPin A4 // set track button pin

#define MTR_DIR true // motor direction constant for forward direction
#define MTR_SPEED 200 // motor speed constant (0-255)

#define CODE_WORD_LENGTH 6 // number of bits in the note code word

#define NOTE_DURATION 500 // duration of each note in milliseconds

#define MAX_TRACKS 5 // maximum number of tracks/players
#define MAX_NOTES_PER_TRACK 100 // maximum number of notes per track

enum NOTE {
  NOTE_IDLE, NOTE_START, NOTE_END, 
  NOTE_A2, NOTE_A2s, NOTE_B2, NOTE_C3, NOTE_C3s, NOTE_D3, NOTE_D3s, NOTE_E3, NOTE_F3, NOTE_F3s, NOTE_G3, NOTE_G3s,
  NOTE_A3, NOTE_A3s, NOTE_B3, NOTE_C4, NOTE_C4s, NOTE_D4, NOTE_D4s, NOTE_E4, NOTE_F4, NOTE_F4s, NOTE_G4, NOTE_G4s,
  NOTE_A4, NOTE_A4s, NOTE_B4, NOTE_C5, NOTE_C5s
};

enum STATE {
  STATE_IDLE, STATE_READ, STATE_PLAY, STATE_PAUSED, STATE_RESTART, STATE_SET_TRACK
};

NOTE noteMemory[MAX_TRACKS][MAX_NOTES_PER_TRACK] = {{NOTE_IDLE}}; // 2D array to store the note codes for each player, initialized to IDLE

SevenSegmentTM1637 display(sevsegClkPin, sevsegDioPin); // initialize the 7-segment display object

#line 51 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void setup();
#line 79 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void loop();
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\button.ino"
STATE btnOutput();
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
uint8_t readData(uint8_t currentTrack, uint8_t noteIndex);
#line 52 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
uint8_t sensClkPosEdge(uint8_t clkdebounced);
#line 59 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
uint8_t parityCheck(uint8_t data);
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\motor.ino"
void driveMtr1(bool direction, uint8_t speed);
#line 12 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\motor.ino"
void driveMtr2(bool direction, uint8_t speed);
#line 23 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\motor.ino"
void stopMtr1();
#line 30 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\motor.ino"
void stopMtr2();
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\speaker.ino"
void speakerPlayNote(NOTE note, int duration);
#line 11 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\speaker.ino"
int NoteToFrequency(NOTE note);
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\utility.ino"
bool debounce(bool input, uint8_t buffer);
#line 12 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\utility.ino"
bool onepulse(bool input);
#line 51 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void setup() {
  Serial.begin(9600);

  pinMode(clkSensPin, INPUT);
  pinMode(codeSensPin, INPUT);

  pinMode(mtr1fwPin, OUTPUT);
  pinMode(mtr1bwPin, OUTPUT);
  pinMode(mtr1pwmPin, OUTPUT);
  pinMode(mtr2fwPin, OUTPUT);
  pinMode(mtr2bwPin, OUTPUT);
  pinMode(mtr2pwmPin, OUTPUT);

  pinMode(spkrPin, OUTPUT); 

  pinMode(sevsegClkPin, OUTPUT);
  pinMode(sevsegDioPin, OUTPUT);

  display.begin(); // initialize the 7-segment display
  display.setBacklight(100);  // set the brightness to 100 %

  pinMode(btnPlayPin, INPUT_PULLUP);
  pinMode(btnPausePin, INPUT_PULLUP);
  pinMode(btnRestartPin, INPUT_PULLUP);
  pinMode(btnReadPin, INPUT_PULLUP);
  pinMode(btnSetTrackPin, INPUT_PULLUP);
}

void loop() {
  static uint8_t currentTrack = 0; // variable to keep track of the current track/player
  static uint8_t noteIndex = 0; // variable to keep track of the current note index for the current track
  static STATE currentState = STATE_IDLE; // variable to keep track of the current state of the system
  static STATE nextState = STATE_IDLE; // variable to keep track of the next state of the system
  static unsigned long lastNoteTime = 0; // variable to keep track of the time when the last note was played

  // test readData function
  // bool readsuccess = readData(currentTrack, noteIndex++);

  switch (currentState) {
    case STATE_IDLE:
      display.clear(); // clear the 7-segment display
      display.print("IDLE"); // display IDLE on the 7-segment display

      if (nextState != STATE_IDLE) {
        currentState = nextState;
      }
      break;
    case STATE_PLAY:
      display.clear(); // clear the 7-segment display
      display.print("PLAY"); // display PLAY on the 7-segment display

      NOTE note = noteMemory[currentTrack][noteIndex]; // get the current note for the current track
      unsigned long currentTime = millis(); // get the current time
      if(note == NOTE_START) {
        lastNoteTime = currentTime;
        noteIndex++; // move to the next note
      } 
      else if(note == NOTE_END) {
        noteIndex = 0; // reset note index for the next playthrough
        currentState = STATE_IDLE; // go back to idle after finishing the track
      } 
      else if((currentTime - lastNoteTime) >= NOTE_DURATION) { // check if it's time to play the next note
        speakerPlayNote(note, NOTE_DURATION); // function to play the note on the speaker
        lastNoteTime = currentTime; // update the last note time
        noteIndex++; // move to the next note
      }

      if (nextState == STATE_IDLE) {
        currentState = STATE_PLAY; // stay in play if nothing is pressed
      } 
      else {
        currentState = nextState; // switch to the next state
      }
      break;
    case STATE_PAUSED:
      display.clear(); // clear the 7-segment display
      display.print("PAUS"); // display "PAUS" on the 7-segment display
      display.blink(); // make the display blink to indicate paused state

      if (nextState == STATE_IDLE) {
        currentState = STATE_PAUSED; // stay in paused if nothing is pressed
      } else {
        currentState = nextState; // switch to the next state
      }
      break;
    case STATE_RESTART:
      display.clear(); // clear the 7-segment display
      display.print("REST"); // display "REST" on the 7-segment display
      delay(1000); // delay for a moment to show the restart message

      noteIndex = 0; // reset note index to restart the track
      currentState = nextState; // switch to the next state
      break;
    case STATE_READ:
      display.clear(); // clear the 7-segment display
      display.print("READ"); // display "READ" on the 7-segment display

      noteIndex = 0; // reset note index to start reading from the beginning of the track
      uint8_t readSuccess = readData(currentTrack, noteIndex); // function to read data from the sensors and store it in the note memory
      if (readSuccess) {
        noteIndex++; // move to the next note index for the next read
      }

      if(nextState == STATE_IDLE) {
        currentState = STATE_READ; // stay in read if nothing is pressed
      } 
      else if(noteMemory[currentTrack][noteIndex] == NOTE_END) {
        currentState = STATE_IDLE; // finish reading and go back to idle if the end note is reached
      }
      else {
        currentState = nextState; // switch to the next state
      }
      break;
    case STATE_SET_TRACK:
      display.clear(); // clear the 7-segment display
      String trackStr = "TR" + String(currentTrack + 1); // create a string to display the current track number (1-indexed)
      display.print(trackStr); // display the current track number on the 7-segment display
      delay(1000); // delay for a moment to show the track number

      currentTrack = (currentTrack + 1) % MAX_TRACKS; // cycle through tracks
      if (nextState != STATE_SET_TRACK) {
        currentState = nextState;
      } 
      else {
        currentState = STATE_IDLE; // prevent staying in SET_TRACK state if the button is held down
      }
      break;
    default:
      currentState = STATE_IDLE; // default to idle state
  }

  if(currentState != STATE_PLAY) {
    noTone(spkrPin); // stop any currently playing tone
  }

  if(currentState == STATE_READ){
    driveMtr1(MTR_DIR, MTR_SPEED); // function to drive motor 1 forward at the specified speed
    driveMtr2(MTR_DIR, MTR_SPEED); 
  }
  else {
    stopMtr1(); // function to stop motor 1
    stopMtr2(); // function to stop motor 2
  }

  nextState = btnOutput(); // function to read button states and determine the next state
  
}


#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\button.ino"
STATE btnOutput(){
    static bool onePulsebtnPlay = 0; // variable for debouncing the play button
    static bool onePulsebtnPause = 0; // variable for debouncing the pause button
    static bool onePulsebtnRestart = 0; // variable for debouncing the restart button
    static bool onePulsebtnRead = 0; // variable for debouncing the read button
    static bool onePulsebtnSetTrack = 0; // variable for debouncing the set track

    static uint8_t btnPlayBuffer = 0; // buffer variable for the play button state
    static uint8_t btnPauseBuffer = 0; // buffer variable for the pause button state
    static uint8_t btnRestartBuffer = 0; // buffer variable for the restart button state
    static uint8_t btnReadBuffer = 0; // buffer variable for the read button state
    static uint8_t btnSetTrackBuffer = 0; // buffer variable for the set track button state

    onePulsebtnPlay = onepulse(debounce(digitalRead(btnPlayPin), btnPlayBuffer));
    onePulsebtnPause = onepulse(debounce(digitalRead(btnPausePin), btnPauseBuffer));
    onePulsebtnRestart = onepulse(debounce(digitalRead(btnRestartPin), btnRestartBuffer));
    onePulsebtnRead = onepulse(debounce(digitalRead(btnReadPin), btnReadBuffer));
    onePulsebtnSetTrack = onepulse(debounce(digitalRead(btnSetTrackPin), btnSetTrackBuffer));

    if (onePulsebtnPlay) {
        return STATE_PLAY;
    } else if (onePulsebtnPause) {
        return STATE_PAUSED;
    } else if (onePulsebtnRestart) {
        return STATE_RESTART;
    } else if (onePulsebtnRead) {
        return STATE_READ;
    } else if (onePulsebtnSetTrack) {
        return STATE_SET_TRACK;
    } else {
        return STATE_IDLE;
    }
}
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
uint8_t readData(uint8_t currentTrack, uint8_t noteIndex) {
  static uint8_t clkSensIn = digitalRead(clkSensPin);
  static uint8_t codeSensIn = digitalRead(codeSensPin);

  static uint8_t clkSensBuffer = 0;
  static uint8_t codeSensBuffer = 0;

  static uint8_t clkDebounced = debounce(clkSensIn, clkSensBuffer);
  static uint8_t codeDebounced = debounce(codeSensIn, codeSensBuffer);

  static uint8_t dataBuffer = 0;
  static uint8_t wordCounter = 0;
  static uint8_t startFlag = 0;

  if (sensClkPosEdge(clkDebounced)) {
    dataBuffer <<= 1;
    dataBuffer |= codeDebounced;
    dataBuffer &= 0x3F; // Keep only the last 6 bits
    wordCounter++;
    if (wordCounter >= CODE_WORD_LENGTH - 1) {
       wordCounter = 0; // Reset word counter for the next code word
    }
    if (parityCheck(dataBuffer)) {
      // Once start code is detected, we can process the note code
      if (dataBuffer == NOTE_START) {
        startFlag = 1; 
        wordCounter = 0;
        noteMemory[currentTrack][noteIndex] = NOTE_START; // Store the START code in the note memory
      }
      else if (startFlag && wordCounter == CODE_WORD_LENGTH - 1) {
        NOTE note = static_cast<NOTE>(dataBuffer);
        // Once end code is detected, we can reset the start flag
        if (note == NOTE_END) {
          startFlag = 0;
          wordCounter = 0;
          Serial.println("Received END note code.");
        } else if (note == NOTE_IDLE) {
          Serial.println("Received IDLE note code.");
        } else {
          Serial.println("Received note: ");
          Serial.println(note);
        } 
        noteMemory[currentTrack][noteIndex] = note; // Store the note in the note memory
      }
      return 1; // Indicate successful read
    }
  }

  return 0; // Indicate unsuccessful read
}

uint8_t sensClkPosEdge(uint8_t clkdebounced) {
  static uint8_t lastClk = LOW;
  uint8_t posEdge = (lastClk == LOW) && (clkdebounced == HIGH);
  lastClk = clkdebounced;
  return posEdge;
}

uint8_t parityCheck(uint8_t data) {
  uint8_t parity = 0;
  for (uint8_t i = 0; i < 8; i++) {
    parity ^= (data >> i) & 1;
  }
  return parity;
}
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\motor.ino"
void driveMtr1(bool direction, uint8_t speed) {
  if (direction) {
    digitalWrite(mtr1fwPin, HIGH);
    digitalWrite(mtr1bwPin, LOW);
  } else {
    digitalWrite(mtr1fwPin, LOW);
    digitalWrite(mtr1bwPin, HIGH);
  }
  analogWrite(mtr1pwmPin, speed);
}

void driveMtr2(bool direction, uint8_t speed) {
  if (direction) {
    digitalWrite(mtr2fwPin, HIGH);
    digitalWrite(mtr2bwPin, LOW);
  } else {
    digitalWrite(mtr2fwPin, LOW);
    digitalWrite(mtr2bwPin, HIGH);
  }
  analogWrite(mtr2pwmPin, speed);
}

void stopMtr1() {
  // coast: disable PWM (write 0) and set inputs low
  analogWrite(mtr1pwmPin, 0);
  digitalWrite(mtr1fwPin, LOW);
  digitalWrite(mtr1bwPin, LOW);
}

void stopMtr2() {
  analogWrite(mtr2pwmPin, 0);
  digitalWrite(mtr2fwPin, LOW);
  digitalWrite(mtr2bwPin, LOW);
}
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\speaker.ino"
void speakerPlayNote(NOTE note, int duration) {
    int frequency = NoteToFrequency(note);
    if (frequency != -1) {
        tone(spkrPin, frequency, duration);
    }
    else {
        noTone(spkrPin); // Stop any currently playing tone
    }
}

int NoteToFrequency(NOTE note) {
  switch (note) {
    case NOTE_A2: return 110;
    case NOTE_A2s: return 116;
    case NOTE_B2: return 123;
    case NOTE_C3: return 130;
    case NOTE_C3s: return 138;
    case NOTE_D3: return 146;
    case NOTE_D3s: return 155;
    case NOTE_E3: return 164;
    case NOTE_F3: return 174;
    case NOTE_F3s: return 185;
    case NOTE_G3: return 196;
    case NOTE_G3s: return 207;
    case NOTE_A3: return 220;
    case NOTE_A3s: return 233;
    case NOTE_B3: return 246;
    case NOTE_C4: return 261;
    case NOTE_C4s: return 277;
    case NOTE_D4: return 293;
    case NOTE_D4s: return 311;
    case NOTE_E4: return 329;
    case NOTE_F4: return 349;
    case NOTE_F4s: return 370;
    case NOTE_G4: return 392;
    case NOTE_G4s: return 415;
    case NOTE_A4: return 440;
    case NOTE_A4s: return 466;
    case NOTE_B4: return 493;
    case NOTE_C5: return 523;
    case NOTE_C5s: return 554;
    default: return -1;
  }
}


#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\utility.ino"
bool debounce(bool input, uint8_t buffer){
  buffer <<= 1;
  buffer |= input;
  bool debOut = input;
  if(buffer == 0)
    debOut = 0;
  else if(buffer == 255)
    debOut = 1;
  return debOut;
}

bool onepulse(bool input){
  static bool lastInput = LOW;
  bool pulseOut = (lastInput == LOW) && (input == HIGH);
  lastInput = input;
  return pulseOut;
}
