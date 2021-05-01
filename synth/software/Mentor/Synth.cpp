#include <Audio.h>
#include "effect_platervbstereo.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       LFO;            //xy=115,139.3333282470703
AudioSynthWaveformDc     LFOPitchAmt;    //xy=117,242.33331298828125
AudioSynthWaveformDc     LFOFilterAmt;   //xy=117,319.3332977294922
AudioSynthWaveformDc     FilterEnvAmt;   //xy=127,390.3332977294922
AudioSynthWaveformDc     LFOOffset;      //xy=276,184.3333282470703
AudioEffectMultiply      LFOFilter;      //xy=288,312.3332977294922
AudioEffectMultiply      LFOPitch;       //xy=293,247.3332977294922
AudioMixer4              LFOMix;         //xy=478,154.3333282470703
AudioEffectMultiply      LFOAmp;         //xy=616,228.33331298828125
AudioMixer4              AmpMix;         //xy=616.333251953125,648.3333358764648
AudioAnalyzePeak         LFOPeak;        //xy=624,123.6666259765625
AudioAmplifier           HPFPreamp;      //xy=773.333251953125,646.3333358764648
AudioMixer4              LFOAmpMix;      //xy=830.3332824707031,247.33333587646484
AudioFilterStateVariable HPF;            //xy=941.333251953125,650.3333969116211
AudioAmplifier           EffectsPreamp;  //xy=963.6667175292969,387.6666259765625
AudioEffectEnsemble      Chorus;         //xy=1206.6666564941406,317.6666259765625
//AudioEffectReverb        Reverb;         //xy=1207.6666564941406,373.6666259765625
AudioEffectPlateReverb   Reverb;         //xy=1207.6666564941406,373.6666259765625
AudioEffectWaveshaper    Overdrive;      //xy=1207.6666564941406,650.6666564941406
AudioEffectDelay         DelayEff;       //xy=1212.6666564941406,563.6666564941406
AudioMixer4              DelayMix;       //xy=1215.6666564941406,441.6666259765625
AudioMixer4              EffectMixL;     //xy=1449.6665344238281,448.6666259765625
AudioMixer4              EffectMixR;     //xy=1450.6665344238281,535.6666564941406
AudioMixer4              FinalMixL;      //xy=1662.66650390625,240.6666259765625
AudioMixer4              FinalMixR;      //xy=1664.66650390625,314.66662216186523
AudioAmplifier           LevelL;         //xy=1801.66650390625,239.66664123535156
AudioAmplifier           LevelR;         //xy=1804.66650390625,312.66661834716797
AudioOutputUSB           USBOut;         //xy=1995.66650390625,224.66661834716797
AudioOutputI2S           i2sOut;         //xy=1998.66650390625,346.66661834716797
AudioConnection          patchCord1(LFO, 0, LFOPitch, 0);
AudioConnection          patchCord2(LFO, 0, LFOFilter, 0);
AudioConnection          patchCord3(LFO, 0, LFOMix, 0);
AudioConnection          patchCord4(LFOPitchAmt, 0, LFOPitch, 1);
AudioConnection          patchCord5(LFOFilterAmt, 0, LFOFilter, 1);
AudioConnection          patchCord6(LFOOffset, 0, LFOMix, 1);
AudioConnection          patchCord15(LFOMix, 0, LFOAmp, 1);
AudioConnection          patchCord16(LFOMix, LFOPeak);
AudioConnection          patchCord19(LFOAmp, 0, LFOAmpMix, 0);
AudioConnection          patchCord20(AmpMix, HPFPreamp);
AudioConnection          patchCord21(HPFPreamp, 0, HPF, 0);
AudioConnection          patchCord22(LFOAmpMix, 0, FinalMixL, 0);
AudioConnection          patchCord23(LFOAmpMix, 0, FinalMixR, 0);
AudioConnection          patchCord24(LFOAmpMix, EffectsPreamp);
AudioConnection          patchCord25(HPF, 2, LFOAmp, 0);
AudioConnection          patchCord26(HPF, 2, LFOAmpMix, 1);
AudioConnection          patchCord27(EffectsPreamp, Chorus);
AudioConnection          patchCord28(EffectsPreamp, Reverb);
AudioConnection          patchCord29(EffectsPreamp, 0, DelayMix, 0);
AudioConnection          patchCord30(EffectsPreamp, Overdrive);
AudioConnection          patchCord31(Chorus, 0, EffectMixL, 0);
AudioConnection          patchCord32(Chorus, 1, EffectMixR, 0);
AudioConnection          patchCord33(Reverb, 0, EffectMixL, 1);
AudioConnection          patchCord34(Reverb, 1, EffectMixR, 1);
AudioConnection          patchCord35(Overdrive, 0, EffectMixR, 3);
AudioConnection          patchCord36(Overdrive, 0, EffectMixL, 3);
AudioConnection          patchCord37(DelayEff, 0, DelayMix, 1);
AudioConnection          patchCord38(DelayMix, DelayEff);
AudioConnection          patchCord39(DelayMix, 0, EffectMixL, 2);
AudioConnection          patchCord40(DelayMix, 0, EffectMixR, 2);
AudioConnection          patchCord41(EffectMixL, 0, FinalMixL, 1);
AudioConnection          patchCord42(EffectMixR, 0, FinalMixR, 1);
AudioConnection          patchCord43(FinalMixL, LevelL);
AudioConnection          patchCord44(FinalMixR, LevelR);
AudioConnection          patchCord45(LevelL, 0, USBOut, 0);
AudioConnection          patchCord46(LevelL, 0, i2sOut, 0);
AudioConnection          patchCord48(LevelR, 0, USBOut, 1);
AudioConnection          patchCord49(LevelR, 0, i2sOut, 1);
AudioControlSGTL5000     sgtl5000;       //xy=1997.66650390625,283.6666259765625
AudioAnalyzePeak         TestPeak;       //xy=1997.000244140625,165.33333587646484
AudioConnection          patchCord47(LevelL, TestPeak);
// GUItool: end automatically generated code

