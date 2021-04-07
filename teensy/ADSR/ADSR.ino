
#include <math.h>
#include "MIDI.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       oscillator;     //xy=83.5,203
AudioEffectEnvelope      envelope;      //xy=240,167
AudioMixer4              mixer;         //xy=386,207
AudioOutputI2S           i2s;            //xy=555,206
AudioConnection          patchCord1(oscillator, envelope);
AudioConnection          patchCord2(oscillator, 0, mixer, 1);
AudioConnection          patchCord3(envelope, 0, mixer, 0);
AudioConnection          patchCord4(mixer, 0, i2s, 0);
AudioConnection          patchCord5(mixer, 0, i2s, 1);
AudioControlSGTL5000     sgtl5000;       //xy=511,138
// GUItool: end automatically generated code

#define A_POT A0
#define D_POT A1
#define S_POT A2
#define R_POT A3

#define LED 3
#define ENABLE 4

#define ENV_MILLIS 10000 // max transition time is 10s

float volume = 0.5;
int pitch = 220;
int curnote = 0;

int a_val = 30;
int d_val = 60;
int s_val = 100;
int r_val = 60;

bool enable = true;

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

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // set the current active note
    curnote = pitch;
    oscillator.amplitude(volume);
    oscillator.frequency(inputToPitch(curnote));
    envelope.noteOn();
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    if (pitch == curnote)
    {
        // turn off the oscillator because the active note has been lifted
        if (!enable)
            oscillator.amplitude(0);
        curnote = 0;
        envelope.noteOff();
    }
}

void setup() 
{
    // reserve some memory for the audio functions
    AudioMemory(20);
    // enable the audio control chip on the Audio Shield
    sgtl5000.enable();
    sgtl5000.volume(0.5);

    // setup four pins for listening to the ADSR controls
    pinMode(A_POT, INPUT);
    pinMode(D_POT, INPUT);
    pinMode(S_POT, INPUT);
    pinMode(R_POT, INPUT);

    // and the switch/LED pins
    pinMode(LED, OUTPUT);
    pinMode(ENABLE, INPUT_PULLUP);
    updateEnable();
    
    oscillator.begin(WAVEFORM_SQUARE);

    // Setup the MIDI listening on channel 1
    MIDI.begin(1);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
}

bool checkPot(int *curValue, int pin)
{
    int newValue = analogRead(pin) >> 3;  // reduce to between 0 to 127
    // has it changed?
    if (newValue != *curValue)
    {
        *curValue = newValue;
        return true;
    }
    return false;
}

void updateEnable()
{
    digitalWrite(LED, enable);
    // turn on the correct channel from the mixer
    mixer.gain(0, enable ? 1 : 0);
    mixer.gain(1, enable ? 0 : 1);
    if (!enable && curnote == 0)
        oscillator.amplitude(0);
}

void loop() 
{
    static long lastpress = 0;
    if ((digitalRead(ENABLE) == 0) && (millis() - lastpress > 200))  // switch is on (line pulled low)
    {
        lastpress = millis();
        enable = !enable;
        updateEnable();
    }

    if (checkPot(&a_val, A_POT))
        envelope.attack(a_val / 127.0 * ENV_MILLIS);  // attack time in ms

    if (checkPot(&d_val, D_POT))
        envelope.decay(d_val / 127.0 * ENV_MILLIS);  // decay time in ms

    if (checkPot(&s_val, S_POT))
        envelope.sustain(s_val / 127.0);  // sustain is a level, not a time

    if (checkPot(&r_val, R_POT))
        envelope.release(r_val / 127.0 * ENV_MILLIS);  // release time in ms

    // check for MIDI messages (the callbacks will handle them if there are any)
    MIDI.read();
}
