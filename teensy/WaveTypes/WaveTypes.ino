
#include <math.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       oscillator;      //xy=252,335
AudioOutputI2S           i2s;           //xy=451,335
AudioConnection          patchCord1(oscillator, 0, i2s, 0);
AudioConnection          patchCord2(oscillator, 0, i2s, 1);
AudioControlSGTL5000     sgtl5000;     //xy=462,266
// GUItool: end automatically generated code

#define PITCH_POT A0
#define WAVETYPE_POT A1

typedef enum 
{
    SINE, TRIANGLE, SAWTOOTH, SQUARE, PULSE_25, PULSE_10
} wavetype;

// map our wavetype to teensy waveform (note there are two pulse waves)
int wavetypes[] = {
    WAVEFORM_SINE, WAVEFORM_TRIANGLE, WAVEFORM_SAWTOOTH, 
    WAVEFORM_SQUARE, WAVEFORM_PULSE, WAVEFORM_PULSE
};

#define N_TYPES 6

int pitch = 220;
wavetype waveform = SAWTOOTH;

// pitch scales logarithmically
float inputToPitch(int input)
{
    int n = map(input, 0, 1023, 21, 108);
    return 440 * pow(2, (n - 69) / 12.0);
}

wavetype getWaveTypeFromValue(int inputValue)
{
    int n = map(inputValue, 0, 1023, 0, N_TYPES - 1);
    return (wavetype)n;
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
    pinMode(WAVETYPE_POT, INPUT);

    // configure and start the oscillator object
    oscillator.amplitude(0.5);
    oscillator.frequency(pitch);
    waveform = getWaveTypeFromValue(analogRead(WAVETYPE_POT));
    oscillator.begin(waveform);
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
        oscillator.frequency(inputToPitch(newpitch));
    }

    // read the wavetype pot position
    int v = analogRead(WAVETYPE_POT);
    wavetype newwave = getWaveTypeFromValue(v);
    // has it changed?
    if (newwave != waveform)
    {
        // update if it has
        waveform = newwave;
        oscillator.begin(wavetypes[waveform]);
        if (waveform == PULSE_25)
            oscillator.pulseWidth(0.25);
        else if (waveform == PULSE_10)
            oscillator.pulseWidth(0.1);
        else    // just to be safe
            oscillator.pulseWidth(0.5);
    }
}
