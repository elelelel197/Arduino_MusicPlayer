#include <Arduino.h>
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
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

#line 46 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void setup();
#line 71 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void loop();
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\button.ino"
STATE btnOutput();
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
int readData(int currentTrack, int noteIndex);
#line 52 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
int sensClkPosEdge(int clkdebounced);
#line 59 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
int parityCheck(byte data);
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\speaker.ino"
void speakerPlayNote(NOTE note, int duration);
#line 11 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\speaker.ino"
int NoteToFrequency(NOTE note);
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\utility.ino"
int debounce(int input, byte buffer);
#line 46 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
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


#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\button.ino"
STATE btnOutput(){
    static int debouncebtnPlay = 0; // variable for debouncing the play button
    static int debouncebtnPause = 0; // variable for debouncing the pause button
    static int debouncebtnRestart = 0; // variable for debouncing the restart button
    static int debouncebtnRead = 0; // variable for debouncing the read button
    static int debouncebtnSetTrack = 0; // variable for debouncing the set track

    static byte btnPlayBuffer = 0; // buffer variable for the play button state
    static byte btnPauseBuffer = 0; // buffer variable for the pause button state
    static byte btnRestartBuffer = 0; // buffer variable for the restart button state
    static byte btnReadBuffer = 0; // buffer variable for the read button state
    static byte btnSetTrackBuffer = 0; // buffer variable for the set track button state

    debouncebtnPlay = debounce(digitalRead(btnPlayPin), debouncebtnPlay);
    debouncebtnPause = debounce(digitalRead(btnPausePin), debouncebtnPause);
    debouncebtnRestart = debounce(digitalRead(btnRestartPin), debouncebtnRestart);
    debouncebtnRead = debounce(digitalRead(btnReadPin), debouncebtnRead);
    debouncebtnSetTrack = debounce(digitalRead(btnSetTrackPin), debouncebtnSetTrack); 

    if (debouncebtnPlay) {
        return STATE_PLAY;
    } else if (debouncebtnPause) {
        return STATE_PAUSED;
    } else if (debouncebtnRestart) {
        return STATE_RESTART;
    } else if (debouncebtnRead) {
        return STATE_READ;
    } else if (debouncebtnSetTrack) {
        return STATE_SET_TRACK;
    } else {
        return STATE_IDLE;
    }
}
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\data_decoder.ino"
int readData(int currentTrack, int noteIndex) {
  static int clkSensIn = digitalRead(clkSensPin);
  static int codeSensIn = digitalRead(codeSensPin);

  static byte clkSensBuffer = 0;
  static byte codeSensBuffer = 0;

  static int clkDebounced = debounce(clkSensIn, clkSensBuffer);
  static int codeDebounced = debounce(codeSensIn, codeSensBuffer);

  static byte dataBuffer = 0;
  static int wordCounter = 0;
  static int startFlag = 0;

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
          Serial.print("Received note: ");
          Serial.print(note);
        } 
        noteMemory[currentTrack][noteIndex] = note; // Store the note in the note memory
      }
      return 1; // Indicate successful read
    }
  }

  return 0; // Indicate unsuccessful read
}

int sensClkPosEdge(int clkdebounced) {
  static int lastClk = LOW;
  int posEdge = (lastClk == LOW) && (clkdebounced == HIGH);
  lastClk = clkdebounced;
  return posEdge;
}

int parityCheck(byte data) {
  int parity = 0;
  for (int i = 0; i < 8; i++) {
    parity ^= (data >> i) & 1;
  }
  return parity;
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
int debounce(int input, byte buffer){
  buffer <<= 1;
  buffer |= input;
  int debOut = input;
  if(buffer == 0)
    debOut = 0;
  else if(buffer == 255)
    debOut = 1;
  return debOut;
}