// 16 voices => 4 mixers for each voice output
// and then 4 env go to envCombine, 4 raw go to rawCombine
// and the two outputs go to AmpMix

// Do it statically because of some question marks over the audio library's handling of dynamically created connection objects...

AudioMixer4 env1, env2, env3, env4;
AudioMixer4 raw1, raw2, raw3, raw4;
AudioMixer4 envMix[4] = {env1, env2, env3, env4};
AudioMixer4 rawMix[4] = {raw1, raw2, raw3, raw4};
AudioMixer4 envCombine;
AudioMixer4 rawCombine;

AudioConnection          patchCord50(env1, 0, envCombine, 0);
AudioConnection          patchCord51(env2, 0, envCombine, 1);
AudioConnection          patchCord52(env3, 0, envCombine, 2);
AudioConnection          patchCord53(env4, 0, envCombine, 3);
AudioConnection          patchCord54(raw1, 0, rawCombine, 0);
AudioConnection          patchCord55(raw2, 0, rawCombine, 1);
AudioConnection          patchCord56(raw3, 0, rawCombine, 2);
AudioConnection          patchCord57(raw4, 0, rawCombine, 3);

AudioConnection          patchCord58(envCombine, 0, AmpMix, 0);
AudioConnection          patchCord59(rawCombine, 0, AmpMix, 2);
    
float v = 0;
void monitorPeak()
{
  if (TestPeak.available())
  {
    float newv = TestPeak.read();
    if (abs(newv - v) > 0.02)
    {
      Serial.print("### ");
      Serial.println(v);
      v = newv;
    }
  }
}

#include <Math.h>
#include "Synth.h"
#include "Voice.h"
#include "Utility.h"

