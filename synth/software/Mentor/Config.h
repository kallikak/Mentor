#pragma once

#define DEBUG_SERIAL 0

#define DBG(v) {Serial.print(#v " = ");Serial.println(v);}
#define DBG2(s,v) {Serial.print(#s " = ");Serial.println(v);}
#define DBG3(s,v) {Serial.print(#s ", " #v " = ");Serial.println(v);}
#define DBGLINE {Serial.println("----------------");}

#include <Audio.h>

// POLYPHONY can be 32 or 16
#define USEPOLY32 1

#if USEPOLY32
#define POLYPHONY     32
#else
#define POLYPHONY     16
#endif

typedef char Str8[9];
typedef char Str12[13];
typedef char Str16[17];
typedef char Str20[21];
typedef char Str100[101];

class Synth;

extern Synth *synth;

/*
#define WAVEFORM_SINE              0
#define WAVEFORM_SAWTOOTH          1
#define WAVEFORM_SQUARE            2
#define WAVEFORM_TRIANGLE          3
#define WAVEFORM_ARBITRARY         4
#define WAVEFORM_PULSE             5
#define WAVEFORM_SAWTOOTH_REVERSE  6
#define WAVEFORM_SAMPLE_HOLD       7
#define WAVEFORM_TRIANGLE_VARIABLE 8
*/

typedef enum 
{
  SINE = WAVEFORM_SINE,
  TRIANGLE = WAVEFORM_TRIANGLE,
  SQUARE = WAVEFORM_SQUARE,
  SAWTOOTH = WAVEFORM_SAWTOOTH,
  REV_SAWTOOTH = WAVEFORM_SAWTOOTH_REVERSE,
  S_AND_H = WAVEFORM_SAMPLE_HOLD
} waveshape;

typedef enum 
{
  RANGE32FT = 0,
  RANGE16FT = 1,
  RANGE8FT = 2,
  RANGE4FT = 3
} range_t;

typedef enum 
{
  UNISON = 0,
  POLY2 = 1,
  POLY4 = 2,
  POLY8 = 3,
  POLY16 = 4,
#if POLYPHONY==32
  POLY32 = 5
#endif  
} poly_t;

#if POLYPHONY==32
#define POLYMAX POLY32
#else
#define POLYMAX POLY16
#endif

typedef enum {
  LFOSYNC_OFF,
  LFOSYNC_4,
  LFOSYNC_3,
  LFOSYNC_2,
  LFOSYNC_1,
  LFOSYNC_1_2,
  LFOSYNC_1_3,
  LFOSYNC_1_4
} LFOSync_t;
  
typedef int ccInt;  // for 0 to 127 control values

typedef struct
{
  ccInt shape;      // Sine -> Sawtooth -> Square
  range_t range : 2;        
  poly_t poly : 3;
  byte detune : 6;
} oscillatorCfg;

typedef struct
{
  ccInt cutoff;
  ccInt resonance;
  ccInt envAmt;   // from -64 to 63
  ccInt hpfCutoff;
} filterCfg;

typedef struct
{
  ccInt attack;
  ccInt decay;
  ccInt sustain;
  ccInt release;
} envelopeCfg;

typedef struct
{
  waveshape shape;
  ccInt rate;
  ccInt pitchAmt;
  ccInt filterAmt;
} lfoCfg;

typedef struct
{
  ccInt gain;
  ccInt envAmt;
  ccInt lfoAmt;
  ccInt velSens;
} ampCfg;

typedef struct
{
  byte chorus : 4;
  byte delay : 4;
  byte reverb : 4;
  byte overdrive : 4;
} effectsCfg;

typedef struct
{
  oscillatorCfg osc;
  filterCfg     filter; 
  envelopeCfg   env;
  lfoCfg        lfo;
  ampCfg        amp;
  effectsCfg    eff;
  Str12         name;
} mentor_config, *config_ptr;

extern config_ptr config;
extern mentor_config initConfig;

void setDefaultConfig();

waveshape getNextWaveshape(waveshape curshape, bool back);
const char *waveshapeStr(waveshape w, bool full);
const char *waveshapeSettingStr(ccInt u, bool symbol);
const char *rangeStr(range_t r);
int getUnison(poly_t p);
int polyVal(poly_t p);

float getRelativeEffectsLevel();

const char *getLFOSyncFactorString(LFOSync_t lfoSyncFactor);
float getLFOSyncFactor(LFOSync_t lfoSyncFactor);

void showOscillatorSummary();
void showFilterSummary();
void showEnvSummary();
void showLFOSummary();
void showAmpSummary();
void showEffectsSummary();

void printOscConfig(oscillatorCfg *cfg);
void printAmpConfig(ampCfg *cfg);
void printEnvelopeConfig(envelopeCfg *cfg);
void printLFOConfig(lfoCfg *cfg);
void printFilterConfig(filterCfg *cfg);
void printEffectsConfig(effectsCfg *cfg);
void printConfig(config_ptr cfg);
