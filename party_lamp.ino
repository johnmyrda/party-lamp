#include <Bounce2.h>
#include <FastLED.h>
#include <EEPROM.h>

// Define variable for FastLED usage
#define NUM_LEDS 60
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
#define DATA_PIN A1

//array for FastLED to use
CRGB leds[NUM_LEDS];

bool partyTime = true;
#define PARTY_STATE_EEPROM_ADDRESS 0
byte partyState = 0;
#define KELVIN_STATE_EEPROM_ADDRESS 1
byte kelvinState = 0;
const byte brightnessPotPin = A0;
const byte buttonParty = 16;
const byte ledButtonParty = 14;
const byte buttonNextPin = 15;
const byte onOffSwitchPin = 19;

#define NUM_COLORTEMPS 3
CRGB colorTemps[NUM_COLORTEMPS] = {Candle, Tungsten40W, CarbonArc};

#define NUM_PATTERNS 6
typedef void (* GenericFP)();
GenericFP patterns[NUM_PATTERNS] = {&rainbowPattern,
                                    &gradientPattern,
                                    &usaPattern,
                                    &sweetChristmas,
                                    &emergencySOS,
                                    &testPattern
                                   };

//variables for frame counting (pattern timing) 
#define FRAME_INTERVAL_DEFAULT 150
unsigned long frame = 0;
unsigned long now = millis();
int frame_interval = FRAME_INTERVAL_DEFAULT;                                 

Bounce buttonNext = Bounce();

void setup()
{
  if(EEPROM.read(KELVIN_STATE_EEPROM_ADDRESS) <= NUM_COLORTEMPS){
    kelvinState = EEPROM.read(KELVIN_STATE_EEPROM_ADDRESS);
  }
  if(EEPROM.read(PARTY_STATE_EEPROM_ADDRESS) <= NUM_PATTERNS){
    partyState = EEPROM.read(PARTY_STATE_EEPROM_ADDRESS);
  }
  pinMode(buttonNextPin, INPUT_PULLUP);
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
  int prevKelvinState = kelvinState;
  kelvinState = getNextIndex(kelvinState, NUM_COLORTEMPS, KELVIN_STATE_EEPROM_ADDRESS);
  if(kelvinState != prevKelvinState || partyTime){
    partyTime = false;
    fill_solid(leds, NUM_LEDS, colorTemps[kelvinState]);
  }
}

//Returns the value of the brightness dial between 0-1023
void updateBrightness() {
  int brightness_reading = analogRead(brightnessPotPin);
  //Serial.print("brightness_reading: " + String(brightness_reading));
  brightness_reading = sqrt(brightness_reading); //try to delogify
  brightness_reading = brightness_reading >> 2; // divide by 4
  brightness_reading = brightness_reading << 5; // multiply by 32
  brightness_reading += 31;
  //Serial.println(" final: " + String(brightness_reading));
  FastLED.setBrightness(brightness_reading);
}

int getNextIndex(const int current_index, int num_items, int eeprom_address) {
  buttonNext.update();
  int return_value = current_index;

  if (buttonNext.fell()) {
    return_value = (current_index + 1) % num_items;
    //Only write the state to eeprom when a button press occurs
    EEPROM.write(eeprom_address, return_value);
  }
  return return_value;
}


void partyMode() {
  partyTime = true;
  partyState = getNextIndex(partyState, NUM_PATTERNS-1, PARTY_STATE_EEPROM_ADDRESS);
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
  genericPattern(rgb, 3);
}

void genericPattern(CRGB * pattern, int length){
  for (int i = 0; i < NUM_LEDS; i++){
      int index = (frame + i) % length;
      leds[i] = CRGB(pattern[index]);
    }
  }

void sweetChristmas(){
  CRGB christmas[6] = {CRGB::Red, CRGB::Red, CRGB::Red,
                  CRGB::Green, CRGB::Green, CRGB::Green,};
  genericPattern(christmas, 6);
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


