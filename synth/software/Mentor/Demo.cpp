#include <IntervalTimer.h>

#include "Config.h"
#include "Demo.h"
#include "Controls.h"
#include "MIDIManager.h"
#include "Synth.h"
#include "Utility.h"

#define NOTEDEMO 1
#define CHORDDEMO 2
#define POLYDEMO 3
#define MONODEMO 4

int curDemoPlaying = 0;

IntervalTimer timer;

typedef struct 
{
  int beat;
  byte note;
  byte velocity;
} demo_data_t, *demo_data_ptr;

#define CLOCKSPERBEAT
#define SPEED 10
#define OFF 0xFF
#define LOOP 0xFF
#define STOP 0xFE

demo_data_t *demo_data;

demo_data_t demo_data_mono[] = {
  {  0, 65, 127}, {  4, 65, OFF}, // F
  {  4, 65, 127}, {  8, 65, OFF}, // F
  {  8, 63, 127}, { 16, 63, OFF}, // Eb
  { 16, 60, 127}, { 20, 60, OFF}, // C
  { 20, 63, 127}, { 24, 63, OFF}, // Eb
  { 24, 64, 127}, { 28, 64, OFF}, // E
  { 28, 65, 127}, { 36, 65, OFF}, // F
  { 36, 77, 127}, { 38, 77, OFF}, // F
  { 38, 71, 127}, { 40, 71, OFF}, // B
  { 40, 70, 127}, { 44, 70, OFF}, // Bb
  { 44, 68, 127}, { 48, 68, OFF}, // Ab
  { 48, 65, 127}, { 50, 65, OFF}, // F
  { 52, 60, 127}, { 54, 60, OFF}, // C
  { 54, 72, 127}, { 56, 72, OFF}, // C
  { 56, 63, 127}, { 58, 63, OFF}, // Eb
  { 58, 72, 127}, { 60, 72, OFF}, // C
  { 60, 64, 127}, { 64, 64, OFF}, // E
  { 64, 65, 127}, { 68, 65, OFF}, // F
  { 68, 65, 127}, { 72, 65, OFF}, // F
  { 72, 63, 127}, { 80, 63, OFF}, // Eb
  { 80, 60, 127}, { 84, 60, OFF}, // C
  { 84, 56, 127}, { 92, 56, OFF}, // Ab
  { 92, 53, 127}, {100, 53, OFF}, // F
  {100, 67, 127}, {102, 67, OFF}, // G
  {102, 61, 127}, {104, 61, OFF}, // Db
  {104, 57, 127}, {108, 57, OFF}, // A
  {108, 67, 127}, {110, 67, OFF}, // G
  {110, 64, 127}, {112, 64, OFF}, // E
  {112, 58, 127}, {114, 58, OFF}, // Bb
  {114, 59, 127}, {116, 59, OFF}, // B
  {116, 60, 127}, {120, 60, OFF}, // C
  {120, 68, 127}, {124, 68, OFF}, // Ab
  {124, 64, 127}, {128, 64, OFF}, // E
  {128, LOOP, 127}
};

demo_data_t demo_data_poly[] = {
  {  0, 60, 127}, { 32, 60, OFF}, // C
  {  0, 63, 127}, { 32, 63, OFF}, // Eb
  {  0, 67, 127}, { 32, 67, OFF}, // G
  {  0, 36, 127}, {  4, 36, OFF}, // C
  {  4, 31, 127}, {  8, 31, OFF}, // G
  {  8, 34, 127}, { 12, 34, OFF}, // Bb
  { 12, 36, 127}, { 20, 36, OFF}, // C
  { 20, 31, 127}, { 24, 31, OFF}, // G
  { 24, 34, 127}, { 32, 34, OFF}, // Bb
  { 32, 60, 127}, { 64, 60, OFF}, // C
  { 32, 63, 127}, { 64, 63, OFF}, // Eb
  { 32, 67, 127}, { 64, 67, OFF}, // G
  { 32, 69, 127}, { 64, 69, OFF}, // A
  { 32, 36, 127}, { 36, 36, OFF}, // C
  { 36, 31, 127}, { 40, 31, OFF}, // G
  { 40, 34, 127}, { 44, 34, OFF}, // Bb
  { 44, 36, 127}, { 52, 36, OFF}, // C
  { 52, 31, 127}, { 56, 31, OFF}, // G
  { 56, 34, 127}, { 64, 34, OFF}, // Bb
  { 64, 60, 127}, { 96, 60, OFF}, // C
  { 64, 65, 127}, { 96, 65, OFF}, // F
  { 64, 67, 127}, { 96, 67, OFF}, // G
  { 64, 70, 127}, { 96, 70, OFF}, // Bb
  { 64, 38, 127}, { 68, 38, OFF}, // D
  { 68, 31, 127}, { 72, 31, OFF}, // G
  { 72, 36, 127}, { 76, 36, OFF}, // C
  { 76, 38, 127}, { 84, 38, OFF}, // D
  { 84, 39, 127}, { 88, 39, OFF}, // Eb
  { 88, 36, 127}, { 96, 36, OFF}, // D
  { 96, 60, 127}, {119, 60, OFF}, // C
  { 96, 62, 127}, {119, 62, OFF}, // D
  { 96, 65, 127}, {119, 65, OFF}, // F
  { 96, 69, 127}, {119, 69, OFF}, // A
  { 96, 38, 127}, {100, 38, OFF}, // D
  {100, 31, 127}, {104, 31, OFF}, // G
  {104, 36, 127}, {108, 36, OFF}, // C
  {108, 38, 127}, {116, 38, OFF}, // D
  {116, 39, 127}, {120, 39, OFF}, // Eb
  {120, 60, 127}, {128, 60, OFF}, // C
  {120, 63, 127}, {128, 63, OFF}, // Eb
  {120, 65, 127}, {128, 65, OFF}, // F
  {120, 68, 127}, {128, 68, OFF}, // Ab
  {120, 32, 127}, {128, 32, OFF}, // Ab
  {128, LOOP, 127}
};

