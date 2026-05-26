int clkSensPin = 2;  // sensor pin number for clock in paper code
int codeSensPin = 4;  // sensor pin number for note code in paper code
int codeWordLength = 6; // number of bits in the note code word

enum Note {
  IDLE, START, END, 
  A2, A2s, B2, C3, C3s, D3, D3s, E3, F3, F3s, G3, G3s,
  A3, A3s, B3, C4, C4s, D4, D4s, E4, F4, F4s, G4, G4s,
  A4, A4s, B4, C5, C5s, D5, D5s, E5, F5, F5s, G5
};

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
        Note note = static_cast<Note>(dataBuffer);
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

int NoteToFrequency(Note note) {
  switch (note) {
    case A2: return 110;
    case A2s: return 116;
    case B2: return 123;
    case C3: return 130;
    case C3s: return 138;
    case D3: return 146;
    case D3s: return 155;
    case E3: return 164;
    case F3: return 174;
    case F3s: return 185;
    case G3: return 196;
    case G3s: return 207;
    case A3: return 220;
    case A3s: return 233;
    case B3: return 246;
    case C4: return 261;
    case C4s: return 277;
    case D4: return 293;
    case D4s: return 311;
    case E4: return 329;
    case F4: return 349;
    case F4s: return 370;
    case G4: return 392;
    case G4s: return 415;
    case A4: return 440;
    case A4s: return 466;
    case B4: return 493;
    case C5: return 523;
    case C5s: return 554;
    case D5: return 587;
    case D5s: return 622;
    case E5: return 659;
    case F5: return 698;
    case F5s: return 740;
    case G5: return 784; 
    default: return -1; 
  }
}

