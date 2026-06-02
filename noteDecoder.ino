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