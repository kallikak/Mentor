#include <stdio.h>
#include <EEPROM.h>

#include "Config.h"
#include "Presets.h"
#include "Synth.h"
#include "Utility.h"

static int lastIndex = 0;
static int lastUserIndex = 0;

mentor_config preset1 = {
  {57, RANGE8FT, POLY8, 10},
  {100, 0, 50, 0},
  {15, 0, 127, 20},
  {TRIANGLE, 0, 0, 0},
  {0, 127, 0, 64},
  {0, 0, 0, 0},
  "Strings"
};

mentor_config preset2 = {
  {63, RANGE16FT, POLY8, 0},
  {86, 0, 30, 20},
  {10, 59, 73, 5},
  {TRIANGLE, 54, 3, 0},
  {0, 127, 0, 64},
  {13, 0, 0, 0},
  "Brass"
};
mentor_config preset3 = {
  {90, RANGE4FT, UNISON, 0},
  {40, 70, 63, 0},
  {8, 59, 73, 5},
  {TRIANGLE, 62, 3, 0},
  {0, 30, 0, 64},
  {13, 0, 0, 0},
  "Solo Brass"
};

mentor_config preset4 = {
  {63, RANGE8FT, POLY4, 2},
  {96, 62, 34, 31},
  {0, 29, 109, 15},
  {REV_SAWTOOTH, 86, 4, 52},
  {0, 127, 0, 64},
  {0, 0, 10, 0},
  "Wobble"
};

mentor_config preset5 = {
  {118, RANGE16FT, POLY2, 1},
  {41, 0, 63, 0},
  {1, 110, 0, 20},
  {TRIANGLE, 30, 0, 20},
  {0, 50, 76, 64},
  {10, 0, 0, 0},
  "Smooth Bass"
};

mentor_config preset6 = {
  {63, RANGE32FT, POLY2, 1},
  {112, 57, -15, 0},
  {0, 50, 0, 20},
  {TRIANGLE, 88, 5, 0},
  {0, 50, 76, 64},
  {0, 0, 10, 0},
  "Dirty Bass"
};

mentor_config preset7 = {
  {90, RANGE4FT, POLY8, 0},
  {108, 0, -24, 50},
  {33, 65, 110, 23},
  {S_AND_H, 64, 80, 89},
  {0, 109, 20, 64},
  {0, 0, 0, 0},
  "Alien"
};

mentor_config preset8 = {
  {96, RANGE8FT, POLY8, 3},
  {50, 100, 50, 0},
  {1, 33, 80, 10},
  {SQUARE, 63, 0, 70},
  {0, 127, 66, 64},
  {12, 0, 0, 0},
  "Pulse"
};

mentor_config preset9 = {
  {49, RANGE8FT, POLY8, 7},
  {100, 70, 63, 0},
  {105, 36, 92, 100},
  {SINE, 0, 0, 0},
  {0, 127, 0, 64},
  {0, 0, 15, 0},
  "Pad"
};

mentor_config preset10 = {
  {127, RANGE8FT, POLY8, 2},
  {71, 43, 4, 40},
  {0, 60, 0, 37},
  {SINE, 0, 0, 0},
  {0, 70, 0, 127},
  {0, 0, 15, 0},
  "Piano"
};

mentor_config preset11 = {
  {63, RANGE16FT, POLY2, 0},
  {95, 127, 0, 0},
  {0, 0, 127, 0},
  {TRIANGLE, 26, 3, 100},
  {0, 30, 0, 64},
  {15, 4, 0, 0},
  "Squash"
};

mentor_config randomPreset = {
  {63, RANGE8FT, POLY16, 0},
  {127, 0, 0, 0},
  {0, 0, 127, 0},
  {TRIANGLE, 0, 0, 0},
  {0, 127, 0, 64},
  {0, 0, 0, 0},
  "Random"
};

mentor_config presets[] = {
  initConfig, preset1, preset2, preset3, preset4, preset5, preset6, preset7, preset8, preset9, preset10, preset11, randomPreset
};

static waveshape shapeArray[] = {
  SINE, TRIANGLE, SQUARE, SAWTOOTH, REV_SAWTOOTH, S_AND_H
};

#define SIZE 100  // actually 88, but leaves space for expansion
#define SIGNATURE_BYTE 0x3A
#define LAST_PRESET_ADDR 1023

