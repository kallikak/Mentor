
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
#define AMP_POT A1

int pitch = 220;
int amplitude = 0.5;
int waveform = WAVEFORM_SINE;

// pitch scales logarithmically
float inputToPitch(int input)
{
    int n = map(input, 0, 1023, 21, 108);
    return 440 * pow(2, (n - 69) / 12.0);
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
    pinMode(AMP_POT, INPUT);

    // configure and start the oscillator object
    oscillator.amplitude(amplitude);
    oscillator.frequency(pitch);
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

    // read the amp pot position
    int newamp = analogRead(AMP_POT);
    // has it changed?
    if (newamp != amplitude)
    {
        // update if it has
        amplitude = newamp;
        oscillator.amplitude(1.0f * amplitude / 1024);
    }
}
