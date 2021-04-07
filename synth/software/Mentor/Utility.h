#pragma once

float noteToFreq(float n);

float freqToNote(float f);

ccInt constrainDoubleCC(ccInt lvl);

ccInt constrainCC(ccInt lvl);

ccInt constrainSignedCC(ccInt lvl);

int constrainInt(int u, int low, int high);

#define BIASHIGH   0.5
#define BIASHIGHER 0.3
#define UNBIASED   1
#define BIASLOW    2
#define BIASLOWER  3
#define BIASMIDDLE -1

int biasedRand(int min, int max, float bias);

const char *notestring(float n);

float calcDetune(float f, float c);

int16_t movingaverage(int16_t newvalue, int16_t avg, int n);

float movingaveragefloat(float newvalue, float avg, int n);
