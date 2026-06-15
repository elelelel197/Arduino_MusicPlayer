#include <Arduino.h>
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
#include <SevenSegmentTM1637.h>

#define clkSensPin 2  // sensor pin number for clock in paper code

#define codeSensPin 5  // sensor pin number for note code in paper code

#define mtr1fwPin 13  // motor 1 forward pin
#define mtr1bwPin 12  // motor 1 backward pin
#define mtr1pwmPin 11 // motor 1 PWM pin

#define mtr2fwPin 7 // motor 2 forward pin
#define mtr2bwPin 8 // motor 2 backward pin
#define mtr2pwmPin 9 // motor 2 PWM pin

#define spkrPin 10 // speaker pin

#define sevsegClkPin 3 // 7-segment display clock pin
#define sevsegDioPin 4 // 7-segment display data pin

#define btnPlayPin A0 // play button pin
#define btnRevMtrPin A1 // pause button pin
#define btnRestartPin A2 // restart button pin
#define btnReadPin A3 // read button pin
#define btnSetTrackPin A4 // set track button pin

#define MTR_SPEED_FRONT 75 // motor speed constant (0-255)
#define MTR_SPEED_BACK 50

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
  STATE_IDLE, STATE_PLAY, STATE_PAUSED, STATE_RESTART, STATE_READ, STATE_SET_TRACK, STATE_REV_MTR
};

NOTE noteMemory[MAX_TRACKS][MAX_NOTES_PER_TRACK] = {{NOTE_END}, {NOTE_END}, {NOTE_END}, {NOTE_END}, {NOTE_END}}; // 2D array to store the note codes for each player, initialized to IDLE

SevenSegmentTM1637 display(sevsegClkPin, sevsegDioPin); // initialize the 7-segment display object

#line 51 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void setup();
#line 79 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void loop();
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\button.ino"
STATE btnOutput();
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
uint8_t readData(uint8_t currentTrack, uint8_t noteIndex);
#line 79 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
bool sensClkPosEdge(bool clkdebounced);
#line 86 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
uint8_t parityCheck(uint8_t data);
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\motor.ino"
void driveMtr1(bool direction, uint8_t speed);
#line 13 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\motor.ino"
void driveMtr2(bool direction, uint8_t speed);
#line 25 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\motor.ino"
void stopMtr1();
#line 32 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\motor.ino"
void stopMtr2();
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\speaker.ino"
void speakerPlayNote(NOTE note, int duration);
#line 11 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\speaker.ino"
int NoteToFrequency(NOTE note);
#line 2 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\utility.ino"
bool debounce(uint8_t input, uint8_t &buffer, bool &debouncedOut);
#line 23 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\utility.ino"
bool onepulse(bool input, bool &lastInput);
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
  pinMode(btnRevMtrPin, INPUT_PULLUP);
  pinMode(btnRestartPin, INPUT_PULLUP);
  pinMode(btnReadPin, INPUT_PULLUP);
  pinMode(btnSetTrackPin, INPUT_PULLUP);
}

