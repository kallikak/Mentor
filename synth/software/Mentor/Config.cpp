#include "Config.h"
#include "LCD.h"

mentor_config defaultConfig = {
  {63, RANGE8FT, POLY8, 0},
  {127, 0, 0, 0},
  {0, 0, 127, 20},
  {TRIANGLE, 0, 0, 0},
  {0, 127, 0, 64},
  {0, 0, 0, 0},
  "Default"
};

mentor_config initConfig = {
  {63, RANGE8FT, POLY8, 0},
  {127, 0, 0, 0},
  {0, 0, 127, 0},
  {TRIANGLE, 0, 0, 0},
  {0, 127, 0, 64},
  {0, 0, 0, 0},
  "INIT"
};

mentor_config activeConfig = defaultConfig;

mentor_config *config = &activeConfig;
bool lfoSync = false;

void setDefaultConfig()
{
  activeConfig = defaultConfig;
}

waveshape getNextWaveshape(waveshape curshape, bool back)
{
  // LFO: SINE <-> TRIANGLE <-> SQUARE <-> SAWTOOTH <-> REV_SAWTOOTH <-> S_AND_H  
  switch (curshape)
  {
    case SINE:
      return back ? S_AND_H : TRIANGLE;
    case TRIANGLE:
      return back ? SINE : SQUARE;
    case SQUARE:
      return back ? TRIANGLE : SAWTOOTH;
    case SAWTOOTH:
      return back ? SQUARE : REV_SAWTOOTH;
    case REV_SAWTOOTH:
      return back ? SAWTOOTH : S_AND_H;
    case S_AND_H:
      return back ? REV_SAWTOOTH : SINE;
    default:
      return SAWTOOTH;
  }
}

const char *waveshapeStr(waveshape w, bool full)
{
  switch (w)
  {
    case SINE:
      return full ? "Sine" : "Sin";
    case TRIANGLE:
      return full ? "Triangle" : "Tri";
    case SQUARE:
      return full ? "Square" : "Sqr";
    case SAWTOOTH:
      return full ? "Sawtooth" : "Saw";
    case REV_SAWTOOTH:
      return full ? "Ramp" : "Ramp";
    case S_AND_H:
      return full ? "Sample and hold" : "S&H";
  }
  return "";
}

const char *waveshapeSettingStr(ccInt u, bool symbol)
{
  static Str100 str;
  if (u == 0)
    return "Tri   ";
  else if (u == 100)
    return "Saw   ";
  else if (u == 200)
    return "Sqr   ";
  else if (u < 200)
  {
    int triPct;
    int sawPct;
    int sqrPct;
    if (u < 100)
    {
      triPct = 100 - u;
      sawPct = u;
      if (!symbol)
        sprintf(str, "Tri %d%% Saw %d%%", triPct, sawPct);
      else
        sprintf(str, "\1%2d\2%2d", triPct, sawPct);
    }
    else
    {
      sawPct = 200 - u;
      sqrPct = u - 100;
      if (!symbol)
        sprintf(str, "Saw %d%% Sqr %d%%", sawPct, sqrPct);
      else
        sprintf(str, "\2%2d\3%2d", sawPct, sqrPct);
    }
  }
  else
  {
    // 200 => 50, 250 => 100
    int pulsewidth = u - 150;
    if (!symbol)
      sprintf(str, "Pulse %d%%", pulsewidth);
    else
      sprintf(str, "\3%3d%%", pulsewidth);
  }
  return str;
}

const char *rangeStr(range_t r)
{
  switch (r)
  {
    case RANGE32FT:
      return "32'";
    case RANGE16FT:
      return "16'";
    case RANGE8FT:
    default:
      return " 8'";
    case RANGE4FT:
      return " 4'";
  }
}

int polyVal(poly_t p)
{
  switch (p)
  {
    case POLY16:
      return 16;
    case POLY8:
    default:
      return 8;
    case POLY4:
      return 4;
    case POLY2:
      return 2;
    case UNISON:
      return 1;
  }
}

// how many voices playing in unison?
int getUnison(poly_t p)
{
  switch (p)
  {
    case POLY16:
      return 1;
    case POLY8:
      return 2;
    case POLY4:
      return 4;
    case POLY2:
    default:
      return 8;
    case UNISON:
      return 16;
  }
}

const char *clockFactorStrings[] = {"Off", "4", "3", "2", "1", "1/2", "1/3", "1/4"};

