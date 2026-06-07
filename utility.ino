bool debounce(bool input, uint8_t buffer){
  buffer <<= 1;
  buffer |= input;
  bool debOut = input;
  if(buffer == 0)
    debOut = 0;
  else if(buffer == 255)
    debOut = 1;
  return debOut;
}

bool onepulse(bool input){
  static bool lastInput = LOW;
  bool pulseOut = (lastInput == LOW) && (input == HIGH);
  lastInput = input;
  return pulseOut;
}