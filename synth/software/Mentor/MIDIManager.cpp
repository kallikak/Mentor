#include <Audio.h>
#include <MIDI.h>

#include "Config.h"
#include "Controls.h"
#include "LCD.h"
#include "MIDIManager.h"
#include "Presets.h"
#include "Synth.h"
#include "Utility.h"

MIDI_CREATE_DEFAULT_INSTANCE();

#define BASE_NOTE 48

bool sustainState = false;
notedata NO_NOTE = {0, 0, -1};

notedata note[POLYPHONY] = { NO_NOTE };
bool voiceuse[POLYPHONY] = { false };

#define MIDI_MIN 21
#define MIDI_MAX 108

bool sustained[MIDI_MAX + 1] = {false};

#define VALIDATE_MIDI(x) if (x < MIDI_MIN || x > MIDI_MAX) return

void debugNotes()
{
  Serial.println("Current notes");
  Serial.println("=============");
  for (int i = 0; i < POLYPHONY; ++i)
  {
    if (note[i].pitch > 0)
    {
      Serial.print("\t[V");
      Serial.print(note[i].voice + 1);
      Serial.print("] Pitch: ");
      Serial.print(notestring(note[i].pitch - 20));
      Serial.print(", velocity: ");
      Serial.print(note[i].velocity);
      Serial.print(", voice: ");
      Serial.print(note[i].voice);
      if (sustainState)
      {
        Serial.print(sustained[note[i].pitch] ? " sustained" : " held"); 
      }
      Serial.println();
    }
  }
}

int getfreevoice()
{
  int v = 0;
  // look for fully free voice
  while (v < POLYPHONY && synth->isVoiceActive(v))
    ++v;
  if (v == POLYPHONY)
  {
    v = 0;
    // now try for one that is in its release stage
    while (v < POLYPHONY && voiceuse[v])
      ++v;
  }
  return v == POLYPHONY ? -1 : v;
}

void shiftnotes(int k)
{
  int i = k;
  while (i < POLYPHONY - 1 && note[i].pitch > 0)
  {
    note[i] = note[i + 1];
    ++i;
  }
  note[i] = NO_NOTE;
}

notedata releaselastvoice(bool checkSustain)
{
  DBG(checkSustain);
  debugNotes();
  int i = 0;
  notedata n = note[i];
  while (checkSustain && sustainState && !sustained[n.pitch]) 
  {
    if (++i == POLYPHONY)
      return releaselastvoice(false);
    n = note[i];
  };
  shiftnotes(i);
  return n;
}

notedata releasehighnotevoice(byte protectpitch)
{
  byte maxpitch = 0;
  int maxindex = -1;
  for (int i = 0; i < POLYPHONY; ++i)
  {
    if (note[i].pitch == 0 || note[i].pitch == protectpitch)
      continue;
    if (note[i].pitch > maxpitch)
    {
      maxpitch = note[i].pitch;
      maxindex = i;
    }
  }
  if (maxindex < 0)
    return NO_NOTE;
  notedata n = note[maxindex];
  shiftnotes(maxindex);
  return n;
}

notedata releaselownotevoice(byte protectpitch)
{
  byte minpitch = 0xff;
  int minindex = -1;
  for (int i = 0; i < POLYPHONY; ++i)
  {
    if (note[i].pitch == 0 || note[i].pitch == protectpitch)
      continue;
    if (note[i].pitch < minpitch)
    {
      minpitch = note[i].pitch;
      minindex = i;
    }
  }
  if (minindex < 0)
    return NO_NOTE;
  notedata n = note[minindex];
  shiftnotes(minindex);
  return n;
}

