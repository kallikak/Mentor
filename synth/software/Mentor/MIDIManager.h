#pragma once

#define CC_MODWHEEL   1
#define CC_SUSTAIN_PEDAL 64

#define CC_SHAPE      20
#define CC_RANGE      21
#define CC_POLY       24
#define CC_DETUNE     94

#define CC_CUTOFF     74
#define CC_RESONANCE  71
#define CC_ENVAMT     22
#define CC_HPF        81

#define CC_ATTACK     73
#define CC_DECAY      80
#define CC_SUSTAIN    27
#define CC_RELEASE    72

#define CC_LFO_SHAPE  12
#define CC_LFO_RATE   3
#define CC_LFO_PITCH  13
#define CC_LFO_FILTER 23
#define CC_LFO_SYNC   30

#define CC_GAIN       25
#define CC_AMP_ENV    26
#define CC_AMP_LFO    28
#define CC_VELSENS    29

#define CC_CHORUS     93
#define CC_DELAY      89
#define CC_REVERB     91
#define CC_OVERDRIVE  90

typedef struct notedata_s
{
  byte pitch;
  byte velocity;
  int voice;
  bool sustain;
} notedata;

void setupMIDI();

void checkMIDI();

void resetMIDI();

void stopAllMIDI();

bool getAftertouch();

void setAftertouch(bool set);

void playNote(byte pitch, byte velocity);

void releaseNote(byte pitch);

void clockTick();
