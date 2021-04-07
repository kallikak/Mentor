#pragma once

#include "Config.h"
#include "Voice.h"
#include "MIDIManager.h"

#define MIXERS        (POLYPHONY >> 2)
#define COMBINEMIXERS (MIXERS >> 2)

#define NUM_CLOCK_FACTORS 8

extern const char *clockFactors[];

class Synth
{
  private:
    Voice* voice[POLYPHONY];
    bool lastLFO = false;
    float lfoFreq = 0;
    LFOSync_t lfoSync = LFOSYNC_OFF;
    float detunings[POLYPHONY];
    AudioMixer4 *envMix[MIXERS];
    AudioMixer4 *rawMix[MIXERS];
    AudioMixer4 *envCombine[COMBINEMIXERS];
    AudioMixer4 *rawCombine[COMBINEMIXERS];
    
    void adjustLevelForEffects();
    void calcDetunings();
    
  public:
    Synth();
    ~Synth();

    bool checkFlashLFO();
    
    void loadConfig(mentor_config *cfg);

    void setWaveShape(ccInt u);
    void setOscRange(range_t u);
    void setDetune(int u);
    void setPoly(poly_t u);
    void bend(int cents);
    float getDetuning(int i);

    void setCutoff(ccInt u);
    void setResonance(ccInt u);
    void setFEnvAmt(ccInt u);
    void setHPFCutoff(ccInt u);
    void modWheel(ccInt u);

    void setAttack(ccInt u);
    void setDecay(ccInt u);
    void setSustain(ccInt u);
    void setRelease(ccInt u);
    void setForceHold(bool force);

    void setLFOShape(waveshape w);
    void setLFORate(ccInt u);
    void setLFOFrequency(float f, bool resetPhase);
    void setLFOPitch(ccInt u);
    void setLFOFilter(ccInt u);
    void setLFOSync(LFOSync_t factor);
    LFOSync_t getLFOSync();
    
    void setGain(ccInt u);
    void setAmpEnvAmt(ccInt u);
    void setAmpLFOAmt(ccInt u);
    
    void setChorus(int u);
    void setDelay(int u);
    void setReverb(int u);
    void setOverdrive(int u);
    
    void playNote(notedata n, int i, int unison);
    void setFrequency(float f);
    void liftNote(notedata n);
    void allNotesOff();
    bool isVoiceActive(int i);
    bool isVoiceReleasing(int i);
    
    void setVolume(int u);
};
