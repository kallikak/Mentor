#include <stdlib.h>
#include <math.h>

#include "Config.h"
#include "Utility.h"

float noteToFreq(float n)
{
  // MIDI note numbers 
  //  f = 440 * 2 ^ ((n - 69) / 12)
  float f = pow(2, (n - 69) / 12.0) * 440;
  return f;
}

float freqToNote(float f)
{
  return 12 * log2(f / 440.0) + 49;
}

ccInt constrainDoubleCC(ccInt lvl)
{
  return max(0, min(255, lvl));
}

ccInt constrainCC(ccInt lvl)
{
  return max(0, min(127, lvl));
}

ccInt constrainSignedCC(ccInt lvl)
{
  return max(-64, min(63, lvl));
}

int constrainInt(int u, int low, int high)
{
  return max(low, min(high, u));
}

int biasedRand(int min, int max, float bias)
{
  float r = 1.0 * rand() / RAND_MAX;  // 0 - 1
  float v;
  if (bias == BIASMIDDLE)
  {
    float r2 = 1.0 * rand() / RAND_MAX;
    v = (r + r2) / 2;
  }
  else
    v = pow(r, bias);
  return min + (max + 1 - min) * v;
}

/*
function biasedRand(min, max, bias)
{
  var v;
  if (bias < 0)
    v = (Math.random() + Math.random()) / 2;
  else
    v = Math.pow(Math.random(), bias);
  return Math.floor(min + (max + 1 - min) * v);
} 
new Array(60).fill().map(() => biasedRand(1, 10, 1)).join(" ");
c.fill(0);for (i = 0; i < 1000; ++i) c[biasedRand(1, 10, 1) - 1]++;c.join(" ");
 */

int roundnotenumber(float n)
{
  if (n > 80)
    return floor(n + 0.3); // allow 70 cents sharp to still identify as the lower note
  else if (n > 60)
    return floor(n + 0.4); // here allow 60
  else if (n < 15)
    return floor(n + 0.6); // low base allow 60 cents flat
  else if (n < 3)
    return floor(n + 0.7); // lowest base allow 70 cents flat
  else
    return floor(n + 0.5); // otherwise just round off
};

static char tempnotestr[10] = {0};
const char *notestring(float n)
{
  static const char* notes[] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};

  int k = roundnotenumber(n);
  int i = (k + 11) % 12;
  if (i < 0)
    return "-";
  sprintf(tempnotestr, "%s%d", notes[(k + 11) % 12], (int)floor((k + 8) / 12));
  return tempnotestr;
}

float calcDetune(float f, float c)
{
  if (c == 0)
    return f;
  return f * pow(2, -c / 1200);
}

int16_t movingaverage(int16_t newvalue, int16_t avg, int n)
{
  return (n * avg + newvalue) / (n + 1);
//  // (3 * avg + 1 * newdat) / 4
//  return (((avg << 2) - avg + newdat) + 2) >> 2;
}

float movingaveragefloat(float newvalue, float avg, int n)
{
  return (n * avg + newvalue) / (n + 1);
}
