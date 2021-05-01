#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <Math.h>

#include "Synth.h"
#include "Voice.h"
#include "Utility.h"

#define ATTACK_FACTOR 4
#define DECAY_FACTOR 4
#define RELEASE_FACTOR 12

#define MAX_FREQ 22050

#include "sawtoothWave.h"
static int16_t pulseWavetable[45][257] = {{0}, {0}};

// pw goes from 50 to 100
void updatePulseWidth(int u)
{
  float pw = max(50, min(100, u)) / 100.0;
  // s[x] - s[x + d] + s[d]
  // PW 0 => 50%, PW 127 => 100%
  int dt = round(256 * pw);
  int offset = sawtoothWavetable[0][0];
  for (int i = 0; i < 45; ++i)
  {
    int j = -dt;
    for (int k = 0; k < 257; ++k, ++j)
    {
      int32_t s = sawtoothWavetable[i][k];
      int32_t t = -sawtoothWavetable[i][j];
      pulseWavetable[i][k] = min(32767, max(-32768, s + t + offset));
    }
  }
}

Voice::Voice()
{
  // create the audio units
  mod = new AudioAmplifier();

  saw = new AudioSynthWaveformModulated();
  sqr = new AudioSynthWaveformModulated();
  tri = new AudioSynthWaveformModulated();
  pulse = new AudioSynthWaveformModulated();

  mix = new AudioMixer4();
  raw = new AudioEffectEnvelope();
  env = new AudioEffectEnvelope();

  filterPreamp = new AudioAmplifier();
  filterEnv = new AudioEffectEnvelope();
  filter = new AudioFilterStateVariable();
  filterModMix = new AudioMixer4();

  // create the patch connections
  // no need to keep track - just create once and they'll die with the application

  new AudioConnection(*mod, *tri);
  new AudioConnection(*mod, *saw);
  new AudioConnection(*mod, *sqr);
  new AudioConnection(*mod, *pulse);
  new AudioConnection(*tri, 0, *mix, 0);
  new AudioConnection(*saw, 0, *mix, 1);
  new AudioConnection(*sqr, 0, *mix, 2);
  new AudioConnection(*pulse, 0, *mix, 3);
  new AudioConnection(*mix, *filterPreamp);
  new AudioConnection(*filterPreamp, *filter);
  new AudioConnection(*filter, 0, *env, 0);
  new AudioConnection(*filter, 0, *raw, 0);
  new AudioConnection(*filterEnv, 0, *filterModMix, 1);
  new AudioConnection(*filterModMix, 0, *filter, 1);
  // set some values

  saw->frequencyModulation(1);
  sqr->frequencyModulation(1);
  tri->frequencyModulation(1);
  pulse->frequencyModulation(1);
  saw->begin(WAVEFORM_BANDLIMIT_SAWTOOTH);
  sqr->begin(WAVEFORM_BANDLIMIT_SQUARE);
  tri->begin(WAVEFORM_TRIANGLE);
  updatePulseWidth(50);
  pulse->begin(WAVEFORM_ARBITRARY);
//  sine->begin(WAVEFORM_SINE);
  saw->amplitude(1.0);
  sqr->amplitude(1.0);
  tri ->amplitude(1.0);
  pulse ->amplitude(1.0);
  mix->gain(0, 0.0);
  mix->gain(1, 1.0);
  mix->gain(2, 0.0);
  mix->gain(3, 0.0);
  filterPreamp->gain(0.15);
  filterModMix->gain(0, 0.5);
  filterModMix->gain(1, 0.5);
  filter->octaveControl(4);
  raw->attack(0.0);
  raw->decay(0.0);
  raw->sustain(1.0);
  raw->release(0.0);
  freq = 0;
  adjf = 0;
}

void Voice::setupMod(AudioEffectMultiply *LFOPitch, AudioEffectMultiply *LFOFilter, AudioSynthWaveformDc *filterEnvAmt)
{
  new AudioConnection(*filterEnvAmt, *filterEnv);
  new AudioConnection(*LFOFilter, 0, *filterModMix, 0);
  new AudioConnection(*LFOPitch, *mod);
}

