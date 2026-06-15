uint8_t readData(uint8_t currentTrack, uint8_t noteIndex) {
  uint8_t clkSensIn = digitalRead(clkSensPin);
  uint8_t codeSensIn = digitalRead(codeSensPin);

  // For debugging: print the raw sensor inputs
  // Serial.print("Raw Clock: ");
  // Serial.println(clkSensIn);

  static uint8_t clkSensBuffer = 0;
  static uint8_t codeSensBuffer = 0;

  static bool lastClkSens = 0;
  static bool lastCodeSens = 0;

  uint8_t clkDebounced = debounce(clkSensIn, clkSensBuffer, lastClkSens);
  uint8_t codeDebounced = debounce(codeSensIn, codeSensBuffer, lastCodeSens);

  // For debugging: print the debounced sensor values
  // Serial.print("Debounced Clock: ");
  // Serial.println(clkDebounced);

  static uint8_t dataBuffer = 0;
  static uint8_t wordCounter = 0;
  static uint8_t startFlag = 0;

  if (sensClkPosEdge(clkDebounced)) {
    dataBuffer <<= 1;
    dataBuffer |= codeDebounced;
    dataBuffer &= 0x3F; // Keep only the last 6 bits
    // Serial.print("Data buffer: ");
    // Serial.println(dataBuffer, BIN);
    wordCounter++;
    if (parityCheck(dataBuffer)) {
      // Once start code is detected, we can process the note code
      uint8_t noteCode = dataBuffer;
      noteCode >>= 1; // Shift right to get bits 1..7 as the note code
      Serial.print("Note code: ");
      Serial.println(noteCode, BIN);
      // Serial.print("wordCounter: ");
      // Serial.println(wordCounter);
      if (noteCode == NOTE_START) {
        Serial.println("Received START note code.");
        startFlag = 1;
        wordCounter = 0;
        noteMemory[currentTrack][noteIndex] = NOTE_START; // Store the START code in the note memory
      }
      else if (startFlag && (wordCounter == CODE_WORD_LENGTH)) {
        NOTE note = static_cast<NOTE>(noteCode); // Convert the note code to the NOTE enum
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
    if (wordCounter >= CODE_WORD_LENGTH) {
       wordCounter = 0; // Reset word counter for the next code word
    }
  }

  return 0; // Indicate unsuccessful read
}

bool sensClkPosEdge(bool clkdebounced) {
  static bool lastClk = LOW;
  bool posEdge = (lastClk == LOW) && (clkdebounced == HIGH);
  lastClk = clkdebounced;
  return posEdge;
}

uint8_t parityCheck(uint8_t data) {
  uint8_t parity = 1;
  for (uint8_t i = 0; i < 8; i++) {
    parity ^= (data >> i) & 1;
  }
  return parity;
}