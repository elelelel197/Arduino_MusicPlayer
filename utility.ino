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