void Voice::connectOutputs(AudioMixer4 *envMix, int i, AudioMixer4 *rawMix, int j)
{
  new AudioConnection(*env, 0, *envMix, i);
  new AudioConnection(*raw, 0, *rawMix, j);
}

void Voice::setFreq(float f)
{
  float savef = adjf;
  updateDetuneCents();
  freq = f;
  adjf = f;
  switch (range)
  {
    case RANGE4FT:
      adjf *= 2;
      break;
    case RANGE16FT:
      adjf /= 2;
      break;
    case RANGE32FT:
      adjf /= 4;
      break;
    default:
      break;
  }
  adjf = calcDetune(adjf, bendCents + detuneCents);
  saw->frequency(adjf);
  // TODO there is a bug in the bandlimited square wave for high frequency
  // Until I can find and fix it, just fake it
  // Seems to happen when 4 notes are played and possibly connected to having mod input
#define SWAP_FREQ 3000  
  if (savef < SWAP_FREQ && adjf > SWAP_FREQ) {
    sqr->begin(WAVEFORM_TRIANGLE); Serial.println("Swapping in triangle"); }
  else if (savef > SWAP_FREQ && adjf < SWAP_FREQ) {
    sqr->begin(WAVEFORM_BANDLIMIT_SQUARE); Serial.println("Swapping out triangle"); }
  sqr->frequency(adjf);
  tri->frequency(adjf);
  
  int idx = max(min(freqToNote(adjf) / 3 + 1, 44), 0); // protect against overflow & underflow
  pulse->arbitraryWaveform(pulseWavetable[idx], MAX_FREQ);
  pulse->frequency(adjf);
}

void Voice::updateDetuneCents()
{
  detuneCents = synth->getDetuning(index - 1);
}

void Voice::bend(int cents)
{
  bendCents = cents;
}

void Voice::refreshFreq()
{
  if (freq > 0)
    setFreq(freq);
}

void Voice::setLevel(ccInt lvl)
{
  float v = constrainCC(lvl) / 127.0;
  saw->amplitude(v);
  sqr->amplitude(v);
  tri->amplitude(v);
  pulse->amplitude(v);
}

void Voice::setShape(ccInt u)
{
  if (u > 200)
  {
    mix->gain(0, 0);
    mix->gain(1, 0);
    mix->gain(2, 0);
    updatePulseWidth(u - 150);
    mix->gain(3, 1);
  }
  else
  {
    updatePulseWidth(0);
    float triPct = 0;
    float sawPct = 0;
    float sqrPct = 0;
    if (u == 0)
      triPct = 1;
    else if (u == 100)
      sawPct = 1;
    else if (u == 200)
      sqrPct = 1;
    else if (u < 200)
    {
      if (u < 100)
      {
        triPct = (100 - u) / 100.0;
        sawPct = u / 100.0;
      }
      else
      {
        sawPct = (200 - u) / 100.0;
        sqrPct = (u - 100) / 100.0;
      } 
    }
    mix->gain(0, triPct);
    mix->gain(1, sawPct);
    mix->gain(2, sqrPct);
    mix->gain(3, 0);
  }
}

void Voice::setRange(range_t u)
{
  range = u;
}

void Voice::setAttack(ccInt lvl)
{
  float ams = constrainCC(lvl) / 127.0 * 11880 / ATTACK_FACTOR;
  env->attack(ams);
  filterEnv->attack(ams);
}

void Voice::setDecay(ccInt lvl)
{
  float dms = constrainCC(lvl) / 127.0 * 11880 / DECAY_FACTOR;
  env->decay(dms);
  filterEnv->decay(dms);
}

void Voice::setSustain(ccInt lvl)
{
  ccInt s = constrainCC(lvl);
  env->sustain(s / 127.0);
  filterEnv->sustain(s / 127.0);
}

void Voice::setRelease(ccInt lvl)
{
  float rms = constrainCC(lvl) / 127.0 * 11880 / RELEASE_FACTOR;
  env->release(rms);
  filterEnv->release(rms);
}

