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
  // display.clear(); // clear the 7-segment display
  // display.print("READ"); // display "READ" on the 7-segment display

  // noteIndex = 0; // reset note index to start reading from the beginning of the track
  // uint8_t readSuccess = readData(currentTrack, noteIndex); // function to read data from the sensors and store it in the note memory
  // if (readSuccess) {
  //   noteIndex++; // move to the next note index for the next read
  //   }

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

  // if(currentState == STATE_READ){
  //   driveMtr1(MTR_DIR, MTR_SPEED); // function to drive motor 1 forward at the specified speed
  //   driveMtr2(MTR_DIR, MTR_SPEED); 
  // }
  // else {
  //   stopMtr1(); // function to stop motor 1
  //   stopMtr2(); // function to stop motor 2
  // }

  nextState = btnOutput(); // function to read button states and determine the next state
  
}

