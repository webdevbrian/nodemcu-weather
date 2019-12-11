/* LED control procedures */
const int SunRledPin = 14;
const int SunGledPin = 12;
const int SunBledPin = 13;
const int CloudRledPin = 5;
const int CloudGledPin = 4;
const int CloudBledPin = 0;

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

void changeColorByHex(String RGBLED, char* hexString) {

  if(RGBLED == "Sun") {
    TurnOff(SunRledPin);
    TurnOff(SunGledPin);
    TurnOff(SunBledPin);
  }

  if(RGBLED == "Cloud") {
    TurnOff(CloudRledPin);
    TurnOff(CloudGledPin);
    TurnOff(CloudBledPin);
  }

  // to RGB
  long number = (long) strtol( &hexString[0], NULL, 16);
  int r = number >> 16;
  int g = number >> 8 & 0xFF;
  int b = number & 0xFF;

  if(RGBLED == "Sun") {
    analogWrite(SunRledPin, r);
    analogWrite(SunGledPin, g);
    analogWrite(SunBledPin, b);
  }

  if(RGBLED == "Cloud") {
    analogWrite(CloudRledPin, r);
    analogWrite(CloudGledPin, g);
    analogWrite(CloudBledPin, b);
  }
}