notedata addNote(byte pitch, byte velocity)
{
  int v = getfreevoice();
  if (v < 0)
  {
    // if sustainState, don't release a note that is currently held unless no alternative
    if (config->osc.poly >= POLY4)
      v = releaselastvoice(true).voice;
    else
      v = releasehighnotevoice(pitch).voice;
  }
  for (int i = 0; i < POLYPHONY; ++i)
  {
    if (note[i].pitch == 0)
    {
      note[i] = { pitch, velocity, v, false };
      voiceuse[v] = true;
      return note[i];
    }
  }
  Serial.println("Add note error!");
  return NO_NOTE; // should be impossible...
}

notedata removeNote(byte pitch)
{
  for (int i = 0; i < POLYPHONY; ++i)
  {
    if (note[i].pitch == pitch)
    {
      notedata n = note[i];
      shiftnotes(i);
      voiceuse[n.voice] = false;
      return n;
    }
  }
  return NO_NOTE; // this will happen if we have overflowed polyphony earlier
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  VALIDATE_MIDI(pitch);
    
#if DEBUG_SERIAL
  char tempstr[50] = {0};
  sprintf(tempstr, "On: %d = %s on [%d]. ", pitch, notestring((int)round(pitch - 20)), velocity);
  Serial.print(tempstr);
#endif  
  // handle unison
  int unison = getUnison(config->osc.poly);
//  AudioNoInterrupts();
  for (int i = 1; i <= unison; ++i)
  {
    notedata n = addNote(pitch, velocity);
    sustained[n.pitch] = false;
    // play it on the synth
    if (n.voice >= 0)
    {
      synth->playNote(n, i, unison);
    }
  }
//  AudioInterrupts();
#if DEBUG_SERIAL
  debugNotes();
#endif  
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  VALIDATE_MIDI(pitch);
#if DEBUG_SERIAL
  char str[17];
  sprintf(str, "Off: %d", pitch);
  Serial.println(str);
#endif  
  if (sustainState)
  {
    sustained[pitch] = true;
  }
  else
  {
    for (int i = 0; i < POLYPHONY; ++i)
    {
      notedata n = removeNote(pitch);
      // stop it on the synth
      if (n.voice < 0)
        break;
      synth->liftNote(n);
    }
  }
#if DEBUG_SERIAL
  debugNotes();
#endif  
}

void clearSustainedNotes()
{
  for (int pitch = MIDI_MIN; pitch <= MIDI_MAX; ++pitch)
  {
    if (sustained[pitch])
    {
      sustained[pitch] = false;
      handleNoteOff(0, pitch, 0);
    }
  }
}

void resetMIDI()
{
  stopAllMIDI();
}

#define CC_VOLUME     9
#define CC_ALLOFF     0x7B

void handleControlChange(byte channel, byte control, byte value)
{
  if (control == CC_MODWHEEL)
    synth->modWheel(value);
  else if (control == CC_ALLOFF)
    resetMIDI();
  else if (control == CC_SUSTAIN_PEDAL)
  {
    sustainState = value == 127;
#if DEBUG_SERIAL    
    Serial.print(value);
    Serial.print(" => sustain is ");
    Serial.println(sustainState ? "On" : "Off");
#endif    
    clearSustainedNotes();
    if (config->env.sustain == 0)
    {
      synth->setForceHold(sustainState);
    }
  }
  else
  {
    handleParameterUpdate(control, value);
  }
}

#define BENDRANGE 12

void handlePitchBend(byte channel, int value)
{
  int bendCents = (int)round(-BENDRANGE * 100.0 * value / 8192.0);
  synth->bend(bendCents);
}

void handleAfterTouch(byte channel, byte pressure)
{
  synth->setLFOFilter(constrainCC(config->lfo.filterAmt + pressure));
}

#define MIDI_CLOCKS_PER_BEAT 24

int clockCount = 0;
int samples = 0;
unsigned long lastbeattime = 0;
float bpm = 200;
int lastupdatebpm = 0;

#define BUFFERSIZE 21
#define MIDBUFFER (BUFFERSIZE >> 1)
float clockbuffer[BUFFERSIZE] = { 0 };
float sortbuffer[BUFFERSIZE];
int clockbufferindex = 0;