void loop() {
  static uint8_t currentTrack = 0; // variable to keep track of the current track/player
  static uint8_t noteIndex = 0; // variable to keep track of the current note index for the current track
  static STATE currentState = STATE_IDLE; // variable to keep track of the current state of the system
  static STATE nextState = STATE_IDLE; // variable to keep track of the next state of the system
  static STATE btnRequest = STATE_IDLE; // variable to receive btnOutput func
  static unsigned long lastNoteTime = 0; // variable to keep track of the time when the last note was played
  static bool motorDir = true; // spin direction of motors

  // test readData function
  // display.clear(); // clear the 7-segment display
  // display.print("READ"); // display "READ" on the 7-segment display

  // noteIndex = 0; // reset note index to start reading from the beginning of the track
  // uint8_t readSuccess = readData(currentTrack, noteIndex); // function to read data from the sensors and store it in the note memory
  // if (readSuccess) {
  //   noteIndex++; // move to the next note index for the next read
  //   }

  // Serial.print("note index: ");
  // Serial.println(noteIndex);

  btnRequest = btnOutput();
  // update nextState if request is drawn
  if(btnRequest != STATE_IDLE){ 
    // play and pause is controlled by play btn
    if(currentState == STATE_PLAY && btnRequest == STATE_PLAY){
      nextState = STATE_PAUSED;
    }
    else if(currentState == STATE_REV_MTR && btnRequest == STATE_REV_MTR){
      nextState = STATE_IDLE;
    }
    else if(currentState == STATE_READ && btnRequest == STATE_READ){
      nextState = STATE_IDLE;
    }
    else {
      nextState = btnRequest;
    }
    // If read is interrupted, put a end note in the back
    if(currentState == STATE_READ && noteMemory[currentTrack][noteIndex] != NOTE_END){
      noteMemory[currentTrack][noteIndex + 1] = NOTE_END; 
    }
    if(currentState == STATE_READ && nextState != STATE_READ){
      noteIndex = 0;
    }
  }
  // change nextState to idle after end note is reached
  else if(noteMemory[currentTrack][noteIndex] == NOTE_END && currentState == STATE_PLAY){
    nextState = STATE_IDLE;
  }
  // change nextState to idle if currentState is restart of set track
  else if(currentState == STATE_SET_TRACK || currentState == STATE_RESTART){
    nextState = STATE_IDLE;
  }

  currentState = nextState;

  if (currentState == STATE_IDLE) {
    Serial.println("idle");
  
    display.clear(); // clear the 7-segment display
    display.print("IDLE"); // display IDLE on the 7-segment display
  
  } 
  else if (currentState == STATE_PLAY) {
    Serial.println("play");
  
    display.clear(); // clear the 7-segment display
    display.print("PLAY"); // display PLAY on the 7-segment display
  
    NOTE note = noteMemory[currentTrack][noteIndex]; // get the current note for the current track
    unsigned long currentTime = millis(); // get the current time
    if (note == NOTE_START) {
      lastNoteTime = currentTime;
      noteIndex++; // move to the next note
    } else if (note == NOTE_END) {
      noteIndex = 0; // reset note index for the next playthrough
    } else if ((currentTime - lastNoteTime) >= NOTE_DURATION) { // check if it's time to play the next note
      speakerPlayNote(note, NOTE_DURATION); // function to play the note on the speaker
      lastNoteTime = currentTime; // update the last note time
      noteIndex++; // move to the next note
    }
  
  } 
  else if (currentState == STATE_PAUSED) {
    Serial.println("pause");
  
    display.clear(); // clear the 7-segment display
    display.print("PAUS"); // display "PAUS" on the 7-segment display
    // display.blink(); // make the display blink to indicate paused state
  
  } 
  else if (currentState == STATE_RESTART) {
    Serial.println("restart");
  
    display.clear(); // clear the 7-segment display
    display.print("REST"); // display "REST" on the 7-segment display
    delay(1000); // delay for a moment to show the restart message
  
    noteIndex = 0; // reset note index to restart the track
  
  } 
  else if (currentState == STATE_READ) {
    // Serial.println("read");
  
    display.clear(); // clear the 7-segment display
    display.print("READ"); // display "READ" on the 7-segment display
  
    uint8_t readSuccess = readData(currentTrack, noteIndex); // function to read data from the sensors and store it in the note memory
    if (readSuccess) {
      noteIndex++; // move to the next note index for the next read
    }
  } 
  else if (currentState == STATE_SET_TRACK) {
    Serial.println("set track");

    display.clear(); // clear the 7-segment display
    String trackStr = "TR" + String(currentTrack + 1); // create a string to display the current track number (1-indexed)
    display.print(trackStr); // display the current track number on the 7-segment display
    delay(1000); // delay for a moment to show the track number

    currentTrack = (currentTrack + 1) % MAX_TRACKS; // cycle through tracks
    noteIndex = 0;

  }
  else if (currentState == STATE_REV_MTR) {
    Serial.println("motor reverse");

    display.clear(); // clear the 7-segment display
    display.print("REVM"); // display "READ" on the 7-segment display
  }
  else {
    Serial.println("state exception"); // default to idle state
  }
  

  // ensure speaker off if not playing
  if (currentState != STATE_PLAY) 
    noTone(spkrPin);

  if(currentState == STATE_READ){
    driveMtr1(true, MTR_SPEED_FRONT); 
    driveMtr2(true, MTR_SPEED_BACK); 
  }
  else if (currentState == STATE_REV_MTR){
    driveMtr1(false, MTR_SPEED_BACK); 
    driveMtr2(false, MTR_SPEED_FRONT); 
  }
  else {
    stopMtr1(); // function to stop motor 1
    stopMtr2(); // function to stop motor 2
  }
  
}


