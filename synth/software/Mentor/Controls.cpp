#include "Adafruit_MCP23017.h"
#include "Rotary.h"
#include <Bounce2mcp.h>

#include "Config.h"
#include "Controls.h"
#include "LCD.h"
#include "Utility.h"
#include "Synth.h"
#include "MIDIManager.h"

#define NUM_SELECT  6
class BounceMcp;

typedef struct
{
  int pinA;
  int pinB;
  uint16_t mask;
  bool halfstate;
  uint8_t _state;
  int minval[NUM_SELECT];
  int maxval[NUM_SELECT];
  BounceMcp *debouncer;
} encoder;

void setSelectLed(int led);
void showSelectSummary(int selected, bool addRow1);
void handleEncoderTurn(int i, int d);
void handleEncoderPress(int i);

int selected = 0;

#define SELECT_OSC  0
#define SELECT_FLT  1
#define SELECT_ENV  2
#define SELECT_LFO  3
#define SELECT_AMP  4
#define SELECT_EFF  5

int lastupdatemillis = 0;
bool summaryDone = false;

bool isFrequencyMode = false;
float droneFreq = 440;
  
Adafruit_MCP23017 *expander = NULL;

int selleds[] = { 2, 3, 4, 5, 6, 7 };

#define NUM_ENCODERS 6

encoder encoders[] = {
  { 0,  1, 0x0003, false, R_START, {0, 0, 0, 0, 0, 0}, {5, 5, 5, 5, 5, 5}, NULL },
  // Shape, Cutoff, Attack, LFO_Shape, Gain, Chorus
  { 8,  9, 0x0300, false, R_START, {0, 0, 0, 0, 0, 0}, {250, 127, 127, 6, 127, 15}, NULL },
  // Range, Res, Decay, Rate, Amp_Env_Amt, Delay
  {10, 11, 0x0700, false, R_START, {0, 0, 0, 0, 0, 0}, {3, 127, 127, 127, 127, 15}, NULL },
  // Poly, Flt_Env_Amt, Sustain, Pitch, LFO_Amt, Reverb
  {12, 13, 0x3000, false, R_START, {0, -63, 0, 0, 0, 0}, {POLYMAX, 64, 127, 127, 127, 15},  NULL },
  // Detune, HPF, Release, LFO_Flt, Vel_sens, Overdrive
  {14, 15, 0x7000, false, R_START, {0, 0, 0, 0, 0, 0}, {50, 127, 127, 127, 127, 15}, NULL }
};

#define SELECT_ENC 0

uint16_t blockMillis = 0;

void setupEncoders()
{
  int j;
  expander = new Adafruit_MCP23017();
  expander->begin((uint8_t)0);
  for (j = 0; j < NUM_ENCODERS; ++j)
  {
    encoder *enc = &encoders[j];
    expander->pinMode(enc->pinA, INPUT);
    expander->pullUp(enc->pinA, HIGH); // turn on a 100K pullup internally
    expander->pinMode(enc->pinB, INPUT);
    expander->pullUp(enc->pinB, HIGH); // turn on a 100K pullup internally
  }
  for (j = 0; j < NUM_SELECT; ++j)
  {
    int p = selleds[j];
    expander->pinMode(p, OUTPUT);
    expander->digitalWrite(p, LOW);
  }
}

static int lastencoder = -1;
static long lastencodermillis = -1;

#define MIN_ENC_TIME 50
#define MAX_ENC_TIME 200
#define MAX_ENC_ACCEL 16

void checkEncoders(unsigned long now)
{
  uint16_t vals = expander->readGPIOAB();
  for (int i = 0; i < NUM_ENCODERS; ++i)
  {
    encoder *e = &encoders[i]; 
    uint16_t val = vals;
    val >>= e->pinA;
    int p0 = val & 0x0001;
    int p1 = (val & 0x0002) >> 1;
    uint8_t pinstate = (p1 << 1) | p0;  
    if (e->halfstate)
      e->_state = ttable_half[e->_state & 0xf][pinstate];
    else
      e->_state = ttable[e->_state & 0xf][pinstate]; 
    int d;      
    if ((e->_state & 0x30) == DIR_CW)
      d = 1;
    else if ((e->_state & 0x30) == DIR_CCW)
      d = -1;
    else
      d = 0;

    if (d)
    {
      // add some acceleration for the encoders unless the value range is small
      bool accel = e->maxval[selected] - e->minval[selected] > 20;
      if (accel)
      {
        if (lastencoder != i)
        {
          lastencoder = i;
          lastencodermillis = -1;
        }
        else 
        {
          lastencoder = i;
          long delta = now - lastencodermillis;
          int factor;
          if (delta >= MAX_ENC_TIME)
            factor = 1;
          else if (delta <= MIN_ENC_TIME)
            factor = MAX_ENC_ACCEL;
          else
            factor = MAX_ENC_ACCEL - round(1.0 * (delta  - MIN_ENC_TIME) / (MAX_ENC_TIME - MIN_ENC_TIME) * (MAX_ENC_ACCEL - 1));
          d *= factor;
          lastencodermillis = now;
        }
      }
      handleEncoderTurn(i, d);
      lastupdatemillis = now;
      summaryDone = false;
    }
  }
}