mentor_config makeRandomPreset()
{
  randomPreset.osc.shape = biasedRand(0, 127, UNBIASED);
  randomPreset.osc.range = (range_t)biasedRand(0, 4, BIASMIDDLE);
  randomPreset.osc.poly = (poly_t)biasedRand(0, 5, BIASMIDDLE);
  randomPreset.osc.detune = biasedRand(0, 50, BIASLOWER);
  
  randomPreset.filter.cutoff = biasedRand(0, 127, BIASHIGH);
  randomPreset.filter.resonance = biasedRand(0, 127, BIASLOW);
  randomPreset.filter.envAmt = biasedRand(-64, 63, UNBIASED);
  randomPreset.filter.hpfCutoff = biasedRand(0, 127, BIASLOW);
  
  randomPreset.env.attack = biasedRand(0, 127, BIASLOW);
  randomPreset.env.decay = biasedRand(0, 127, UNBIASED);
  randomPreset.env.sustain = biasedRand(0, 127, BIASHIGHER);
  randomPreset.env.release = biasedRand(0, 127, BIASLOW);
  
  randomPreset.lfo.shape = shapeArray[biasedRand(0, 6, UNBIASED)];
  randomPreset.lfo.rate = biasedRand(0, 127, UNBIASED);
  randomPreset.lfo.pitchAmt = biasedRand(0, 127, BIASLOWER);
  randomPreset.lfo.filterAmt = biasedRand(0, 127, UNBIASED);
  
  randomPreset.amp.gain = biasedRand(0, 6, BIASLOWER);
  randomPreset.amp.envAmt = biasedRand(0, 127, BIASHIGHER);
  randomPreset.amp.lfoAmt = biasedRand(0, 127, BIASLOW);
  randomPreset.amp.velSens = 64;

  randomPreset.eff.chorus = 0;
  randomPreset.eff.delay = 0;
  randomPreset.eff.reverb = 0;
  randomPreset.eff.overdrive = 0;
  // just one effect
  switch (biasedRand(0, 4, BIASLOW))
  {
    case 0:
      break; 
    case 1:
      randomPreset.eff.chorus = biasedRand(0, 15, UNBIASED);
      break; 
    case 2:
      randomPreset.eff.delay = biasedRand(0, 15, UNBIASED);
      break; 
    case 3:
      randomPreset.eff.reverb = biasedRand(0, 15, UNBIASED);
      break; 
    case 4:
      randomPreset.eff.overdrive = biasedRand(0, 15, BIASLOW);
      break; 
  }
  
  return randomPreset;
}

Str8 userPresetName;
const char *getPresetName(int i)
{
  if (i >= 0 && i < NUM_PRESETS - 1)
    return presets[i].name;
  else if (i == NUM_PRESETS - 1)
    return "Random";
  else if (i < NUM_PRESETS + NUM_USER)
  {
    sprintf(userPresetName, "User %d", i - NUM_PRESETS + 1);
    return userPresetName;
  }
  return "INIT";
}

mentor_config getPreset(int i)
{
  EEPROM.update(LAST_PRESET_ADDR, i);
  if (i > 0 && i < NUM_PRESETS - 1)
    return presets[i];
  else if (i == NUM_PRESETS - 1)
    return makeRandomPreset();
  else if (i < NUM_PRESETS + NUM_USER)
    return readUserPreset(i - NUM_PRESETS);
  else
    return presets[0];
}

int getSavedStartupPreset()
{
  return EEPROM.read(LAST_PRESET_ADDR);
}

void loadPreset(int i)
{
  lastIndex = i;
  *config = getPreset(i);
  synth->loadConfig(config);
}

int getLastPresetIndex()
{
  return lastIndex;
}

void updateUserPreset(byte i)
{
  int address = i * SIZE;
  EEPROM.update(address, SIGNATURE_BYTE);
  EEPROM.update(address + 1, i);
  EEPROM.put(address + 2, *config);
  lastUserIndex = i;
}

int getLastUserPresetIndex()
{
  return lastUserIndex;
}

mentor_config readUserPreset(byte i)
{
  int address = i * SIZE;
  char sig = EEPROM.read(address);
  int index = EEPROM.read(address + 1);
  if (sig == SIGNATURE_BYTE && index == i)  // appears valid
  {
    EEPROM.get(address + 2, *config);
  }
  else
  {
    *config = initConfig;
    Serial.println("User preset read failed.");
  }
  return *config;
}