#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\button.ino"
STATE btnOutput(){
    static bool lastDebBtnPlay = 0; // variable to store the last state of the play button for deb
    static bool lastDebBtnRevMtr = 0;
    static bool lastDebBtnRestart = 0;
    static bool lastDebBtnRead = 0;
    static bool lastDebBtnSetTrack = 0;

    static bool lastOpBtnPlay = 0; // variable to store the last state of the play button for Op
    static bool lastOpBtnRevMtr = 0;
    static bool lastOpBtnRestart = 0;
    static bool lastOpBtnRead = 0;
    static bool lastOpBtnSetTrack = 0;

    static uint8_t btnPlayBuffer = 0; // buffer variable for button states
    static uint8_t btnRevMtrBuffer = 0; 
    static uint8_t btnRestartBuffer = 0; 
    static uint8_t btnReadBuffer = 0; 
    static uint8_t btnSetTrackBuffer = 0; 

    bool onePulseBtnPlay = onepulse(debounce(digitalRead(btnPlayPin), btnPlayBuffer, lastDebBtnPlay), lastOpBtnPlay);
    bool onePulseBtnRevMtr = onepulse(debounce(digitalRead(btnRevMtrPin), btnRevMtrBuffer, lastDebBtnRevMtr), lastOpBtnRevMtr);
    bool onePulseBtnRestart = onepulse(debounce(digitalRead(btnRestartPin), btnRestartBuffer, lastDebBtnRestart), lastOpBtnRestart);
    bool onePulseBtnRead = onepulse(debounce(digitalRead(btnReadPin), btnReadBuffer, lastDebBtnRead), lastOpBtnRead);
    bool onePulseBtnSetTrack = onepulse(debounce(digitalRead(btnSetTrackPin), btnSetTrackBuffer, lastDebBtnSetTrack), lastOpBtnSetTrack);

    // Serial.print("Button input:");
    // Serial.print(onePulseBtnPlay);
    // Serial.print(onePulseBtnRevMtr);
    // Serial.print(onePulseBtnRestart);
    // Serial.print(onePulseBtnRead);
    // Serial.println(onePulseBtnSetTrack);

    if (onePulseBtnPlay) {
        return STATE_PLAY;
    } else if (onePulseBtnRevMtr) {
        return STATE_REV_MTR;
    } else if (onePulseBtnRestart) {
        return STATE_RESTART;
    } else if (onePulseBtnRead) {
        return STATE_READ;
    } else if (onePulseBtnSetTrack) {
        return STATE_SET_TRACK;
    } else {
        return STATE_IDLE;
    }
}
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
uint8_t readData(uint8_t currentTrack, uint8_t noteIndex) {
  uint8_t clkSensIn = digitalRead(clkSensPin);
  uint8_t codeSensIn = digitalRead(codeSensPin);

  // For debugging: print the raw sensor inputs
  Serial.print("Raw Clock: ");
  Serial.println(clkSensIn);
  Serial.print("Raw Data");
  Serial.println(codeSensIn);

  static uint8_t clkSensBuffer = 0;
  static uint8_t codeSensBuffer = 0;

  static bool lastClkSens = 0;
  static bool lastCodeSens = 0;

  uint8_t clkDebounced = debounce(clkSensIn, clkSensBuffer, lastClkSens);
  uint8_t codeDebounced = debounce(codeSensIn, codeSensBuffer, lastCodeSens);

  // For debugging: print the debounced sensor values
  // Serial.print("Debounced Clock: ");
  // Serial.println(clkDebounced);

  static uint8_t dataBuffer = 0;
  static uint8_t wordCounter = 0;
  static uint8_t startFlag = 0;
  bool readSucFlag = 0;

  if (sensClkPosEdge(clkDebounced)) {
    dataBuffer <<= 1;
    dataBuffer |= codeDebounced;
    dataBuffer &= 0x3F; // Keep only the last 6 bits
    // Serial.print("Data buffer: ");
    // Serial.println(dataBuffer, BIN);
    wordCounter++;
    if (parityCheck(dataBuffer)) {
      // Once start code is detected, we can process the note code
      uint8_t noteCode = dataBuffer;
      noteCode >>= 1; // Shift right to get bits 1..7 as the note code
      // Serial.print("Note code: ");
      // Serial.println(noteCode, BIN);
      // Serial.print("wordCounter: ");
      // Serial.println(wordCounter);
      if (noteCode == NOTE_START && !startFlag) {
        Serial.println("Received START note code.");
        startFlag = 1;
        readSucFlag = 1;
        wordCounter = 0;
        noteMemory[currentTrack][noteIndex] = NOTE_START; // Store the START code in the note memory
      }
      else if (startFlag && (wordCounter == CODE_WORD_LENGTH)) {
        NOTE note = static_cast<NOTE>(noteCode); // Convert the note code to the NOTE enum
        // Once end code is detected, we can reset the start flag
        if (note == NOTE_END) {
          startFlag = 0;
          wordCounter = 0;
          Serial.println("Received END note code.");
        } else if (note == NOTE_IDLE) {
          Serial.println("Received IDLE note code.");
        } else {
          Serial.print("Received note: ");
          Serial.println(note);
        } 
        noteMemory[currentTrack][noteIndex] = note; // Store the note in the note memory
        
        readSucFlag = 1;
      }
    }
    if (wordCounter >= CODE_WORD_LENGTH) 
      wordCounter = 0; // Reset word counter for the next code word
  }

  if (readSucFlag)
    return 1;
  else
    return 0;
}