void handleEncoderTurn(int i, int d)
{
  encoder *e = &encoders[i];
  switch (i)
  {
    case SELECT_ENC:
      selected += d;
      selected = (selected + NUM_SELECT) % NUM_SELECT; 
      setSelectLed(selected);
      break;
    case 1: // Shape, Cutoff, Attack, LFO_Shape, Gain, Chorus
      switch (selected)
      {
        case SELECT_OSC:  // Shape
          handleParameterUpdate(CC_SHAPE, constrainInt(config->osc.shape + d, e->minval[selected], e->maxval[selected]));
          break;
        case SELECT_FLT:  // Cutoff
          handleParameterUpdate(CC_CUTOFF, constrainCC(config->filter.cutoff + d));
          break;
        case SELECT_ENV:  // Attack
          handleParameterUpdate(CC_ATTACK, constrainCC(config->env.attack + d));
          break;
        case SELECT_LFO:  // LFO_Shape
          handleParameterUpdate(CC_LFO_SHAPE, getNextWaveshape(config->lfo.shape, d < 0));
          break;
        case SELECT_AMP:  // Gain
          handleParameterUpdate(CC_GAIN, constrainCC(config->amp.gain + d));
          break;
        case SELECT_EFF:  // Chorus
          handleParameterUpdate(CC_CHORUS, constrainInt(config->eff.chorus + d, e->minval[selected], e->maxval[selected]));
          break;
      }
      break;
    case 2: // Range, Res, Decay, Rate, Amp_Env_Amt, Delay
      switch (selected)
      {
        case SELECT_OSC:  // Range
          if (isFrequencyMode)
          {
            droneFreq += d / 4.0;
            droneFreq = max(21, min(108, droneFreq));
            synth->setFrequency(noteToFreq(droneFreq));
            writeParamValue(1, "Freq", noteToFreq(droneFreq));
          }
          else
            handleParameterUpdate(CC_RANGE, constrainInt((int)config->osc.range + d, e->minval[selected], e->maxval[selected]));
          break;
        case SELECT_FLT:  // Res
          handleParameterUpdate(CC_RESONANCE, constrainCC(config->filter.resonance + d));
          break;
        case SELECT_ENV:  // Decay
          handleParameterUpdate(CC_DECAY, constrainCC(config->env.decay + d));
          break;
        case SELECT_LFO:  // Rate
          handleParameterUpdate(CC_LFO_RATE, constrainCC(config->lfo.rate + d));
          break;
        case SELECT_AMP:  // Amp_Env_Amt
          handleParameterUpdate(CC_AMP_ENV, constrainCC(config->amp.envAmt + d));
          break;
        case SELECT_EFF:  // Delay
          handleParameterUpdate(CC_DELAY, constrainInt(config->eff.delay + d, e->minval[selected], e->maxval[selected]));
          break;
      }
      break;
    case 3:// Poly, Flt_Env_Amt, Sustain, Pitch, LFO_Amt, Reverb
      switch (selected)
      {
        case SELECT_OSC:  // Poly
          handleParameterUpdate(CC_POLY, constrainInt(config->osc.poly + d, e->minval[selected], e->maxval[selected]));
          break;
        case SELECT_FLT:  // Flt_Env_Amt
          handleParameterUpdate(CC_ENVAMT, constrainSignedCC(config->filter.envAmt + d));
          break;
        case SELECT_ENV:  // Sustain
          handleParameterUpdate(CC_SUSTAIN, constrainCC(config->env.sustain + d));
          break;
        case SELECT_LFO:  // Pitch
          handleParameterUpdate(CC_LFO_PITCH, constrainCC(config->lfo.pitchAmt + d));
          break;
        case SELECT_AMP:  // LFO_Amt
          handleParameterUpdate(CC_AMP_LFO, constrainCC(config->amp.lfoAmt + d));
          break;
        case SELECT_EFF:  // Reverb
          handleParameterUpdate(CC_REVERB, constrainInt(config->eff.reverb + d, e->minval[selected], e->maxval[selected]));
          break;
      }
      break;
    case 4:// Detune, HPF, Release, LFO_Flt, Vel_sens, Overdrive
      switch (selected)
      {
        case SELECT_OSC:  // Detune
          handleParameterUpdate(CC_DETUNE, constrainInt(config->osc.detune + d, e->minval[selected], e->maxval[selected]));
          break;
        case SELECT_FLT:  // HPF
          handleParameterUpdate(CC_HPF, constrainCC(config->filter.hpfCutoff + d));
          break;
        case SELECT_ENV:  // Release
          handleParameterUpdate(CC_RELEASE, constrainCC(config->env.release + d));
          break;
        case SELECT_LFO:  // LFO_Flt
          handleParameterUpdate(CC_LFO_FILTER, constrainCC(config->lfo.filterAmt + d));
          break;
        case SELECT_AMP:  // Vel_sens
          handleParameterUpdate(CC_VELSENS, constrainCC(config->amp.velSens + d));
          break;
        case SELECT_EFF:  // Overdrive
          handleParameterUpdate(CC_OVERDRIVE, constrainInt(config->eff.overdrive + d, e->minval[selected], e->maxval[selected]));
          break;
      }
      break;
  }
  showSelectSummary(selected, i == SELECT_ENC);
}

