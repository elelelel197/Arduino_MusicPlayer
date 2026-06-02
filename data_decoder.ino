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
    if (wordCounter >= codeWordLength) {
       wordCounter = 0; // Reset word counter for the next code word
    }
    if (parityCheck(dataBuffer)) {
      // Once start code is detected, we can process the note code
      if (dataBuffer == START) {
        startFlag = 1; 
        wordCounter = 0;
        noteMemory[currentTrack][noteIndex] = START; // Store the START code in the note memory
      } 
      else if (startFlag && wordCounter == CODE_WORD_LENGTH - 1) {
        NOTE note = static_cast<NOTE>(dataBuffer);
        // Once end code is detected, we can reset the start flag
        if (note == END) {
          startFlag = 0;
          wordCounter = 0;
          Serial.println("Received END note code.");
        } else if (note == IDLE) {
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