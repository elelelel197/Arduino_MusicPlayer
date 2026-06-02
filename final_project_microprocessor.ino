#include <DIYables_4Digit7Segment_TM1637.h>

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

#define CODE_WORD_LENGTH 6 // number of bits in the note code word

#define NOTE_DURATION 500 // duration of each note in milliseconds

#define MAX_TRACKS 5 // maximum number of tracks/players
#define MAX_NOTES_PER_TRACK 100 // maximum number of notes per track

enum NOTE {
  IDLE, START, END, 
  NOTE_A2, NOTE_A2s, NOTE_B2, NOTE_C3, NOTE_C3s, NOTE_D3, NOTE_D3s, NOTE_E3, NOTE_F3, NOTE_F3s, NOTE_G3, NOTE_G3s,
  NOTE_A3, NOTE_A3s, NOTE_B3, NOTE_C4, NOTE_C4s, NOTE_D4, NOTE_D4s, NOTE_E4, NOTE_F4, NOTE_F4s, NOTE_G4, NOTE_G4s,
  NOTE_A4, NOTE_A4s, NOTE_B4, NOTE_C5, NOTE_C5s
};

enum STATE {
  IDLE, READ, PLAY, PAUSED, RESTART, SET_TRACK
};

NOTE noteMemory[MAX_TRACKS][MAX_NOTES_PER_TRACK] = IDLE; // 2D array to store the note codes for each player, initialized to IDLE

void setup() {Serial.begin(9600);
         
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

  pinMode(btnPlayPin, INPUT_PULLDOWN);
  pinMode(btnPausePin, INPUT_PULLDOWN);
  pinMode(btnRestartPin, INPUT_PULLDOWN);
  pinMode(btnReadPin, INPUT_PULLDOWN);
  pinMode(btnSetTrackPin, INPUT_PULLDOWN);
}

void loop() {
  static int currentTrack = 0; // variable to keep track of the current track/player
  static int noteIndex = 0; // variable to keep track of the current note index for the current track
  static STATE currentState = IDLE; // variable to keep track of the current state of the system
  static STATE nextState = IDLE; // variable to keep track of the next state of the system
  static lastNoteTime = 0; // variable to keep track of the time when the last note was played

  switch (currentState) {
    case IDLE:
      if (nextState != IDLE) {
        currentState = nextState;
      }
      break;
    case PLAY:
      NOTE note = noteMemory[currentTrack][noteIndex]; // get the current note for the current track
      unsigned long currentTime = millis(); // get the current time
      if(note == START) {
        lastNoteTime = currentTime;
        noteIndex++; // move to the next note
      } else if(note == END) {
        noteIndex = 0; // reset note index for the next playthrough
        currentState = IDLE; // go back to idle after finishing the track
      }
      else if(note != IDLE && (currentTime - lastNoteTime) >= NOTE_DURATION) { // check if it's time to play the next note
        playNoteOnSpeaker(note); // function to play the note on the speaker
        lastNoteTime = currentTime; // update the last note time
        noteIndex++; // move to the next note
      }
      if (nextState == IDLE) {
        currentState = PLAY; // stay in play if nothing is pressed
      } else {
        currentState = nextState; // switch to the next state
      }
      break;
    case PAUSED:
      if (nextState == IDLE) {
        currentState = PAUSED; // stay in paused if nothing is pressed
      } else {
        currentState = nextState; // switch to the next state
      }
      break;
    case RESTART:
      noteIndex = 0; // reset note index to restart the track
      currentState = nextState; // switch to the next state
      break;
    case READ:
      noteIndex = 0; // reset note index to start reading from the beginning of the track
      int readsuccess = readData(currentTrack, noteIndex); // function to read data from the sensors and store it in the note memory
      if (readsuccess) {
        noteIndex++; // move to the next note index for the next read
      }
      
      if(nextState == IDLE) {
        currentState = READ; // stay in read if nothing is pressed
      } else {
        currentState = nextState; // switch to the next state
      }
      break;
    case SET_TRACK:
      currentTrack = (currentTrack + 1) % MAX_TRACKS; // cycle through tracks
      Serial.print("Current Track: ");
      Serial.println(currentTrack);
      if (nextState != SET_TRACK) {
        currentState = nextState;
      } else {
        currentState = IDLE; // prevent staying in SET_TRACK state if the button is held down
      }
      break;
    case READ:
  }

  nextState = btnOutput(); // function to read button states and determine the next state

}

