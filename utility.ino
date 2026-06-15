// 4-sample debounce: state changes only after 0b0000 -> 0 or 0b1111 -> 1
bool debounce(uint8_t input, uint8_t &buffer, bool &debouncedOut) {
  const uint8_t WINDOW_MASK = 0x0F; // keep last 2 samples
  // ensure input is 0 or 1
  input = (input ? 1 : 0);

  buffer = (buffer << 1) | input;
  buffer &= WINDOW_MASK;

  // for debugging: print the buffer in binary
  // Serial.print("Buffer: ");
  // Serial.println(buffer, HEX);

  // only change output when buffer is all zeros or all ones
  if (buffer == 0x00) {
    debouncedOut = false;
  } else if (buffer == WINDOW_MASK) {
    debouncedOut = true;
  }
  return debouncedOut;
}

bool onepulse(bool input, bool &lastInput){
  bool pulseOut = (lastInput == HIGH) && (input == LOW);
  lastInput = input;
  return pulseOut;
}