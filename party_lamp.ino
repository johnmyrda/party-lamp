//#include <RunningAverage.h>

#include <Bounce2.h>
#include <Kelvin2RGB.h>

/* LedStripRainbow: Example Arduino sketch that shows
   how to make a moving rainbow pattern on an
   Addressable RGB LED Strip from Pololu.

   To use this, you will need to plug an Addressable RGB LED
   strip from Pololu into pin 12.  After uploading the sketch,
   you should see a moving rainbow.
*/

#include <PololuLedStrip.h>

// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<A1> ledStrip;

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

// Create a buffer for holding the colors (3 bytes per color).
#define LED_COUNT 60
rgb_color colors[LED_COUNT];

int partyState = 0;
int kelvinState = 0;
const int brightnessPotPin = A0;
const int buttonParty = 16;
const int ledButtonParty = 14;
const int buttonNextPin = 15;
const int onOffSwitchPin = 19;

int colorTemps[4] = {1700, 2700, 3500, 5000};

#define NUM_PATTERNS 4
void (*patterns[NUM_PATTERNS]) ();

Bounce buttonNext = Bounce();

//RunningAverage brightnessRA(5);

void setup()
{
  //Serial.begin(9600);
  patterns[0] = rainbowPattern;
  patterns[1] = gradientPattern;
  patterns[2] = usaPattern;
  patterns[3] = emergencySOS;
  patterns[4] = testPattern;

  for (int i = 0; i <= 10; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  pinMode(15, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
  //pinMode(nextButtonPin, INPUT_PULLUP);
  buttonNext.attach(buttonNextPin);
  pinMode(onOffSwitchPin, INPUT_PULLUP);
  pinMode(buttonParty, INPUT_PULLUP);
  pinMode(ledButtonParty, OUTPUT);
}

// Converts a color from HSV to RGB.
// h is hue, as a number between 0 and 360.
// s is the saturation, as a number between 0 and 255.
// v is the value, as a number between 0 and 255.
rgb_color hsvToRgb(uint16_t h, uint8_t s, uint8_t v)
{
  uint8_t f = (h % 60) * 255 / 60;
  uint8_t p = (255 - s) * (uint16_t)v / 255;
  uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
  uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
  uint8_t r = 0, g = 0, b = 0;
  switch ((h / 60) % 6) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }
  return (rgb_color) {
    r, g, b
  };
}

rgb_color kelvinToRgb(uint16_t temperature, uint8_t brightness) {
  Kelvin2RGB kelvin(temperature, brightness);
  return (rgb_color) {
    kelvin.Red, kelvin.Green, kelvin.Blue
  };
}

void kelvinMode() {
  kelvinState = getNextIndex(kelvinState, 3);

  //Kelvin2RGB uses 0-100 for brightness
  int brightness = getBrightness() / 10;
  for (byte i = 0; i < LED_COUNT; i++) {
    colors[i] = kelvinToRgb(colorTemps[kelvinState], brightness);
  }
}

//Returns the value of the brightness dial between 0-1023
int getBrightness() {
  int brightness_reading = analogRead(brightnessPotPin);
  brightness_reading = brightness_reading >> 7; // divide by 128
  brightness_reading = brightness_reading << 5; // multiply by 32
  return brightness_reading;
  //  brightnessRA.addValue(brightness);
//  Serial.println(brightness);
//  return brightnessRA.getFastAverage();
}

int getNextIndex(const int current_index, int num_items) {
  buttonNext.update();
  int return_value = current_index;

  if (buttonNext.fell()) {
    return_value = (current_index + 1) % num_items;
  }
  return return_value;
}


void partyMode() {
  partyState = getNextIndex(0, NUM_PATTERNS-1);

  //hsvToRgb uses 0-255 for brightness
  //int brightness = getBrightness() / 4;
  (*patterns[partyState]) ();
}

void loop()
{
  if (digitalRead(buttonParty) == LOW) {
    digitalWrite(ledButtonParty, HIGH);
    partyMode();
  } else {
    digitalWrite(ledButtonParty, LOW);
    kelvinMode();
  }

  // Write the colors to the LED strip.
  ledStrip.write(colors, LED_COUNT);

  delay(10);
}

void rainbowPattern() {
  uint16_t time = millis() >> 2;
  for (uint16_t i = 0; i < LED_COUNT; i++)
  {
    byte x = (time >> 2) - (i << 3);
    colors[i] = hsvToRgb((uint32_t)x * 359 / 256, 255, getBrightness() / 4);
  }
}

void gradientPattern() {
  // Update the colors.
  byte time = millis() >> 2;
  for (uint16_t i = 0; i < LED_COUNT; i++)
  {
    byte x = time - 8 * i;
    colors[i] = setRGB(RGB(x, 255 - x, x));
  }
}

void testPattern() {
  rgb_color rgb[3] = {RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255)};
  for (byte i = 0; i < LED_COUNT; i++) {
    colors[i] = setRGB(rgb[i % 3]);
  }
}

rgb_color RGB(char red, char green, char blue) {
  rgb_color color;
  color.red = red;
  color.green = green;
  color.blue = blue;
  return color;
}

void usaPattern() {
  long time = millis() >> 8;
  rgb_color red = RGB(255, 0, 0);
  rgb_color white = RGB(255, 255, 255);
  rgb_color blue = RGB(0, 0, 255);
  rgb_color usa[3] = {red, white, blue};
  for (byte i = 0; i < LED_COUNT; i++) {
    byte usa_index = (time + i ) % 3;
    colors[i] = setRGB(usa[usa_index]);
  }
}

void emergencySOS() {
  int num_intervals = 34;
  bool SOS[34] =    {1, 0, 1, 0, 1,  0, 0, 0,    1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1,   0, 0, 0,    1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0};
  int interval_time = 300; //in milliseconds
  long mod_time = interval_time * num_intervals;
  byte current_time_unit = (millis() % mod_time) / interval_time;
  //SOS in binary -             S -s_s_s     space-___ O-l_l_l                  space-___ S -s_s_s

  if (SOS[current_time_unit]) {
    for (byte i = 0; i < LED_COUNT; i++) {
      colors[i] = setRGB(RGB(255, 0, 0));
    }
  } else {
    for (byte i = 0; i < LED_COUNT; i++) {
      colors[i] = setRGB(RGB(0, 0, 0));
    }
  }
}

//brightness is represented as a number between 0 and 256
rgb_color setRGB(const rgb_color color) {
  int brightness = getBrightness() / 4;
  return (RGB(
            (color.red * brightness) / 255,
            (color.green * brightness) / 255,
            (color.blue * brightness) / 255)
         );
}