void Voice::setForceHold(bool force)
{
  env->setForceHold(force);
  filterEnv->setForceHold(force);
}

void Voice::setCutoff(ccInt u)
{
  // exp curve from about 25Hz to 6000Hz
  float co = 25 * exp(5.5 * u / 127.0);
  filter->frequency(co);
  filter->octaveControl(log2f(12000.0 / (float)co));
}

void Voice::setResonance(ccInt u)
{
  filter->resonance(u * 4.3 / 127.0 + 0.7);
}

void Voice::modWheel(ccInt lvl)
{
  setCutoff(constrainCC(config->filter.cutoff + lvl));
}  

void Voice::setFrequency(float f, int i, int u)
{
  unison = u;
  index = i;
  setFreq(f);
}

void Voice::playNote(int n, ccInt amp, int i, int u)
{
  unison = u;
  index = i;
  setFreq(noteToFreq(n));
  // fully sensitive implies scale from 0 to 127
  // decrease sensitivity, scale from d to 127 with increasing d
  ccInt minSens = 127 - config->amp.velSens;
  float relamp = amp / 127.0;
  setLevel(minSens + relamp * (127 - minSens));
  saw->sync();
  sqr->sync();
  tri->sync();
  pulse->sync();
  env->noteOn();
  raw->noteOn();
  filterEnv->noteOn();
}

void Voice::releaseNote()
{
  env->noteOff();
  raw->noteOff();
  filterEnv->noteOff();
  index = 0;
  freq = 0;
}

void Voice::stopNote()
{
  env->release(1);
  raw->release(1);
  filterEnv->release(1);
  releaseNote();
  setRelease(config->env.release);
}

bool Voice::isActive()
{
  return (config->amp.envAmt > 0 && env->isActive()) || 
    (config->amp.gain > 0 && raw->isActive());
}

bool Voice::isReleased()
{
  return freq == 0;
}

/*
// GUItool: begin automatically generated code
AudioAmplifier           voiceMod;       //xy=89,215
AudioSynthWaveformModulated voiceSaw;       //xy=270,197
AudioSynthWaveformModulated voiceSqr;       //xy=270,238
AudioSynthWaveformModulated voiceTri;       //xy=271,156
AudioSynthWaveformModulated voicePulse;     //xy=271,281
AudioEffectEnvelope      voiceFilterEnv; //xy=420,321
AudioMixer4              voiceMix;       //xy=424,220
AudioAmplifier           voiceFilterPreamp; //xy=597,220
AudioMixer4              voiceFilterModMix; //xy=603,327
AudioOutputI2S           i2s1;           //xy=750,94
AudioFilterStateVariable voiceFilter;    //xy=783,269
AudioEffectEnvelope      voiceEnv;       //xy=936,231
AudioAmplifier           voiceRaw;       //xy=938,278
AudioConnection          patchCord1(voiceMod, 0, voiceTri, 0);
AudioConnection          patchCord2(voiceMod, 0, voiceSaw, 0);
AudioConnection          patchCord3(voiceMod, 0, voiceSqr, 0);
AudioConnection          patchCord4(voiceMod, 0, voicePulse, 0);
AudioConnection          patchCord5(voiceSaw, 0, voiceMix, 1);
AudioConnection          patchCord6(voiceSqr, 0, voiceMix, 2);
AudioConnection          patchCord7(voiceTri, 0, voiceMix, 0);
AudioConnection          patchCord8(voiceTri, 0, voiceMix, 0);
AudioConnection          patchCord9(voicePulse, 0, voiceMix, 3);
AudioConnection          patchCord10(voiceFilterEnv, 0, voiceFilterModMix, 1);
AudioConnection          patchCord11(voiceMix, voiceFilterPreamp);
AudioConnection          patchCord12(voiceFilterPreamp, 0, voiceFilter, 0);
AudioConnection          patchCord13(voiceFilterModMix, 0, voiceFilter, 1);
AudioConnection          patchCord14(voiceFilter, 0, voiceEnv, 0);
AudioConnection          patchCord15(voiceFilter, 0, voiceRaw, 0);
// GUItool: end automatically generated code
*/
