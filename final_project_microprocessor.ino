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