int sort_desc(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  float a = *((float *)cmp1);
  float b = *((float *)cmp2);
  return b - a;
}

float median() 
{
  memcpy(sortbuffer, clockbuffer, BUFFERSIZE * sizeof(float));
  qsort(sortbuffer, BUFFERSIZE, sizeof(sortbuffer[0]), sort_desc);
  return sortbuffer[MIDBUFFER];
}

void handleClock()
{
  if (synth->getLFOSync() == LFOSYNC_OFF)
    return;
#if DEBUG_SERIAL    
  Serial.println("MIDI clock");
#endif     
  clockCount++;
  samples++;
  unsigned long thisbeattime = micros();
  if (lastbeattime > 0 && thisbeattime > lastbeattime)
  {
    float newbpm = 60 * 1000 * 1000.0 / MIDI_CLOCKS_PER_BEAT / (thisbeattime - lastbeattime);
#if DEBUG_SERIAL    
    Serial.print("newbpm is ");
    Serial.println(newbpm);
#endif
    clockbuffer[clockbufferindex] = newbpm;
    clockbufferindex = (clockbufferindex + 1) % BUFFERSIZE;
    if (samples >= BUFFERSIZE)
    {
      bpm = median();
#if DEBUG_SERIAL    
      Serial.print("Millis since last beat: ");
      Serial.print((thisbeattime - lastbeattime) / 1000.0);
      Serial.print(" => bpm of ");
      Serial.println(bpm);
#endif     
    }
    if (clockCount == MIDI_CLOCKS_PER_BEAT) // should be 24...
    {
      clockCount = 0;
      long rbpm = round(bpm);
      if (rbpm > 0 && abs(lastupdatebpm - rbpm) >= 1)
      {
#if DEBUG_SERIAL    
        Serial.print("bpm is ");
        Serial.print(bpm);
        Serial.print(" = ");
        Serial.print(bpm / 60.0);
        Serial.println("Hz");
#endif        
        lastupdatebpm = bpm;
        synth->setLFOFrequency(bpm / 60.0, true);
      }
    }
  }
  lastbeattime = thisbeattime;
}

void playNote(byte pitch, byte velocity)
{
  handleNoteOn(1, pitch, velocity);
}

void releaseNote(byte pitch)
{
  handleNoteOff(1, pitch, 0); 
}

void clockTick()
{
  handleClock();
}

void handleProgramChange(byte channel, byte number)
{
  loadPreset(number);  
}

void setupMIDI()
{
  MIDI.begin(MIDI_CHANNEL_OMNI);   
  MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.setHandlePitchBend(handlePitchBend);
  MIDI.setHandleClock(handleClock);
  MIDI.setHandleAfterTouchChannel(handleAfterTouch);
  MIDI.setHandleProgramChange(handleProgramChange);
  
  usbMIDI.setHandleNoteOn(handleNoteOn);
  usbMIDI.setHandleNoteOff(handleNoteOff);
  usbMIDI.setHandleControlChange(handleControlChange);
  usbMIDI.setHandlePitchChange(handlePitchBend);
  usbMIDI.setHandleClock(handleClock);
  usbMIDI.setHandleAfterTouch(handleAfterTouch);
  usbMIDI.setHandleProgramChange(handleProgramChange);
}

void stopAllMIDI()
{
  // make sure everything is cleaned up
  for (int i = 0; i < POLYPHONY; ++i)
  {
    if (note[i].pitch > 0)
      releaseNote(note[i].pitch);
  }
  // clear everything
  for (int pitch = MIDI_MIN; pitch <= MIDI_MAX; ++pitch)
  {
    sustained[pitch] = false;
    handleNoteOff(0, pitch, 0);
    // just to be extra sure...
    synth->allNotesOff();
  }
}

void checkMIDI()
{
  MIDI.read();
  usbMIDI.read();
}
