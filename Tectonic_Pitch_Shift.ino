// Granular Effect Example - Pitch shift or freeze sound
//
// This example is meant to be used with 3 buttons (pin 0,
// 1, 2) and 2 knobs (pins 16/A2, 17/A3), which are present
// on the audio tutorial kit.
//   https://www.pjrc.com/store/audio_tutorial_kit.html
//
// Data files to put on your SD card can be downloaded here:
//   http://www.pjrc.com/teensy/td_libs_AudioDataFiles.html
//
// This example code is in the public domain.

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>
#include "octave_down.h"

AudioInputI2S            i2s2;
AudioPlaySdWav           playSdWav1;
AudioMixer4              mixer1;
AudioEffectGranular      granular1;
AudioEffectOctaveDown	 octavedown1;
AudioMixer4              mixer2;
AudioOutputI2S           i2s1;

AudioConnection          patchCord10(i2s2, 0, mixer1, 2);
AudioConnection          patchCord11(i2s2, 1, mixer1, 3);

AudioConnection          patchCord1(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord2(playSdWav1, 1, mixer1, 1);

AudioConnection          patchCord3(mixer1, octavedown1);
AudioConnection          patchCord4(mixer1, granular1);

AudioConnection          patchCord5(octavedown1, 0, mixer2, 0);
AudioConnection          patchCord6(granular1, 0, mixer2, 1);
AudioConnection          patchCord7(mixer1, 0, mixer2, 2);

AudioConnection          patchCord8(mixer2, 0, i2s1, 0);
AudioConnection          patchCord9(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=236,248

Bounce button1 = Bounce(1, 15);

#define GRANULAR_MEMORY_SIZE 12800  // enough for 290 ms at 44.1 kHz
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

#define NUM_FILES  4
const char *filenames[NUM_FILES]={"SDTEST2.WAV", "SDTEST1.WAV", "SDTEST3.WAV", "SDTEST4.WAV"};
int nextfile=0;

bool off = false;

void setup() {
  Serial.begin(9600);
  AudioMemory(100);
  pinMode(1, INPUT_PULLUP);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);

  // the Granular effect requires memory to operate
  granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);

#ifdef USE_SDCARD
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);
  mixer1.gain(2, 0);
  mixer1.gain(3, 0);
#else
  mixer1.gain(0, 0);
  mixer1.gain(1, 0);
  mixer1.gain(2, 0.5);
  mixer1.gain(3, 0.5);
#endif


  granular1.setSpeed(0.5);
}

void loop() {
#ifdef USE_SDCARD
  if (playSdWav1.isPlaying() == false) {
    // start the next song playing
    playSdWav1.play(filenames[nextfile]);
    Serial.print("Playing: ");
    Serial.println(filenames[nextfile]);
    delay(5); // brief delay for the library read WAV info
    nextfile = nextfile + 1;
    if (nextfile >= NUM_FILES) {
      nextfile = 0;
    }
  }
#endif

  button1.update();
  // read knobs, scale to 0-1.0 numbers
  float knobA14 = (float)analogRead(A14) / 1023.0;
  float knobA15 = (float)analogRead(A15) / 1023.0;
  float knobA16 = (float)analogRead(A16) / 1023.0;
  float knobA17 = (float)analogRead(A17) / 1023.0;
  float knobA18 = (float)analogRead(A18) / 1023.0;
  float knobA19 = (float)analogRead(A19) / 1023.0;

  // Button 1 starts Pitch Shift effect
  if (button1.fallingEdge()) {
    mixer2.gain(0, 0);
    mixer2.gain(1, 0);
    mixer2.gain(2, 0);
    granular1.stop();
    octavedown1.end();
    off = true;
  }
  if (button1.risingEdge()) {
    float gain_msec = 25.0 + (knobA17 * 75.0);
    granular1.beginPitchShift(gain_msec);
    float win_size = 20.0 + (knobA19 * 250.0);
    float win_offset = win_size * ((knobA18 * 0.45) + 0.05);
    octavedown1.begin(120.0, 33.0);
    Serial.printf("Begin with %0.1fms window, %0.1fms offset, and %0.1fms grains\n",
	win_size, win_offset, gain_msec);
    off = false;
  }

  if (!off) {
    mixer2.gain(0, knobA16);
    mixer2.gain(1, knobA15);
    mixer2.gain(2, knobA14);
  }
}
