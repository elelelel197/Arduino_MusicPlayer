uint8_t readData(bool &startFlag) {
  uint8_t clkSensIn = digitalRead(clkSensPin);
  uint8_t codeSensIn = digitalRead(codeSensPin);

  // For debugging: print the raw sensor inputs
  Serial.print("Raw Clock: ");
  Serial.println(clkSensIn);
  Serial.print("Raw Data");
  Serial.println(codeSensIn);

  static uint8_t clkSensBuffer = 0;
  static uint8_t codeSensBuffer = 0;

  static bool lastClkSens = 0;
  static bool lastCodeSens = 0;

  uint8_t clkDebounced = debounce(clkSensIn, clkSensBuffer, lastClkSens);
  uint8_t codeDebounced = debounceL(codeSensIn, codeSensBuffer, lastCodeSens);

  // For debugging: print the debounced sensor values
  // Serial.print("Debounced Clock: ");
  // Serial.println(clkDebounced);

  static uint8_t dataBuffer = 0;
  static uint8_t wordCounter = 0;
  bool readSucFlag = 0;
  uint8_t noteCode = 0;

  if (sensClkPosEdge(clkDebounced)) {
    dataBuffer <<= 1;
    dataBuffer |= codeDebounced;
    dataBuffer &= 0x3F; // Keep only the last 6 bits
    // Serial.print("Data buffer: ");
    // Serial.println(dataBuffer, BIN);
    wordCounter++;
    if (parityCheck(dataBuffer)) {
      // Once start code is detected, we can process the note code
      noteCode = dataBuffer;
      noteCode >>= 1; // Shift right to get bits 1..7 as the note code
      // Serial.print("Note code: ");
      // Serial.println(noteCode, BIN);
      // Serial.print("wordCounter: ");
      // Serial.println(wordCounter);
      if (noteCode == NOTE_START && !startFlag) {
        Serial.println("Received START note code.");
        startFlag = 1;
        readSucFlag = 1;
        wordCounter = 0;
      }
      else if (startFlag && (wordCounter == CODE_WORD_LENGTH)) {
        // Once end code is detected, we can reset the start flag
        if (noteCode == NOTE_END) {
          startFlag = 0;
          wordCounter = 0;
          Serial.println("Received END note code.");
        } else if (noteCode == NOTE_IDLE) {
          Serial.println("Received IDLE note code.");
        } else {
          Serial.print("Received note: ");
          Serial.println(noteCode);
        } 
        
        readSucFlag = 1;
      }
    }
    if (wordCounter >= CODE_WORD_LENGTH) 
      wordCounter = 0; // Reset word counter for the next code word
  }

  if (readSucFlag)
    return noteCode;
  else
    return 0xFF;
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

// 3-sample debounce: state changes only after 0b000 -> 0 or 0b111 -> 1
bool debounceL(uint8_t inputL, uint8_t &bufferL, bool &debouncedOutL) {
  const uint8_t WINDOW_MASK = 0x0F; // keep last 3 samples
  inputL = (inputL ? 1 : 0);

  bufferL = (bufferL << 1) | inputL;
  bufferL &= WINDOW_MASK;

  if (bufferL == 0x00) {
    debouncedOutL = false;
  } else if (bufferL == WINDOW_MASK) {
    debouncedOutL = true;
  }
  return debouncedOutL;
}