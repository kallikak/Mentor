
#include <math.h>
#include "MIDI.h"

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

#define RANGE_POT A0
#define WAVETYPE_POT A1

typedef enum 
{
    SINE, TRIANGLE, SAWTOOTH, SQUARE, PULSE_25, PULSE_10
} wavetype;

int wavetypes[] = {
    WAVEFORM_SINE, WAVEFORM_TRIANGLE, WAVEFORM_SAWTOOTH, 
    WAVEFORM_SQUARE, WAVEFORM_PULSE, WAVEFORM_PULSE
};

#define N_TYPES 6

float volume = 0.5;
int pitch = 220;
int octaveshift = 0;
int curnote = 0;
wavetype waveform = (wavetype)100;  // illegal value guarantees update on first loop

// Define the callbacks for handling MIDI note on and note off messages
void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleNoteOff(byte channel, byte pitch, byte velocity);

// Create a hardware MIDI instance on Serial1 (pins 1 and 2)
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// pitch scales logarithmically
float inputToPitch(int n)
{
    return 440 * pow(2, (n - 69) / 12.0);
}

wavetype getWaveTypeFromValue(int inputValue)
{
    int n = map(inputValue, 0, 1023, 0, N_TYPES - 1);
    return (wavetype)n;
}

void playNote(bool updateWaveType)
{
    if (updateWaveType)
    {
        oscillator.begin(wavetypes[waveform]);
        if (waveform == PULSE_25)
            oscillator.pulseWidth(0.25);
        else if (waveform == PULSE_10)
            oscillator.pulseWidth(0.1);
        else    // just to be safe
            oscillator.pulseWidth(0.5);
    }
    if (curnote)
    {
        oscillator.amplitude(volume);
        oscillator.frequency(inputToPitch(curnote + octaveshift * 12));
    }
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // set the current active note
    curnote = pitch;
    playNote(false);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    if (pitch == curnote)
    {
        // turn off the oscillator because the active note has been lifted
        oscillator.amplitude(0);
        curnote = 0;
    }
}

void setup() 
{
    // reserve some memory for the audio functions
    AudioMemory(20);
    // enable the audio control chip on the Audio Shield
    sgtl5000.enable();
    sgtl5000.volume(0.5);

    // setup the two pins for listening to the range and wavetype controls
    pinMode(RANGE_POT, INPUT);
    pinMode(WAVETYPE_POT, INPUT);

    // Setup the MIDI listening on channel 1
    MIDI.begin(1);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
}

void loop() 
{
    // read the range pot position
    int newrange = map(analogRead(RANGE_POT), 0, 1023, -2, 2);
    // has it changed?
    if (newrange != octaveshift)
    {
        // update if it has
        octaveshift = newrange;
        playNote(false);
    }

    // read the wavetype pot position
    int v = analogRead(WAVETYPE_POT);
    wavetype newwave = getWaveTypeFromValue(v);
    // has it changed?
    if (newwave != waveform)
    {
        // update if it has
        waveform = newwave;
        playNote(true);
    }

    // check for MIDI messages (the callbacks will handle them if there are any)
    MIDI.read();
}