void handleParameterUpdate(ccInt param, int value)
{
  switch (param)
  {
    case CC_SHAPE:  // Shape
      config->osc.shape = value;
      synth->setWaveShape(config->osc.shape);
      writeRow(1, waveshapeSettingStr(config->osc.shape, false));
      break;
    case CC_CUTOFF:  // Cutoff
      config->filter.cutoff = value;
      synth->setCutoff(config->filter.cutoff);
      writeParamValue(1, "Cutoff", config->filter.cutoff);
      break;
    case CC_ATTACK:  // Attack
      config->env.attack = value;
      synth->setAttack(config->env.attack);
      writeParamValue(1, "Attack", config->env.attack);
      break;
    case CC_LFO_SHAPE:  // LFO_Shape
      config->lfo.shape = (waveshape)value;
      synth->setLFOShape(config->lfo.shape);
      writeParamString(1, "LFO", waveshapeStr(config->lfo.shape, false));
      break;
    case CC_GAIN:  // Gain
      config->amp.gain = value;
      synth->setGain(config->amp.gain);
      writeParamValue(1, "Gain", config->amp.gain);
      break;
    case CC_CHORUS:  // Chorus
      config->eff.chorus = value;
      synth->setChorus(config->eff.chorus);
      writeParamValue(1, "Chorus", config->eff.chorus);
      break;
    case CC_RANGE:  // Range
      config->osc.range = (range_t)value;
      synth->setOscRange(config->osc.range);
      writeParamString(1, "Range", rangeStr(config->osc.range));
      break;
    case CC_RESONANCE:  // Res
      config->filter.resonance = value;
      synth->setResonance(config->filter.resonance); 
      writeParamValue(1, "Resonance", config->filter.resonance);
      break;
    case CC_DECAY:  // Decay
      config->env.decay = value;
      synth->setDecay(config->env.decay);
      writeParamValue(1, "Decay", config->env.decay);
      break;
    case CC_LFO_RATE:  // Rate
      if (synth->getLFOSync() != LFOSYNC_OFF)
      {
        writeRow(1, "LFO is synced");
      }
      else
      {
        config->lfo.rate = value;
        synth->setLFORate(config->lfo.rate);
        writeParamValue(1, "Rate", config->lfo.rate);
      }
      break;
    case CC_AMP_ENV:  // Amp_Env_Amt
      config->amp.envAmt = value;
      synth->setAmpEnvAmt(config->amp.envAmt);
      writeParamValue(1, "Amp Env Amt", config->amp.envAmt);
      break;
    case CC_DELAY:  // Delay
      config->eff.delay = value;
      synth->setDelay(config->eff.delay);
      writeParamValue(1, "Delay", config->eff.delay);
      break;
    case CC_POLY:  // Poly
      config->osc.poly = (poly_t)value;
      if (config->osc.poly == UNISON)
        writeRow(1, "Unison");
      else
        writeParamValue(1, "Polyphony", polyVal(config->osc.poly));
      synth->setPoly(config->osc.poly);
      break;
    case CC_ENVAMT:  // Flt_Env_Amt
      config->filter.envAmt = value;
      synth->setFEnvAmt(config->filter.envAmt);
      writeParamValue(1, "Env amt", config->filter.envAmt);
      break;
    case CC_SUSTAIN:  // Sustain
      config->env.sustain = value;
      synth->setSustain(config->env.sustain);
      writeParamValue(1, "Sustain", config->env.sustain);
      break;
    case CC_LFO_PITCH:  // Pitch
      config->lfo.pitchAmt = value;
      synth->setLFOPitch(config->lfo.pitchAmt);
      writeParamValue(1, "LFO Pitch", config->lfo.pitchAmt);
      break;
    case CC_AMP_LFO:  // LFO_Amt
      config->amp.lfoAmt = value;
      synth->setAmpLFOAmt(config->amp.lfoAmt);
      writeParamValue(1, "Amp LFO Amt", config->amp.lfoAmt);
      break;
    case CC_REVERB:  // Reverb
      config->eff.reverb = value;
      synth->setReverb(config->eff.reverb);
      writeParamValue(1, "Reverb", config->eff.reverb);
      break;
    case CC_DETUNE:  // Detune
      config->osc.detune = value;
      synth->setDetune(config->osc.detune);
      writeParamValue(1, "Detune", config->osc.detune);
      break;
    case CC_HPF:  // HPF
      config->filter.hpfCutoff = value;
      synth->setHPFCutoff(config->filter.hpfCutoff);
      writeParamValue(1, "HPF Cutoff", config->filter.hpfCutoff);
      break;
    case CC_RELEASE:  // Release
      config->env.release = value;
      synth->setRelease(config->env.release);
      writeParamValue(1, "Release", config->env.release);
      break;
    case CC_LFO_FILTER:  // LFO_Flt
      config->lfo.filterAmt = value;
      synth->setLFOFilter(config->lfo.filterAmt);
      writeParamValue(1, "LFO Filter", config->lfo.filterAmt);
      break;
    case CC_VELSENS:  // Vel_sens
      config->amp.velSens = value;
      writeParamValue(1, "Vel Sens", config->amp.velSens);
      break;
    case CC_OVERDRIVE:  // Overdrive
      config->eff.overdrive = value;
      synth->setOverdrive(config->eff.overdrive);
      writeParamValue(1, "Overdrive", config->eff.overdrive);
      break;
  }
}

