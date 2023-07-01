#include <Arduino.h>
#include "FastLED.h"
#include "RunningAverage.h"

// #define NUM_LEDS      120
#define NUM_LEDS      41
#define DATA_PIN      17
#define FRAMES_PER_SECOND 120
#define MAX_POINTS 5
#define ONE_POINT (NUM_LEDS / 2 / MAX_POINTS)
#define BRIGHTNESS 20

// Yellow wire is connected to TX / GPIO 1 with weak pull up
// Red wire is connected to pin 35
// black is 34
#define P1_IN_PIN 34
#define P2_IN_PIN 35

CRGB leds[NUM_LEDS];
uint8_t gHue = 0;

int centerIdx = floor(NUM_LEDS / 2);

int p1Score = 0;
int p2Score = 0;
bool button1pressed = false;
bool button2pressed = false;
bool pulse1 = false;
bool stopGame = false;
int p1State = 0;
int p2State = 0;
int p1Threshhold = 2800;
int p2Threshhold = 2800;
long changed = 0;

RunningAverage p1RA(10);
RunningAverage p2RA(10);

void markChanged() {
  changed = 2000l * (p1Score + p2Score + 1);
}

void reset() {
  p1Score = 0;
  p2Score = 0;
  for (int i = 0; i < FRAMES_PER_SECOND / 2; i++) {
    p1RA.addValue( analogRead(P1_IN_PIN));
    p2RA.addValue( analogRead(P2_IN_PIN));
  }
  if (p1RA.getAverage() > p1Threshhold) {
    p1State = FRAMES_PER_SECOND * 2 / 3;
  }
  if (p2RA.getAverage() > p2Threshhold) {
    p2State = FRAMES_PER_SECOND * 2 / 3;
  }
  markChanged();
}

void setup() {
  Serial.begin(115200);
  while(!Serial) {
    delay(10);
  }

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(P1_IN_PIN, INPUT);
  pinMode(P2_IN_PIN, INPUT);

  // To try:
  // setCpuFrequencyMhz(120);

  // uint32_t Freq = 0;
  // // setCpuFrequencyMhz(120);
  // Freq = getCpuFrequencyMhz();
  // Serial.print("CPU Freq = ");
  // Serial.print(Freq);
  // Serial.println(" MHz");
  // Freq = getXtalFrequencyMhz();
  // Serial.print("XTAL Freq = ");
  // Serial.print(Freq);
  // Serial.println(" MHz");
  // Freq = getApbFrequency();
  // Serial.print("APB Freq = ");
  // Serial.print(Freq);
  // Serial.println(" Hz");

  reset();

}


// ************* Start of "test" ********************
void comet();

