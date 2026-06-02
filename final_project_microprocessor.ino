#include <DIYables_4Digit7Segment_TM1637.h>

#define clkSensPin 2  // sensor pin number for clock in paper code

#define codeSensPin 4  // sensor pin number for note code in paper code

#define mtr1fwPin 8  // motor 1 forward pin
#define mtr1bwPin 9  // motor 1 backward pin
#define mtr1pwmPin 10 // motor 1 PWM pin

#define mtr2fwPin 6 // motor 2 forward pin
#define mtr2bwPin 7 // motor 2 backward pin
#define mtr2pwmPin 5 // motor 2 PWM pin

#define spkrPin 3 // speaker pin

#define sevsegClkPin 11 // 7-segment display clock pin
#define sevsegDioPin 12 // 7-segment display data pin

#define codeWordLength 6 // number of bits in the note code word

enum NOTE {
  IDLE, START, END, 
  NOTE_A2, NOTE_A2s, NOTE_B2, NOTE_C3, NOTE_C3s, NOTE_D3, NOTE_D3s, NOTE_E3, NOTE_F3, NOTE_F3s, NOTE_G3, NOTE_G3s,
  NOTE_A3, NOTE_A3s, NOTE_B3, NOTE_C4, NOTE_C4s, NOTE_D4, NOTE_D4s, NOTE_E4, NOTE_F4, NOTE_F4s, NOTE_G4, NOTE_G4s,
  NOTE_A4, NOTE_A4s, NOTE_B4, NOTE_C5, NOTE_C5s
};

void setup() {
  Serial.begin(9600);       
  pinMode(clkSensPin, INPUT); 
  pinMode(codeSensPin, INPUT); 
}

void loop() {
  readData();
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