// I have removed support for 32 voices though some of the code below remains a little more complex
// because of the history.
// 16 voices is much easier to manage sonically.
Synth::Synth()
{
  int i;
  
  for (i = 0; i < POLYPHONY; ++i)
  {
    voice[i] = new Voice();
    voice[i]->setupMod(&LFOPitch, &LFOFilter, &FilterEnvAmt);
    int m = (int)(i / 4);
    int n = i % 4;
    // voices 1-4 connect to envMix/rawMix 1 inputs 1-4
    // voices 5-8 connect to envMix/rawMix 2 inputs 1-4
    // voices 9-12 connect to envMix/rawMix 3 inputs 1-4
    // voices 13-16 connect to envMix/rawMix 4 inputs 1-4
    voice[i]->connectOutputs(&envMix[m], n, &rawMix[m], n);
    
    envMix[m].gain(n, 0.25);
    rawMix[m].gain(n, 0.25);
  }
  
  AudioMemory(600);
  for (i = 0; i < 4; ++i)
  {
    envCombine.gain(i, 0.25);
    rawCombine.gain(i, 0.25);
    EffectMixL.gain(i, 0);
    EffectMixR.gain(i, 0);
  }
  HPFPreamp.gain(1);
  HPF.frequency(0);
  HPF.resonance(0);
  LFOAmpMix.gain(0, 0);
  LFOAmpMix.gain(1, 0.5);
  EffectsPreamp.gain(1);
  FinalMixL.gain(0, 0.5);
  FinalMixL.gain(1, 0.5);
  FinalMixR.gain(0, 0.5);
  FinalMixR.gain(1, 0.5);
  LevelL.gain(1.0);
  LevelR.gain(1.0);
  
  LFO.amplitude(1);
  LFOOffset.amplitude(1);
  LFOMix.gain(0, 0.5);
  LFOMix.gain(1, 0.5);
  
  Chorus.lfoRate(20);
  DelayEff.delay(0, 500);
  DelayMix.gain(0, 0.75);
  DelayMix.gain(1, 0.5);
//  float overdriveShaper[17] = 
//  { 
//    -0.588, -0.579, -0.549, -0.488,-0.396, -0.320,  -0.228, -0.122,
//    0, 0.122, 0.228, 0.320, 0.396, 0.488, 0.549, 0.579, 0.588
//  };
  float overdriveShaper[9] = 
  { 
    -0.5, -0.45, -0.4, 0.2, 0, 0.2, 0.4, 0.45, 0.5
//    -0.5, -0.45, -0.4, 0.2, 0, 0.2, 0.4, 0.45, 0.5
//    -0.7, -0.6, -0.5, 0.2, 0, 0.2, 0.5, 0.6, 0.7
  };
//  for (int i = 0; i < 9; ++i)
//    overdriveShaper[i] *= 0.5;
  Overdrive.shape(overdriveShaper, 9);
//  Freeverb.roomsize(0.7);
//  Freeverb.damping(0.3);
//  Reverb.reverbTime(1.5);
  Reverb.size(1.0);
  Reverb.lowpass(0.5);

  calcDetunings();
  
  sgtl5000.enable();
  sgtl5000.volume(0.5);
  sgtl5000.enhanceBassEnable();
  sgtl5000.autoVolumeControl(0,2,0,-18.0,200,2000);  //maxGain,response,hard limit,threshold,attack, decay
  sgtl5000.autoVolumeEnable();
}

Synth::~Synth()
{
  for (int i = 0; i < POLYPHONY; ++i)
    delete voice[i];
}

void Synth::loadConfig(mentor_config *cfg)
{
  setDetune(cfg->osc.detune);
  setWaveShape(cfg->osc.shape);
  setOscRange(cfg->osc.range);

  setAttack(cfg->env.attack);
  setDecay(cfg->env.decay);
  setSustain(cfg->env.sustain);
  setRelease(cfg->env.release);

  setCutoff(cfg->filter.cutoff);
  setResonance(cfg->filter.resonance);  
  setFEnvAmt(cfg->filter.envAmt);
  setHPFCutoff(cfg->filter.hpfCutoff);
  
  setLFOShape(cfg->lfo.shape);
  setLFORate(cfg->lfo.rate);
  setLFOPitch(cfg->lfo.pitchAmt);
  setLFOFilter(cfg->lfo.filterAmt);
    
  setGain(cfg->amp.gain);
  setAmpEnvAmt(cfg->amp.envAmt);
  setAmpLFOAmt(cfg->amp.lfoAmt);

  setChorus(cfg->eff.chorus);
  setDelay(cfg->eff.delay);
  setReverb(cfg->eff.reverb);
  setOverdrive(cfg->eff.overdrive);

  printConfig(cfg);
}

bool Synth::checkFlashLFO()
{
  if (LFOPeak.available())
  {
    float p = LFOPeak.read();
    bool v = p >= 0.5;
//    Serial.println(p);
    if (v != lastLFO)
    {
      lastLFO = v;
      return true;
    }
  }
  return false;
}

