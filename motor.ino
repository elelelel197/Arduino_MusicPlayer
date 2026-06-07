void driveMtr1(bool direction, uint8_t speed) {
  if (direction) {
    digitalWrite(mtr1fwPin, HIGH);
    digitalWrite(mtr1bwPin, LOW);
  } else {
    digitalWrite(mtr1fwPin, LOW);
    digitalWrite(mtr1bwPin, HIGH);
  }
  analogWrite(mtr1pwmPin, speed);
}

void driveMtr2(bool direction, uint8_t speed) {
  if (direction) {
    digitalWrite(mtr2fwPin, HIGH);
    digitalWrite(mtr2bwPin, LOW);
  } else {
    digitalWrite(mtr2fwPin, LOW);
    digitalWrite(mtr2bwPin, HIGH);
  }
  analogWrite(mtr2pwmPin, speed);
}

void stopMtr1() {
  // coast: disable PWM (write 0) and set inputs low
  analogWrite(mtr1pwmPin, 0);
  digitalWrite(mtr1fwPin, LOW);
  digitalWrite(mtr1bwPin, LOW);
}

void stopMtr2() {
  analogWrite(mtr2pwmPin, 0);
  digitalWrite(mtr2fwPin, LOW);
  digitalWrite(mtr2bwPin, LOW);
}