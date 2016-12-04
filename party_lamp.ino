#include <Bounce2.h>
#include <FastLED.h>

// Create an ledStrip object and specify the pin it will use.
#define DATA_PIN A1

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

// Create a buffer for holding the colors (3 bytes per color).
#define NUM_LEDS 60
#define CHIPSET WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

int partyState = 0;
int kelvinState = 0;
const int brightnessPotPin = A0;
const int buttonParty = 16;
const int ledButtonParty = 14;
const int buttonNextPin = 15;
const int onOffSwitchPin = 19;

#define NUM_COLORTEMPS 3
CRGB colorTemps[NUM_COLORTEMPS] = {Candle, Tungsten40W, CarbonArc};

#define NUM_PATTERNS 5
typedef void (* GenericFP)();
GenericFP patterns[NUM_PATTERNS] = {&rainbowPattern,
                                    &gradientPattern,
                                    &usaPattern,
                                    &emergencySOS,
                                    &testPattern
                                   };
#define FRAME_INTERVAL_DEFAULT 150
unsigned long frame = 0;
unsigned long now = millis();
int frame_interval = FRAME_INTERVAL_DEFAULT;                                 

Bounce buttonNext = Bounce();

void setup()
{
  
  pinMode(A1, OUTPUT);
  digitalWrite(A1, LOW);
  for (int i = 0; i <= 10; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  pinMode(15, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
  buttonNext.attach(buttonNextPin);
  pinMode(onOffSwitchPin, INPUT_PULLUP);
  pinMode(buttonParty, INPUT_PULLUP);
  pinMode(ledButtonParty, OUTPUT);
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS); 
  FastLED.clear();
  FastLED.show();
  // limit my draw to 1A at 5v of power draw
  FastLED.setMaxPowerInVoltsAndMilliamps(5,1000);   
  //Serial.begin(9600);  
}

void kelvinMode() {
  kelvinState = getNextIndex(kelvinState, NUM_COLORTEMPS);
  //Serial.println("kelvinState: " + String(kelvinState));
  fill_solid(leds, NUM_LEDS, colorTemps[kelvinState]);
}

//Returns the value of the brightness dial between 0-1023
void updateBrightness() {
  int brightness_reading = analogRead(brightnessPotPin);
  //Serial.print("original: " + String(brightness_reading) + " quantized: ");
  brightness_reading = brightness_reading >> 7; // divide by 128
  brightness_reading = brightness_reading << 5; // multiply by 32
  brightness_reading += 31;
  //Serial.println(brightness_reading);
  FastLED.setBrightness(brightness_reading);
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
  partyState = getNextIndex(partyState, NUM_PATTERNS-1);
  now = millis();

  unsigned long newFrame = now / frame_interval;
  if (newFrame != frame) {
    frame = newFrame;
    patterns[partyState]();
  }  
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
  updateBrightness();
  FastLED.show();
}

void rainbowPattern() {
  int rainbow_density = 4; //lower==less dense
  fill_rainbow(leds, NUM_LEDS, frame * rainbow_density);
}

void gradientPattern() {
  byte time = millis() >> 2; 
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    byte x = time - 8 * i;
    leds[i] = CRGB(x, 255 - x, x);
  }
}

void testPattern() {
  CRGB rgb[3] = {CRGB::Red, CRGB::Green, CRGB::Blue};
  for (byte i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(rgb[i % 3]);
  }
}

void genericPattern(CRGB * pattern, int length){
  for (int i = 0; i < NUM_LEDS; i++){
      int index = (frame + i) % length;
      leds[i] = CRGB(pattern[index]);
    }
  }

void usaPattern() {
  CRGB usa[12] = {CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
                  CRGB::White, CRGB::White, CRGB::White, CRGB::White, 
                  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue};
  genericPattern(usa, 12);
}

void emergencySOS() {
  int num_intervals = 34;
  bool SOS[34] =    {1, 0, 1, 0, 1,  0, 0, 0,    1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1,   0, 0, 0,    1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0};
  int interval_time = 300; //in milliseconds
  long mod_time = interval_time * num_intervals;
  byte current_time_unit = (millis() % mod_time) / interval_time;
  //SOS in binary -             S -s_s_s     space-___ O-l_l_l                  space-___ S -s_s_s
  if (SOS[current_time_unit]) {
    fill_solid(leds, NUM_LEDS, CRGB::Red);
  } else {
    FastLED.clear();
  }
}