bool sensClkPosEdge(bool clkdebounced) {
  static bool lastClk = LOW;
  bool posEdge = (lastClk == LOW) && (clkdebounced == HIGH);
  lastClk = clkdebounced;
  return posEdge;
}

uint8_t parityCheck(uint8_t data) {
  uint8_t parity = 1;
  for (uint8_t i = 0; i < 8; i++) {
    parity ^= (data >> i) & 1;
  }
  return parity;
}
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\motor.ino"
void driveMtr1(bool direction, uint8_t speed) {
  // Serial.println("mtr1active");
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
  // Serial.println("mtr2active");
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
// 4-sample debounce: state changes only after 0b0000 -> 0 or 0b1111 -> 1
bool debounce(uint8_t input, uint8_t &buffer, bool &debouncedOut) {
  const uint8_t WINDOW_MASK = 0x03; // keep last 3 samples
  // ensure input is 0 or 1
  input = (input ? 1 : 0);

  buffer = (buffer << 1) | input;
  buffer &= WINDOW_MASK;

  // for debugging: print the buffer in binary
  // Serial.print("Buffer: ");
  // Serial.println(buffer, HEX);

  // only change output when buffer is all zeros or all ones
  if (buffer == 0x00) {
    debouncedOut = false;
  } else if (buffer == WINDOW_MASK) {
    debouncedOut = true;
  }
  return debouncedOut;
}

bool onepulse(bool input, bool &lastInput){
  bool pulseOut = (lastInput == HIGH) && (input == LOW);
  lastInput = input;
  return pulseOut;
}
