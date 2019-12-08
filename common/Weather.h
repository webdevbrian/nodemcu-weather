/* Basic Single RGB LED control procedures */
const int RledPin = 14;
const int GledPin = 12;
const int BledPin = 13;

void TurnOn(int pin) {
  analogWrite(pin, 255);
}

void TurnOff(int pin) {
  analogWrite(pin, 0);
}

int FadeOn(int pin) {
  int fadeValue;
  for(fadeValue = 0 ; fadeValue <= 255; fadeValue += 1) {
    analogWrite(pin, fadeValue);
    // wait to see the dimming effect
    delay(2);
  }

  analogWrite(pin, 255);
  return(fadeValue);
}

int FadeOff(int pin) {
  int fadeValue;
  for(fadeValue = 255 ; fadeValue >= 0; fadeValue -= 1) {
    analogWrite(pin, fadeValue);
    // wait to see the dimming effect
    delay(2);
  }

  analogWrite(pin, 0);
  return(fadeValue);
}

int Pulse(int pin, int iterationCount, int speedMs) {
  int fadeValue;
  int i = 1;
  while (i < iterationCount) {
    for(fadeValue = 255 ; fadeValue >= 100; fadeValue -= 1) {
      analogWrite(pin, fadeValue);
      // wait to see the dimming effect
      delay(speedMs);
    }

    for(fadeValue = 100 ; fadeValue <= 255; fadeValue += 1) {
      analogWrite(pin, fadeValue);
      // wait to see the dimming effect
      delay(speedMs);
    }

    i++;
  }

  analogWrite(pin, 255);
  return(fadeValue);
}

void changeColorByHex(char* hexString) {
  TurnOff(RledPin);
  TurnOff(GledPin);
  TurnOff(BledPin);

  // to RGB
  long number = (long) strtol( &hexString[0], NULL, 16);
  int r = number >> 16;
  int g = number >> 8 & 0xFF;
  int b = number & 0xFF;

  analogWrite(RledPin, r);
  analogWrite(GledPin, g);
  analogWrite(BledPin, b);
}