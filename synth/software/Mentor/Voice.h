#pragma once

#include "config.h"

class Voice
{
  private:
    AudioAmplifier              *mod;
    
    AudioSynthWaveformModulated *saw;
    AudioSynthWaveformModulated *sqr;
    AudioSynthWaveformModulated *tri;
    AudioSynthWaveformModulated *pulse;
    
    AudioMixer4                 *mix;
    AudioEffectEnvelope         *env;
    AudioEffectEnvelope         *raw;
    
    AudioEffectEnvelope         *filterEnv;
    AudioAmplifier              *filterPreamp;
    AudioFilterStateVariable    *filter;
    AudioMixer4                 *filterModMix;

    float freq;
    float adjf;
    int index; 
    int unison;
    float detuneCents = 0;
    int bendCents = 0;
    range_t range;
    
  public:
    Voice();
    ~Voice();

  void setupMod(AudioEffectMultiply *LFOPitch, AudioEffectMultiply *LFOFilter, AudioSynthWaveformDc *filterEnvAmt);
  void connectOutputs(AudioMixer4 *envMix, int i, AudioMixer4 *rawMix, int j);
  
  void setFreq(float f);
  void refreshFreq();
  void updateDetuneCents();
  void bend(int cents);
  void setLevel(ccInt lvl);
  void setShape(ccInt lvl);
  void setRange(range_t u);
  
  void setAttack(ccInt lvl);
  void setDecay(ccInt lvl);
  void setSustain(ccInt lvl);
  void setRelease(ccInt lvl);
  void setForceHold(bool force);
  
  void setCutoff(ccInt lvl);
  void setResonance(ccInt lvl);
  void modWheel(ccInt lvl);

  void playNote(int n, ccInt amp, int i, int u);
  void setFrequency(float f, int i, int u);
  void releaseNote();
  void stopNote();
  bool isActive();
  bool isReleased();
};
