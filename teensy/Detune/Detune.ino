
#include <math.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       oscillator1;      //xy=252,289
AudioSynthWaveform       oscillator2; //xy=254,337
AudioMixer4              mixer;         //xy=448,338
AudioOutputI2S           i2s;           //xy=612,338
AudioConnection          patchCord1(oscillator1, 0, mixer, 0);
AudioConnection          patchCord2(oscillator2, 0, mixer, 1);
AudioConnection          patchCord3(mixer, 0, i2s, 0);
AudioConnection          patchCord4(mixer, 0, i2s, 1);
AudioControlSGTL5000     sgtl5000;     //xy=599,273
// GUItool: end automatically generated code

#define PITCH_POT A0
#define DETUNE_POT A1

int pitch = 220;
int detune = 0;
int detunepotvalue = 0;
int waveform = WAVEFORM_SAWTOOTH;

// pitch scales logarithmically
float inputToPitch(int input, int detune)
{
    int n = map(input, 0, 1023, 21, 108);
    float f = 440 * pow(2, (n - 69) / 12.0);
    // calculate the detuned frequency using the fact that there are 1200 cents in an octave
    return f * pow(2, detune / 1200.0);
}

/*
 * Because pot reading can be noisy, we will work with an average of the most recent 'n' readings.
 * This is a useful utility function for working with a Teensy.
 */
int smoothpot(int newvalue, int curvalue, int n)
{
  return floor(((n - 1) * curvalue + newvalue) / n);
}

void setup() 
{
    // reserve some memory for the audio functions
    AudioMemory(20);
    // enable the audio control chip on the Audio Shield
    sgtl5000.enable();
    sgtl5000.volume(0.5);

    // setup the two pins for listening to the pitch and amplitude controls
    pinMode(PITCH_POT, INPUT);
    pinMode(DETUNE_POT, INPUT);

    // configure and start the oscillators
    oscillator1.frequency(pitch);
    oscillator1.amplitude(0.5);
    oscillator1.begin(waveform);
    oscillator2.frequency(pitch);
    oscillator2.amplitude(0.5);
    oscillator2.begin(waveform);
}

void loop() 
{
    // read the pitch pot position
    int newpitch = analogRead(PITCH_POT);
    // has it changed?
    if (newpitch != pitch)
    {
        // update if it has
        pitch = newpitch;
        oscillator1.frequency(inputToPitch(newpitch, 0));
        oscillator2.frequency(inputToPitch(newpitch, detune));
    }

    // read the detune pot position
    detunepotvalue = smoothpot(analogRead(DETUNE_POT), detunepotvalue, 16);
    int newdetune = detunepotvalue / 5 - 102; // range is -102 to 102 cents
    // make 0 a bit sticky since it is an important value and there is 
    // a lot of noise when reading a pot like this
    if (newdetune > 0)
        newdetune = max(0, newdetune - 2);
    else if (newdetune < 0)
        newdetune = min(0, newdetune + 2);
    // has it changed?
    if (newdetune != detune)
    {
        // update if it has
        detune = newdetune;
        oscillator1.frequency(inputToPitch(newpitch, 0));
        oscillator2.frequency(inputToPitch(newpitch, detune));
    }
}
