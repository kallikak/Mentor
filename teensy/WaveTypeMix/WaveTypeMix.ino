
#include <math.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       triangle;      //xy=252,289
AudioSynthWaveform       square; //xy=253,388
AudioSynthWaveform       sawtooth; //xy=254,337
AudioMixer4              mixer;         //xy=448,338
AudioOutputI2S           i2s;           //xy=612,338
AudioConnection          patchCord1(triangle, 0, mixer, 0);
AudioConnection          patchCord3(sawtooth, 0, mixer, 1);
AudioConnection          patchCord2(square, 0, mixer, 2);
AudioConnection          patchCord4(mixer, 0, i2s, 0);
AudioConnection          patchCord5(mixer, 0, i2s, 1);
AudioControlSGTL5000     sgtl5000;     //xy=599,273
// GUItool: end automatically generated code

#define PITCH_POT A0
#define MIX_POT A1

#define TRIANGLE 0
#define SAWTOOTH 1
#define SQUARE 2

int pitch = 220;
float mixf = 1.0;

// pitch scales logarithmically
float inputToPitch(int input)
{
    int n = map(input, 0, 1023, 21, 108);
    return 440 * pow(2, (n - 69) / 12.0);
}

void applyMixValue(float mixf)
{
    // allocate across the wavetypes
    // 0 to 1 mixes triangle and sawtooth, 1 to 2 mixes sawtooth and square
    if (mixf < 1)
    {
        mixer.gain(TRIANGLE, 1 - mixf);
        mixer.gain(SAWTOOTH, mixf);
        mixer.gain(SQUARE, 0);
    }
    else
    {
        mixer.gain(TRIANGLE, 0);
        mixer.gain(SAWTOOTH, 2 - mixf);
        mixer.gain(SQUARE, mixf - 1);
    }
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
    pinMode(MIX_POT, INPUT);

    // configure and start the three oscillators
    triangle.frequency(pitch);
    triangle.amplitude(0.5);
    triangle.begin(WAVEFORM_TRIANGLE);

    sawtooth.frequency(pitch);
    sawtooth.amplitude(0.5);
    sawtooth.begin(WAVEFORM_SAWTOOTH);

    square.frequency(pitch);
    square.amplitude(0.5);
    square.begin(WAVEFORM_SQUARE);

    applyMixValue(mixf);
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
        triangle.frequency(inputToPitch(newpitch));
        sawtooth.frequency(inputToPitch(newpitch));
        square.frequency(inputToPitch(newpitch));
    }

    // read the mix pot position
    float newmix = 2 * analogRead(MIX_POT) / 1023.0;
    // has it changed?
    if (newmix != mixf)
    {
        // update if it has
        mixf = newmix;
        applyMixValue(mixf);
    }
}
