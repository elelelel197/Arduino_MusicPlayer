#include <Arduino.h>
#line 1 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
int clkSensPin = 2;  // sensor pin number for clock in paper code
int codeSensPin = 4;  // sensor pin number for note code in paper code
int codeWordLength = 6; // number of bits in the note code word

enum NOTE {
  IDLE, START, END, 
  NOTE_A2, NOTE_A2s, NOTE_B2, NOTE_C3, NOTE_C3s, NOTE_D3, NOTE_D3s, NOTE_E3, NOTE_F3, NOTE_F3s, NOTE_G3, NOTE_G3s,
  NOTE_A3, NOTE_A3s, NOTE_B3, NOTE_C4, NOTE_C4s, NOTE_D4, NOTE_D4s, NOTE_E4, NOTE_F4, NOTE_F4s, NOTE_G4, NOTE_G4s,
  NOTE_A4, NOTE_A4s, NOTE_B4, NOTE_C5, NOTE_C5s, NOTE_D5, NOTE_D5s, NOTE_E5, NOTE_F5, NOTE_F5s, NOTE_G5
};

#line 12 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void setup();
#line 18 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void loop();
#line 22 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void readData();
#line 73 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
int debounce(int input, byte buffer);
#line 84 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
int sensClkPosEdge(int clkdebounced);
#line 91 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
int parityCheck(byte data);
#line 99 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
int NoteToFrequency(NOTE note);
#line 12 "C:\\Users\\Administrator\\Desktop\\final_project_microprocessor\\final_project_microprocessor.ino"
void setup() {
  Serial.begin(9600);       
  pinMode(clkSensPin, INPUT); 
  pinMode(codeSensPin, INPUT); 
}

void loop() {
  readData();
}

void readData(){
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
    if (wordCounter >= codeWordLength) {
       wordCounter = 0; // Reset word counter for the next code word
    }
    if (parityCheck(dataBuffer)) {
      // Once start code is detected, we can process the note code
      if (dataBuffer == START) {
        startFlag = 1; 
        wordCounter = 0;
      } 
      // Once end code is detected, we can reset the start flag
      else if (dataBuffer == END) {
        if (startFlag) {
          Serial.println("End of note code received.");
          startFlag = 0;
        }
        wordCounter = 0;
      }
      else if (startFlag && wordCounter == codeWordLength - 1) {
        NOTE note = static_cast<NOTE>(dataBuffer);
        int frequency = NoteToFrequency(note);
        if (frequency != -1) {
          Serial.print("Received note: ");
          Serial.print(note);
          Serial.print(" with frequency: ");
          Serial.println(frequency);
        } else {
          Serial.println("Received invalid note code.");
        }
      } 
    }
  }
}

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
    case NOTE_D5: return 587;
    case NOTE_D5s: return 622;
    case NOTE_E5: return 659;
    case NOTE_F5: return 698;
    case NOTE_F5s: return 740;
    case NOTE_G5: return 784;
    default: return -1;
  }
}


