
#include <math.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformDc     pitchAmt;       //xy=107,193
AudioSynthWaveform       LFO;            //xy=107,254
AudioEffectMultiply      multiply;       //xy=264,220
AudioSynthWaveformModulated oscillator;     //xy=414,227
AudioOutputI2S           i2s;            //xy=595,229
AudioConnection          patchCord1(pitchAmt, 0, multiply, 0);
AudioConnection          patchCord2(LFO, 0, multiply, 1);
AudioConnection          patchCord3(multiply, 0, oscillator, 0);
AudioConnection          patchCord4(oscillator, 0, i2s, 0);
AudioConnection          patchCord5(oscillator, 0, i2s, 1);
AudioControlSGTL5000     sgtl5000;       //xy=534,166
// GUItool: end automatically generated code

#define PITCH_POT A0
#define SHAPE_POT A1
#define RATE_POT A2
#define AMOUNT_POT A3

#define LED 3

// map our wavetype to teensy waveform (note there are two pulse waves)
int shapes[] = {
    WAVEFORM_SINE, WAVEFORM_TRIANGLE, WAVEFORM_SAWTOOTH, 
    WAVEFORM_SAWTOOTH_REVERSE, WAVEFORM_SQUARE, WAVEFORM_SAMPLE_HOLD
};

#define N_TYPES 6

int pitch = 220;
int shape = 0;
int rate = 0;
int amount = 0;

bool ledstate = false;  // toggle LED to give feedback for shape changes

// pitch scales logarithmically
float inputToPitch(int input)
{
    int n = map(input, 0, 1023, 21, 108);
    return 440 * pow(2, (n - 69) / 12.0);
}

int getShapeFromValue(int inputValue)
{
    return map(inputValue, 0, 1023, 0, N_TYPES - 1);
}

void setLFORate(int u)
{
// convert log scale 0 to 127 to a linear rate
// f(1) = 0.5Hz, f(127) = 40Hz
    float f = exp(u / 25.0) / 4.0;
    LFO.frequency(f);
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
    pinMode(SHAPE_POT, INPUT);
    pinMode(RATE_POT, INPUT);
    pinMode(AMOUNT_POT, INPUT);
    
    pinMode(LED, OUTPUT);

    // configure and start the oscillator object
    oscillator.amplitude(0.5);
    oscillator.frequency(pitch);
    oscillator.begin(WAVEFORM_SAWTOOTH);
    oscillator.frequencyModulation(1);

    setLFORate(rate);
    LFO.amplitude(1);
    shape = getShapeFromValue(analogRead(SHAPE_POT));
    LFO.begin(shape);
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
    int v = analogRead(SHAPE_POT);
    int newshape = getShapeFromValue(v);
    // has it changed?
    if (newshape != shape)
    {
        // update if it has
        shape = newshape;
        LFO.begin(shapes[shape]);
        digitalWrite(LED, ledstate);
        ledstate = !ledstate;
    }
    
    // read the cutoff pot position
    int newrate = analogRead(RATE_POT) >> 3;
    // has it changed?
    if (newrate != rate)
    {
        // update if it has
        rate = newrate;
        setLFORate(rate);
    }
    
    // read the cutoff pot position
    int newamount = analogRead(AMOUNT_POT) >> 3;
    // has it changed?
    if (newamount != amount)
    {
        // update if it has
        amount = newamount;
        pitchAmt.amplitude(amount / 127.0);
    }
}