void loop()
{

  long startTime = millis();

    // EVERY_N_MILLISECONDS(100)
    // {
    //   Serial.print(p1Score);
    //   Serial.print(":");
    //   Serial.println(p2Score);
    // }

    FastLED.clear();

  if (changed < 0) {
    comet();
  }
  else {
    leds[centerIdx] = CRGB::Green;
  }

      p1RA.addValue( analogRead(P1_IN_PIN));
      p2RA.addValue( analogRead(P2_IN_PIN));

      int p1Val = p1RA.getAverage();


    EVERY_N_MILLISECONDS(100)
    {
      if (p1Val > p1Threshhold && p1Val < 3900) {
        Serial.printf("p1Val = %d\n", p1Val);
      }
    }

      // if reading high...
      if (p1Val > p1Threshhold) {
        // and the player state is unhooked
        if (p1State == 0) {
          markChanged();
          // give them a point
          p1Score++;
          // mark the player as hooked (FRAMES_PER_SECOND would be off for 1 second)
          p1State = FRAMES_PER_SECOND * 2 / 3;
        }
        else {
          p1Threshhold = min(p1Val - 200, 2800);
        }
      }
      else {
        // if the player was hooked
        if (p1State > 0) {
          markChanged();

          // make them unhooked after some amount of time (set above)
          p1State--;
        }
      }

      // mark the lights with the number of points
      // fill_solid(leds, map(p1Score, 0, MAX_POINTS, 0, NUM_LEDS), CRGB::Blue);
      for (int i = floor(NUM_LEDS / 2) - 1; i > floor(NUM_LEDS / 2) - ONE_POINT * p1Score; i--)
      {
        leds[i] = CRGB::Red;
      }

      if (p1Score == MAX_POINTS) {
        FastLED.clear();

        for (int j = 0; j < FRAMES_PER_SECOND * 3; j++)
        {

          for (int i = floor(NUM_LEDS / 2) - 1; i >= floor(NUM_LEDS / 2) - ONE_POINT * p1Score; i--)
          {
            leds[i] = CRGB::Red;
            leds[i].nscale8(beatsin8(40));
          }
          FastLED.show();
          FastLED.delay(1000 / FRAMES_PER_SECOND);
        }
        reset();
      }


      // ----- Player 2:

      int p2Val = p2RA.getAverage();

    EVERY_N_MILLISECONDS(100)
    {
      if (p2Val > p2Threshhold && p2Val < 3900) {
        Serial.printf("p2Val = %d\n", p2Val);
      }
    }

      // if reading high...
      if (p2Val > p2Threshhold) {
        // and the player state is unhooked
        if (p2State == 0) {
          markChanged();
          // give them a point
          p2Score++;
          // mark the player as hooked (FRAMES_PER_SECOND would be off for 1 second)
          p2State = FRAMES_PER_SECOND * 2 / 3;
        }
        else {
          p2Threshhold = min(p2Val - 200, 2800);
        }
      }
      else {
        // if the player was hooked
        if (p2State > 0) {
          markChanged();

          // make them unhooked after some amount of time (set above)
          p2State--;
        }
      }

      // mark the lights with the number of points
      for (int i = ceil(NUM_LEDS / 2) + 1; i < floor(NUM_LEDS / 2) + ONE_POINT * p2Score; i++)
      {
        leds[i] = CRGB::Blue;
      }


      if (p2Score == MAX_POINTS) {
        FastLED.clear();

        for (int j = 0; j < FRAMES_PER_SECOND * 3; j++)
        {
          for (int i = ceil(NUM_LEDS / 2) + 1; i <= floor(NUM_LEDS / 2) + ONE_POINT * p2Score; i++)
          {
            leds[i] = CRGB::Blue;
            leds[i].nscale8(beatsin8(40));
          }
          FastLED.show();
          FastLED.delay(1000 / FRAMES_PER_SECOND);
        }

        reset();
      }

  FastLED.show();
  long delayAmt = (1000 / FRAMES_PER_SECOND) - (millis() - startTime);
  if (delayAmt > 0) {
    changed--;
  }
  else {
    delayAmt = 0;
  }
  // Serial.print("Sleeping for x ms: ");
  // Serial.println(delayAmt);
  FastLED.delay(delayAmt);
}

// ************* End of "test" ******************


// // ************* Start of "rainbowWithGlitter" ********************
// void rainbow() 
// {
//   // FastLED's built-in rainbow generator
//   fill_rainbow( leds, NUM_LEDS, gHue, 7);
// }

// void addGlitter( fract8 chanceOfGlitter) 
// {
//   if( random8() < chanceOfGlitter) {
//     leds[ random16(NUM_LEDS) ] += CRGB::White;
//   }
// }

// void rainbowWithGlitter() 
// {
//   // built-in FastLED rainbow, plus some random sparkly glitter
//   rainbow();
//   addGlitter(80);
// }


// void loop() {
//   FastLED.clear();

//   rainbowWithGlitter();

//   FastLED.show();
//   FastLED.delay(1000 / FRAMES_PER_SECOND);
// }
// // ************* End of "rainbowWithGlitter" ******************


// ******* Start of "comet" ***********
#define LENGTH_OF_TAIL 20

int direction = 1;
int latch = NUM_LEDS - 1 + LENGTH_OF_TAIL;
int tailIdx = LENGTH_OF_TAIL;

void comet() {
  FastLED.clear();

  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(30, 0, NUM_LEDS - 1 + LENGTH_OF_TAIL + LENGTH_OF_TAIL);
  for (int i = 0; i < LENGTH_OF_TAIL; i++) {
    int newPos = pos - (direction * i) - LENGTH_OF_TAIL;
    if (newPos >= 0 && newPos < NUM_LEDS) {
      leds[newPos] = CHSV(gHue, 255, 192 - (i * 192 / LENGTH_OF_TAIL));
    }
  }

  if (pos == NUM_LEDS - 1 + LENGTH_OF_TAIL + LENGTH_OF_TAIL) {
    direction = -1;
  }
  else if (pos == 0) {
    direction = 1;
  }

  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);

// I would use EVERY_N_MILLISECONDS but with my loop above, the time is greater than 10 milliseconds and this isn't fired.
// EVERY_N_MILLISECONDS(10) { 
  gHue++; 
  // }
}
// ******* End of "comet" ***********