void setupControls()
{
  setupEncoders();
  pinMode(A8, INPUT); // analog input for volume pot
  setSelectLed(selected);
  lastupdatemillis = millis();
}

int avgrawvol = 0;

void checkControls(int loopcount)
{
  unsigned long curmillis = millis();
  
  if (loopcount % 20)
  {
//    check volume
    int raw = analogRead(A8);
    int v = movingaverage(raw, avgrawvol, 3);
    avgrawvol = v;
    synth->setVolume(v);
  }
  if (loopcount % 5)
    checkEncoders(curmillis);
  if (!summaryDone && curmillis - lastupdatemillis >= 2000)
  {
    showSelectSummary(selected, true);
    summaryDone = true;
  }
}

void showSelectSummary(int selected, bool addRow1)
{
  switch (selected)
  {
    case SELECT_OSC:
      showOscillatorSummary();
      if (addRow1)
        writeRow(1, ">> Oscillator <<");
      break;
    case SELECT_FLT:
      showFilterSummary();
      if (addRow1)
        writeRow(1, "  >> Filter <<  ");
      break;
    case SELECT_ENV:
      showEnvSummary();
      if (addRow1)
        writeRow(1, " >> Envelope << ");
      break;
    case SELECT_LFO:
      showLFOSummary();
      if (addRow1)
        writeRow(1, "   >> LFO <<   ");
      break;
    case SELECT_AMP:
      showAmpSummary();
      if (addRow1)
        writeRow(1, ">> Amplifier <<");
      break;
    case SELECT_EFF:
      showEffectsSummary();
      if (addRow1)
        writeRow(1, " >> Effects << ");
      break;
  }
}

void setSelectLed(int led)
{
  for (int i = 0; i < NUM_SELECT; ++i)
    expander->digitalWrite(selleds[i], led == i);
}

void setFrequencyMode(bool set, float f)
{
  DBG2("Setting frequency mode", set)
  isFrequencyMode = set;
  if (set)
  {
    droneFreq = f;
  }
}
