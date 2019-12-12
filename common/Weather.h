/* LED control procedures */
// TODO: Convert Sun and Cloud LEDs to neopixels to save on GPIO pins
const int SunRledPin = 14; //D5
const int SunGledPin = 12; //D6
const int SunBledPin = 13; //D7
const int CloudRledPin = 5; //D1
const int CloudGledPin = 4; //D2
const int CloudBledPin = 0; //D3

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

double MyNormalize(double v) {
  v = v - floor(v);
  if (v < 0)
  v = v + 1;
  return v;
}

// calculate the current phase of the moon
float GetPhase(int nYear, int nMonth, int nDay) {
  float phase;
  double AG, IP;
  long YY, MM, K1, K2, K3, JD;
  YY = nYear - floor((12 - nMonth) / 10);
  MM = nMonth + 9;

  if (MM >= 12) {
    MM = MM - 12;
  }

  K1 = floor(365.25 * (YY + 4712));
  K2 = floor(30.6 * MM + 0.5);
  K3 = floor(floor((YY / 100) + 49) * 0.75) - 38;
  JD = K1 + K2 + nDay + 59;

  if (JD > 2299160) {
    JD = JD - K3;
  }

  IP = MyNormalize((JD - 2451550.1) / 29.530588853);
  AG = IP*29.53;
  phase = 0;

  if ((AG < 1.84566) && (phase == 0)) {
    phase = 0; //new; 0% illuminated
  }

  if ((AG < 5.53699) && (phase == 0)) {
    phase = .25; //Waxing crescent; 25% illuminated
  }

  if ((AG < 9.922831) && (phase == 0)) {
    phase = .50; //First quarter; 50% illuminated
  }

  if ((AG < 12.91963) && (phase == 0)) {
    phase = .75; //Waxing gibbous; 75% illuminated
  }

  if ((AG < 16.61096) && (phase == 0)) {
    phase = 1; //Full; 100% illuminated
  }

  if ((AG < 20.30228) && (phase == 0)) {
    phase = .75; //Waning gibbous; 75% illuminated
  }

  if ((AG < 23.99361) && (phase == 0)) {
    phase = .50; //Last quarter; 50% illuminated
  }

  if ((AG < 27.68493) && (phase == 0)) {
    phase = .25; //Waning crescent; 25% illuminated
  }

  if (phase == 0) {
    phase = 0; //default to new; 0% illuminated
  }

  return phase;
}