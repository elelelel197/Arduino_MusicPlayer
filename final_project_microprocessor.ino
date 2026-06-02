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
  NOTE_IDLE, NOTE_START, NOTE_END, 
  NOTE_A2, NOTE_A2s, NOTE_B2, NOTE_C3, NOTE_C3s, NOTE_D3, NOTE_D3s, NOTE_E3, NOTE_F3, NOTE_F3s, NOTE_G3, NOTE_G3s,
  NOTE_A3, NOTE_A3s, NOTE_B3, NOTE_C4, NOTE_C4s, NOTE_D4, NOTE_D4s, NOTE_E4, NOTE_F4, NOTE_F4s, NOTE_G4, NOTE_G4s,
  NOTE_A4, NOTE_A4s, NOTE_B4, NOTE_C5, NOTE_C5s
};

enum STATE {
  STATE_IDLE, STATE_READ, STATE_PLAY, STATE_PAUSED, STATE_RESTART, STATE_SET_TRACK
};

NOTE noteMemory[MAX_TRACKS][MAX_NOTES_PER_TRACK] = {{NOTE_IDLE}}; // 2D array to store the note codes for each player, initialized to IDLE

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

  pinMode(btnPlayPin, INPUT_PULLUP);
  pinMode(btnPausePin, INPUT_PULLUP);
  pinMode(btnRestartPin, INPUT_PULLUP);
  pinMode(btnReadPin, INPUT_PULLUP);
  pinMode(btnSetTrackPin, INPUT_PULLUP);
}

void loop() {
  static int currentTrack = 0; // variable to keep track of the current track/player
  static int noteIndex = 0; // variable to keep track of the current note index for the current track
  static STATE currentState = STATE_IDLE; // variable to keep track of the current state of the system
  static STATE nextState = STATE_IDLE; // variable to keep track of the next state of the system
  static unsigned long lastNoteTime = 0; // variable to keep track of the time when the last note was played

  // test
  // int readsuccess = readData(currentTrack, noteIndex++);

  switch (currentState) {
    case STATE_IDLE:
      if (nextState != STATE_IDLE) {
        currentState = nextState;
      }
      break;
    case STATE_PLAY:
      NOTE note = noteMemory[currentTrack][noteIndex]; // get the current note for the current track
      unsigned long currentTime = millis(); // get the current time
      if(note == NOTE_START) {
        lastNoteTime = currentTime;
        noteIndex++; // move to the next note
      } else if(note == NOTE_END) {
        noteIndex = 0; // reset note index for the next playthrough
        currentState = STATE_IDLE; // go back to idle after finishing the track
      }
      else if(note != NOTE_IDLE && (currentTime - lastNoteTime) >= NOTE_DURATION) { // check if it's time to play the next note
        speakerPlayNote(note, NOTE_DURATION); // function to play the note on the speaker
        lastNoteTime = currentTime; // update the last note time
        noteIndex++; // move to the next note
      }
      if (nextState == STATE_IDLE) {
        currentState = STATE_PLAY; // stay in play if nothing is pressed
      } else {
        currentState = nextState; // switch to the next state
      }
      break;
    case STATE_PAUSED:
      if (nextState == STATE_IDLE) {
        currentState = STATE_PAUSED; // stay in paused if nothing is pressed
      } else {
        currentState = nextState; // switch to the next state
      }
      break;
    case STATE_RESTART:
      noteIndex = 0; // reset note index to restart the track
      currentState = nextState; // switch to the next state
      break;
    case STATE_READ:
      noteIndex = 0; // reset note index to start reading from the beginning of the track
      int readsuccess = readData(currentTrack, noteIndex); // function to read data from the sensors and store it in the note memory
      if (readsuccess) {
        noteIndex++; // move to the next note index for the next read
      }

      if(nextState == STATE_IDLE) {
        currentState = STATE_READ; // stay in read if nothing is pressed
      } else {
        currentState = nextState; // switch to the next state
      }
      break;
    case STATE_SET_TRACK:
      currentTrack = (currentTrack + 1) % MAX_TRACKS; // cycle through tracks
      Serial.print("Current Track: ");
      Serial.println(currentTrack);
      if (nextState != STATE_SET_TRACK) {
        currentState = nextState;
      } else {
        currentState = STATE_IDLE; // prevent staying in SET_TRACK state if the button is held down
      }
      break;
    default:
      currentState = STATE_IDLE; // default to idle state
  }

  nextState = btnOutput(); // function to read button states and determine the next state

}