demo_data_t demo_data_note[] = {
  {  0, 65, 127}, { 99990, 65, OFF}, // F
  {99990, STOP, 127}
};

demo_data_t demo_data_chord[] = {
  {  0, 39, 127}, { 64, 39, OFF}, // Eb
  {  0, 70, 127}, { 64, 70, OFF}, // Bb
  {  0, 63, 127}, { 64, 63, OFF}, // Eb
  {  0, 67, 127}, { 64, 67, OFF}, // G
  {68, LOOP, 127}
};

static volatile int beat = 0;
static volatile int clk = 0;
int lastclk = -1;
int lastbeat = -1;
int demoindex = 0;
static volatile demo_data_ptr nextnote = NULL;

int sort_demo(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  demo_data_t a = *((demo_data_t *)cmp1);
  demo_data_t b = *((demo_data_t *)cmp2);
  if (a.note == LOOP || a.beat == STOP)
    return 1; // must be at the end
  else if (b.note == LOOP || b.beat == STOP)
    return -1; // must be at the end
  return a.beat - b.beat;
}

#ifdef DEBUG_STR
void debugSortedNotes(demo_data_t notes[], size_t N)
{  
  for (int i = 0; i < N; ++i)
  {
    demo_data_t n = notes[i];
    if (n.beat == STOP)
      Serial.println("STOP");
    else if (n.note == LOOP)
      Serial.println("LOOP");
    else
    {
      Serial.print(n.beat);
      Serial.print(" ");
      Serial.print(notestring(n.note));
      Serial.println(n.velocity == OFF ? " off" : " on");
    }
  } 
}
#endif

void initDemo()
{
  qsort(demo_data_poly, sizeof(demo_data_poly) / sizeof(demo_data_poly[0]), sizeof(demo_data_poly[0]), sort_demo);
//  debugSortedNotes(demo_data_poly, sizeof(demo_data_poly) / sizeof(demo_data_poly[0]));
  qsort(demo_data_mono, sizeof(demo_data_mono) / sizeof(demo_data_mono[0]), sizeof(demo_data_mono[0]), sort_demo);
//  debugSortedNotes(demo_data_mono, sizeof(demo_data_mono) / sizeof(demo_data_mono[0]));
  qsort(demo_data_chord, sizeof(demo_data_chord) / sizeof(demo_data_chord[0]), sizeof(demo_data_chord[0]), sort_demo);
}

void callback()
{
  if (nextnote)
  {
    clk++;
    if (clk % 6 == 0)
      beat++;
  }
}

bool demoIsPlaying()
{
  return curDemoPlaying > 0;
}

void setupDemoStart()
{
  beat = 0;
  demoindex = 0;
  timer.begin(callback, SPEED * 1000);
  nextnote = &demo_data[0];
}

void handleDemoStop()
{
  timer.end();
  beat = 0;
  clk = 0;
  lastclk = -1;
  lastbeat = -1;
  demoindex = 0;
  nextnote = NULL;
  resetMIDI();
}

void startDemo(int demo)
{
  if (demoIsPlaying())
    return;
  switch (demo)
  {
    case NOTEDEMO:
      demo_data = demo_data_note;
      synth->allNotesOff();
      setFrequencyMode(true, demo_data[0].note);
      break;
    case CHORDDEMO:
      demo_data = demo_data_chord;
      break;
    default:
    case POLYDEMO:
      demo_data = demo_data_poly;
      break;
    case MONODEMO:
      demo_data = demo_data_mono;
      break;
  }
  setupDemoStart();
  curDemoPlaying = demo;
}

void stopDemo()
{
  if (!demoIsPlaying())
    return;
  curDemoPlaying = 0;
  handleDemoStop();
  setFrequencyMode(false, 0);
}

const char *getDemoDescription(int i)
{
  if (demoIsPlaying())
    return "Stop";
  else
  {
    switch (i)
    {
      case POLYDEMO:
        return "Polyphonic";
      case MONODEMO:
        return "Monophonic";
      case NOTEDEMO:
        return "Single note";
      case CHORDDEMO:
        return "Repeat chord";
    }
  }
  return "Unknown";
}

void checkDemo()
{
  if (clk == lastclk)
    return;
  clockTick();
  lastclk = clk;
  if (beat == lastbeat)
    return;
  lastbeat = beat;
  if (demoIsPlaying())
  {
    if (!nextnote || nextnote->beat == STOP)
    {
      stopDemo();
    }
    else 
    {
      while (nextnote->beat <= beat)
      {
        if (nextnote->note == LOOP)
        {
          beat = 0;
          demoindex = 0;
          nextnote = &demo_data[0];
        }
        if (nextnote->velocity == OFF)
          releaseNote(nextnote->note);
        else
          playNote(nextnote->note, nextnote->velocity);
        demoindex++;
        nextnote = &demo_data[demoindex];
      }
    }
  }
}
