uint8_t readData(uint8_t currentTrack, uint8_t noteIndex) {
  static uint8_t clkSensIn = digitalRead(clkSensPin);
  static uint8_t codeSensIn = digitalRead(codeSensPin);

  static uint8_t clkSensBuffer = 0;
  static uint8_t codeSensBuffer = 0;

  static uint8_t clkDebounced = debounce(clkSensIn, clkSensBuffer);
  static uint8_t codeDebounced = debounce(codeSensIn, codeSensBuffer);

  static uint8_t dataBuffer = 0;
  static uint8_t wordCounter = 0;
  static uint8_t startFlag = 0;

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
          Serial.println("Received note: ");
          Serial.println(note);
        } 
        noteMemory[currentTrack][noteIndex] = note; // Store the note in the note memory
      }
      return 1; // Indicate successful read
    }
  }

  return 0; // Indicate unsuccessful read
}

uint8_t sensClkPosEdge(uint8_t clkdebounced) {
  static uint8_t lastClk = LOW;
  uint8_t posEdge = (lastClk == LOW) && (clkdebounced == HIGH);
  lastClk = clkdebounced;
  return posEdge;
}

uint8_t parityCheck(uint8_t data) {
  uint8_t parity = 0;
  for (uint8_t i = 0; i < 8; i++) {
    parity ^= (data >> i) & 1;
  }
  return parity;
}