const float clockFactors[] = {1.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.5f, 0.3333333f, 0.25f};

float getLFOSyncFactor(LFOSync_t lfoSyncFactor)
{
  return clockFactors[(int)lfoSyncFactor];
}

const char *getLFOSyncFactorString(LFOSync_t lfoSyncFactor)
{
  return clockFactorStrings[(int)lfoSyncFactor];
}

float getRelativeEffectsLevel()
{
  effectsCfg *cfg = &config->eff;
  return (cfg->chorus + cfg->delay + cfg->reverb + cfg->overdrive) / (4 * 15);
}

void showOscillatorSummary()
{
  oscillatorCfg *cfg = &config->osc;
  Str100 str = {0};
  sprintf(str, "%3s %3s %2d %2d", 
    waveshapeSettingStr(cfg->shape, true), rangeStr(cfg->range), polyVal(cfg->poly), cfg->detune);
  writeRow(0, str);
}

void showFilterSummary()
{
  filterCfg *cfg = &config->filter;
  Str100 str = {0};
  sprintf(str, "%3d %3d %3d %3d", cfg->cutoff, cfg->resonance, cfg->envAmt, cfg->hpfCutoff);
  writeRow(0, str);
}

void showEnvSummary()
{
  envelopeCfg *cfg = &config->env;
  Str100 str = {0};
  sprintf(str, "%3d %3d %3d %3d", cfg->attack, cfg->decay, cfg->sustain, cfg->release);
  writeRow(0, str);
}

void showLFOSummary()
{
  lfoCfg *cfg = &config->lfo;
  Str100 str = {0};
  sprintf(str, "%3s %3d %3d %3d", waveshapeStr(cfg->shape, false), cfg->rate, cfg->pitchAmt, cfg->filterAmt);
  writeRow(0, str);
}

void showAmpSummary()
{
  ampCfg *cfg = &config->amp;
  Str100 str = {0};
  sprintf(str, "%3d %3d %3d %3d", cfg->gain, cfg->envAmt, cfg->lfoAmt, cfg->velSens);
  writeRow(0, str);
}

void showEffectsSummary()
{
  effectsCfg *cfg = &config->eff;
  Str100 str = {0};
  sprintf(str, "%3d %3d %3d %3d", cfg->chorus, cfg->delay, cfg->reverb, cfg->overdrive);
  writeRow(0, str);
}

void printOscConfig(oscillatorCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Osc: %s (%d), range: %s, poly: %d, detune: %d", 
    waveshapeSettingStr(cfg->shape, false), cfg->shape, rangeStr(cfg->range), polyVal(cfg->poly), cfg->detune);
  Serial.println(str);
}

void printAmpConfig(ampCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Amp: gain %d, envAmt %d, LFOAmt %d, vel sens %d", cfg->gain, cfg->envAmt, cfg->lfoAmt, cfg->velSens);
  Serial.println(str);
}

void printEnvelopeConfig(envelopeCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Envelope: attack %d, decay %d, sustain %d, release %d", cfg->attack, cfg->decay, cfg->sustain, cfg->release);
  Serial.println(str);
}

void printLFOConfig(lfoCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "LFO: rate %d %s, pitch: %d, filter: %d", cfg->rate, waveshapeStr(cfg->shape, true), 
    cfg->pitchAmt, cfg->filterAmt);
  Serial.println(str);
}

void printFilterConfig(filterCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Filter: cutoff %d, resonance %d, envamt %d, HPF %d", 
    cfg->cutoff, cfg->resonance, cfg->envAmt, cfg->hpfCutoff);
  Serial.println(str);
}

void printEffectsConfig(effectsCfg *cfg)
{
  Str100 str = {0};
  sprintf(str, "Effects: chorus %d, delay %d, reverb %d, overdrive %d", cfg->chorus, cfg->delay, cfg->reverb, cfg->overdrive);
  Serial.println(str);
}

void printConfig(mentor_config *cfg)
{
  if (!cfg)
    cfg = config;
  Serial.println("=============");
  Serial.println("Configuration");
  Serial.println("=============");
  Serial.println(cfg->name);
  printOscConfig(&cfg->osc);
  printFilterConfig(&cfg->filter);
  printEnvelopeConfig(&cfg->env);
  printLFOConfig(&cfg->lfo);
  printAmpConfig(&cfg->amp);
  printEffectsConfig(&cfg->eff);
  Serial.println(sizeof(*cfg));
  Serial.println("=============");
}