void Synth::playNote(notedata n, int i, int unison)
{
  Voice *v = voice[n.voice];
  v->playNote(n.pitch, n.velocity, i, unison);
#if DEBUG_SERIAL
  Serial.print("Playing note ");
  Serial.print(n.pitch);
  Serial.print(" on voice ");
  Serial.println(n.voice + 1);
#endif  
}

void Synth::setFrequency(float f)
{
  int unison = getUnison(config->osc.poly);
  for (int i = 0; i < unison; ++i)
  {
//    DBG2(Setting frequency on voice, i)
    voice[i]->setFrequency(f, i, unison);
  }
#if DEBUG_SERIAL
  Serial.print("Setting frequency ");
  Serial.print(f);
  Serial.println(" on all voices");
#endif  
}

void Synth::liftNote(notedata n)
{
  Voice *v = voice[n.voice];
#if DEBUG_SERIAL
  Serial.print("Releasing note on voice ");
  Serial.println(n.voice + 1);
#endif  
  v->releaseNote();
}

void Synth::allNotesOff()
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->stopNote();
  }
}

bool Synth::isVoiceActive(int i)
{
  if (i >= 0 && i < POLYPHONY)
    return voice[i]->isActive();
  return false;
}

bool Synth::isVoiceReleasing(int i)
{
  if (i >= 0 && i < POLYPHONY)
    return voice[i]->isActive() && voice[i]->isReleased();
  return false;
}

void Synth::setWaveShape(ccInt u)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->setShape(u);
  }
}

void Synth::setOscRange(range_t u)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->setRange(u);
    voice[i]->refreshFreq();
  }
}

// detune from -c to c according to index among voices
void Synth::calcDetunings()
{
  int maxcents = config->osc.detune;
  int unison = getUnison(config->osc.poly);
  float delta = 2.0 * maxcents / (unison - 1);
  for (int i = 0; i < unison; ++i)
  {
    detunings[i] = unison == 1 ? 0 : i * delta - maxcents;
#if DEBUG_SERIAL
    char str[100];
    sprintf(str, "\n\tmaxcents: %d, voice: %d/%d, result: %.2f", maxcents, i, unison, detunings[i]);
    Serial.print(str);
#endif    
  }
}

void Synth::setDetune(int u)
{
  calcDetunings();
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->refreshFreq();
  }
#if DEBUG_SERIAL
  Serial.println();
  Serial.print("Detunings: ");
  Serial.print(config->osc.detune);
  Serial.print(" across ");
  Serial.print(getUnison(config->osc.poly));
  Serial.print(" voices [");
  for (int i = 0; i < getUnison(config->osc.poly); ++i)
  {
    Serial.print(detunings[i]);
    Serial.print(" ");
  }
  Serial.println("]");
#endif  
}

float Synth::getDetuning(int i)
{
  return detunings[i];
}

void Synth::setPoly(poly_t u)
{
  calcDetunings();
}

void Synth::bend(int cents)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->bend(cents);
    voice[i]->refreshFreq();
  }
}

void Synth::setCutoff(ccInt u)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->setCutoff(u);
  }
}

void Synth::setResonance(ccInt u)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->setResonance(u);
  }
}

void Synth::setFEnvAmt(ccInt u)
{
  Serial.println(constrainSignedCC(u) / 64.0);
  FilterEnvAmt.amplitude(constrainSignedCC(u) / 64.0);
}

void Synth::setHPFCutoff(ccInt u)
{
  // exp curve from about 25Hz to 6000 Hz
  float co = 25 * exp(5.5 * u / 127.0);
  HPF.frequency(co);
  HPF.octaveControl(log2f(12000.0 / (float)co));
}

void Synth::modWheel(ccInt u)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->modWheel(u);
  }
}

void Synth::setAttack(ccInt u)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->setAttack(u);
  }
}

void Synth::setDecay(ccInt u)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->setDecay(u);
  }
}

void Synth::setSustain(ccInt u)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->setSustain(u);
  }
}

void Synth::setRelease(ccInt u)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->setRelease(u);
  }
}

void Synth::setForceHold(bool force)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    voice[i]->setForceHold(force);
  }
}

