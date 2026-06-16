// 8-sample debounce: state changes only after 0b000000 -> 0 or 0b111111 -> 1
bool debounce(uint8_t input, uint8_t &buffer, bool &debouncedOut) {
  const uint8_t WINDOW_MASK = 0xFF; // keep last 8 samples
  // ensure input is 0 or 1
  input = (input ? 1 : 0);

  buffer = (buffer << 1) | input;
  buffer &= WINDOW_MASK;

  // only change output when buffer is all zeros or all ones
  if (buffer == 0x00) {
    debouncedOut = false;
  } else if (buffer == WINDOW_MASK) {
    debouncedOut = true;
  }
  return debouncedOut;
}

// onepulse: detect a transition (falling edge for INPUT_PULLUP buttons)
// lastInput must be preserved between calls and should be initialized to the pin's stable state
bool onepulse(bool input, bool &lastInput){
  // input and lastInput are boolean (true/high, false/low)
  bool pulseOut = (lastInput == true) && (input == false); // HIGH -> LOW (button press with INPUT_PULLUP)
  lastInput = input;
  return pulseOut;
}
