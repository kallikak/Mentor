
#include <math.h>
#include "MIDI.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       oscillator;     //xy=146.5,227
AudioMixer4              delaymixer;         //xy=350,288
AudioEffectDelay         delayeff;         //xy=353,416
AudioMixer4              mixer; //xy=520,249
AudioOutputI2S           i2s;            //xy=694,257
AudioConnection          patchCord1(oscillator, 0, delaymixer, 0);
AudioConnection          patchCord2(oscillator, 0, mixer, 0);
AudioConnection          patchCord3(delaymixer, delayeff);
AudioConnection          patchCord4(delaymixer, 0, mixer, 1);
AudioConnection          patchCord5(delayeff, 0, delaymixer, 1);
AudioConnection          patchCord6(mixer, 0, i2s, 0);
AudioConnection          patchCord7(mixer, 0, i2s, 1);
AudioControlSGTL5000     sgtl5000;       //xy=610,348
// GUItool: end automatically generated code

#define WAVETYPE_POT A0
#define DELAY_LVL_POT A1
#define DELAY_TIME_POT A2
#define DELAY_FB_POT A3

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
int curnote = 0;
wavetype waveform = (wavetype)100;  // illegal value guarantees update on first loop

int delaylevel = 0;
int delaytime = 500;  // millis
int delayfeedback = 63;

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
        oscillator.frequency(inputToPitch(curnote));
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
    // need alot of extra memory for a delay effect
    AudioMemory(400);
    // enable the audio control chip on the Audio Shield
    sgtl5000.enable();
    sgtl5000.volume(0.5);

    // setup the two pins for listening to the range and wavetype controls
    pinMode(WAVETYPE_POT, INPUT);
    pinMode(DELAY_LVL_POT, INPUT);
    pinMode(DELAY_TIME_POT, INPUT);
    pinMode(DELAY_FB_POT, INPUT);

    // Setup the MIDI listening on channel 1
    MIDI.begin(1);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    
    delaylevel = 0.5;
    delayeff.delay(0, delaytime);
    delaymixer.gain(0, 0.75);
    delaymixer.gain(1, delayfeedback / 127.0);
    mixer.gain(0, 1 - delaylevel / 127.0);
    mixer.gain(1, delaylevel / 127.0);
}

void loop() 
{
    // read the delay level pot position
    int newlevel = analogRead(DELAY_LVL_POT) >> 3;
    // has it changed?
    if (newlevel != delaylevel)
    {
        // update if it has
        delaylevel = newlevel;
        mixer.gain(0, 1 - delaylevel / 127.0);
        mixer.gain(1, delaylevel / 127.0);
    }
    
    // read the delay time pot position
    int newtime = analogRead(DELAY_TIME_POT) ; // up to 1s (well just over)
    // has it changed by at least 20ms?
    // (really need to avoid jitter here because it will have significant audible consequences)
    if (abs(newtime - delaytime) > 20)
    {
        // update if it has
        delaytime = newtime;
        delayeff.delay(0, delaytime);
    }
    
    // read the delay feedback pot position
    int newfeedback = analogRead(DELAY_FB_POT) >> 3;
    // has it changed?
    if (newfeedback != delayfeedback)
    {
        // update if it has
        delayfeedback = newfeedback;
        delaymixer.gain(1, delayfeedback / 127.0);
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