void Synth::setLFOShape(waveshape w)
{
  if (lfoSync != LFOSYNC_OFF)
    setLFOFrequency(config->lfo.shape == WAVEFORM_SAMPLE_HOLD ? 2 * lfoFreq : lfoFreq, false);
  LFO.begin(w);
}

void Synth::setLFORate(ccInt u)
{
// convert log scale 1 to 127 to a linear rate
// f(1) = 0.5Hz
// f(127) = 40Hz
// f(x) = Math.exp(x / 25) / 4
// ?+ 100{]_,E^(_/25) / 4}
//   float f = exp(u / 25.0) / 2.0;
// # Exp ([0 64 127]/19) / 10
// [ 1/10 2.9032649822423 79.967910329469 ]
  float f = exp(u / 19.0) / 10.0;
  setLFOFrequency(f, false);
}

void Synth::setLFOFrequency(float f, bool resetPhase)
{
  lfoFreq = f;
  float c = getLFOSyncFactor(lfoSync);
#ifdef SERIAL_DEBUG  
  Serial.print("LFO freq = ");
  Serial.print(f * c);
  Serial.print(" = ");
  Serial.print(f);
  Serial.print(" from ");
  Serial.print(f);
  Serial.print(" * ");
  Serial.println(c);
#endif  
  LFO.frequency(config->lfo.shape == WAVEFORM_SAMPLE_HOLD ? 2 * f * c : f * c);
  if (resetPhase)
    LFO.phase(0);
}

void Synth::setLFOSync(LFOSync_t factor)
{
  lfoSync = factor;
  setLFOFrequency(lfoFreq, false);
}

LFOSync_t Synth::getLFOSync()
{
  return lfoSync;
}

void Synth::setLFOPitch(ccInt u)
{
  LFOPitchAmt.amplitude(u / 127.0);
}

void Synth::setLFOFilter(ccInt u)
{
  LFOFilterAmt.amplitude(u / 127.0);
}

void Synth::setGain(ccInt u)
{
  float rawGain = constrainCC(u) / 127.0 / 2;
  AmpMix.gain(2, rawGain);
}

void Synth::setAmpEnvAmt(ccInt u)
{
  float envGain = constrainCC(u) / 127.0 / 2;
  AmpMix.gain(0, envGain);
}

void Synth::setAmpLFOAmt(ccInt u)
{
  float lfoAmp = constrainCC(u) / 127.0;
  LFOAmpMix.gain(0, lfoAmp);
  LFOAmpMix.gain(1, 1 - lfoAmp);
}

void Synth::setChorus(int u)
{
  float v = u / 15.0 / 2;
  EffectMixL.gain(0, v);
  EffectMixR.gain(0, v);
  adjustLevelForEffects();
}

void Synth::setDelay(int u)
{
  float v = u / 15.0 / 2;
  EffectMixL.gain(2, v);
  EffectMixR.gain(2, v);
  adjustLevelForEffects();
}

void Synth::setReverb(int u)
{
  float v = u / 15.0 / 2;
  EffectMixL.gain(1, v);
  EffectMixR.gain(1, v);
  adjustLevelForEffects();
}

void Synth::setOverdrive(int u)
{
  float v = u / 15.0;
  EffectMixL.gain(3, v);
  EffectMixR.gain(3, v);
  adjustLevelForEffects();
}

void Synth::adjustLevelForEffects()
{
  float lvl = getRelativeEffectsLevel();
  if (lvl > 0.4)
  {
    FinalMixL.gain(0, 1 - lvl);
    FinalMixL.gain(1, lvl);
    FinalMixR.gain(0, 1 - lvl);
    FinalMixR.gain(1, lvl);
  }
}

#define LEVEL_SCALE 30

void Synth::setVolume(int u)
{
  // f () k x ~ x / (1 + k * (1 - x / 127))
  int k = 2;
  u >>= 3;
  int log_u = max(0, min(127, 1.0 * u / (1 + k * (1 - u / 127.0))));
//  Serial.print(u);Serial.print(" => ");Serial.println(log_u);
  LevelL.gain(LEVEL_SCALE * log_u / 127.0); 
  LevelR.gain(LEVEL_SCALE * log_u / 127.0); 
}
