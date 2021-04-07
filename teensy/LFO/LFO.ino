
#include <math.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformDc     pitchAmt;            //xy=90,133
AudioSynthWaveform       LFO;      //xy=90,194
AudioSynthWaveformDc     filterAmt; //xy=90,255
AudioEffectMultiply      multiply;      //xy=247,160
AudioEffectMultiply      multiply1; //xy=250,233
AudioSynthWaveformModulated oscillator;   //xy=396,166
AudioFilterStateVariable filter;        //xy=541,224
AudioOutputI2S           i2s;            //xy=707,208
AudioConnection          patchCord1(pitchAmt, 0, multiply, 0);
AudioConnection          patchCord2(LFO, 0, multiply, 1);
AudioConnection          patchCord3(LFO, 0, multiply1, 0);
AudioConnection          patchCord4(filterAmt, 0, multiply1, 1);
AudioConnection          patchCord5(multiply, 0, oscillator, 0);
AudioConnection          patchCord6(multiply1, 0, filter, 1);
AudioConnection          patchCord7(oscillator, 0, filter, 0);
AudioConnection          patchCord8(filter, 0, i2s, 0);
AudioConnection          patchCord9(filter, 0, i2s, 1);
AudioControlSGTL5000     sgtl5000;       //xy=618,140
// GUItool: end automatically generated code

#define PITCH_POT A0
#define CUTOFF_POT A1
#define RATE_POT A2
#define AMOUNT_POT A3

#define LED 3
#define PITCH_FLT_SW 4

int pitch = 220;
int cutoff = 2000;
int rate = 0;
int amount = 0;
bool isPitch = true;

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

void setLFORate(int u)
{
// convert log scale 0 to 127 to a linear rate
// f(1) = 0.5Hz, f(127) = 40Hz
    float f = exp(u / 25.0) / 4.0;
    LFO.frequency(f);
}

void updateLFO()
{
    digitalWrite(LED, isPitch);
    pitchAmt.amplitude(isPitch ? amount / 127.0 : 0);
    filterAmt.amplitude(isPitch ? 0 : amount / 127.0);
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
    pinMode(CUTOFF_POT, INPUT);
    pinMode(RATE_POT, INPUT);
    pinMode(AMOUNT_POT, INPUT);
    
    // and the switch/LED pins
    pinMode(LED, OUTPUT);
    pinMode(PITCH_FLT_SW, INPUT_PULLUP);
    updateLFO();

    // configure and start the oscillator object
    oscillator.amplitude(0.5);
    oscillator.frequency(pitch);
    oscillator.begin(WAVEFORM_SAWTOOTH);
    oscillator.frequencyModulation(1);

    setLFORate(rate);
    LFO.amplitude(1);
    LFO.begin(WAVEFORM_TRIANGLE);
}

void loop() 
{
    static long lastpress = 0;
    if ((digitalRead(PITCH_FLT_SW) == 0) && (millis() - lastpress > 200))  // switch is on (line pulled low)
    {
        lastpress = millis();
        isPitch = !isPitch;
        updateLFO();
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
    
    // read the cutoff pot position
    int newcutoff = analogRead(CUTOFF_POT) >> 3;
    // has it changed?
    if (newcutoff != cutoff)
    {
        // update if it has
        cutoff = newcutoff;
        setCutoff(cutoff);
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
        updateLFO();
    }
}
