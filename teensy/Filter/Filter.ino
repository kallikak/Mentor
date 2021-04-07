
#include <math.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       oscillator;     //xy=83.5,203
AudioFilterStateVariable filter;        //xy=237,210
AudioMixer4              mixer;         //xy=408,215
AudioOutputI2S           i2s;            //xy=586,212
AudioConnection          patchCord1(oscillator, 0, filter, 0);
AudioConnection          patchCord2(filter, 0, mixer, 0);
AudioConnection          patchCord3(filter, 2, mixer, 1);
AudioConnection          patchCord4(mixer, 0, i2s, 0);
AudioConnection          patchCord5(mixer, 0, i2s, 1);
AudioControlSGTL5000     sgtl5000;       //xy=538,138
// GUItool: end automatically generated code

#define PITCH_POT A0
#define WAVETYPE_POT A1
#define CUTOFF_POT A2
#define RES_POT A3

#define LED 3
#define HIGH_LOW_SW 4

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
int cutoff = 2000;
int resonance = 0;
bool isLowPass = true;

// pitch scales logarithmically
float inputToPitch(int input)
{
    int n = map(input, 0, 1023, 21, 108);
    return 440 * pow(2, (n - 69) / 12.0);
}

// input is from 0 to 127
void setCutoff(int u)
{
    // Use an exponential curve from 50Hz to about 12kHz
    float co = 50 * exp(5.481 * u / 127.0);
    filter.frequency(co);
    filter.octaveControl(log2f(12000.0 / (float)co));
}

// input is from 0 to 127
void setResonance(int u)
{
    // Convert to an appropriate range for the Teensy Audio Library filter object
    filter.resonance(u * 4.3 / 127.0 + 0.7);
}

wavetype getWaveTypeFromValue(int inputValue)
{
    int n = map(inputValue, 0, 1023, 0, N_TYPES - 1);
    return (wavetype)n;
}

void updateFilter()
{
    digitalWrite(LED, isLowPass);
    // turn on the correct channel from the mixer
    mixer.gain(0, isLowPass ? 1 : 0);
    mixer.gain(1, isLowPass ? 0 : 1);
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
    // and the two pins for the filter pots
    pinMode(CUTOFF_POT, INPUT);
    pinMode(RES_POT, INPUT);
    
    // and the switch/LED pins
    pinMode(LED, OUTPUT);
    pinMode(HIGH_LOW_SW, INPUT_PULLUP);
    updateFilter();
    
    setCutoff(cutoff / 127.0);
    setResonance(resonance / 127.0);

    // configure and start the oscillator object
    oscillator.amplitude(1);
    oscillator.frequency(pitch);
    waveform = getWaveTypeFromValue(analogRead(WAVETYPE_POT));
    oscillator.begin(waveform);
}

void loop() 
{
    static long lastpress = 0;
    if ((digitalRead(HIGH_LOW_SW) == 0) && (millis() - lastpress > 200))
    {
        // switch is on (line pulled low)
        lastpress = millis();
        isLowPass = !isLowPass;
        updateFilter();
    }
    
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
    
    // read the cutoff pot position
    int newcutoff = analogRead(CUTOFF_POT) >> 3;
    // has it changed?
    if (newcutoff != cutoff)
    {
        // update if it has
        cutoff = newcutoff;
        setCutoff(cutoff);
    }
    
    // read the resonance pot position
    int newres = analogRead(RES_POT) >> 3;
    // has it changed?
    if (newres != resonance)
    {
        // update if it has
        resonance = newres;
        setResonance(resonance);
    }
}
