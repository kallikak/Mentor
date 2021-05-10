#include <Bounce2.h>

#include "Config.h"
#include "Control.h"
#include "Controls.h"
#include "Demo.h"
#include "LCD.h"
#include "MIDIManager.h"
#include "Rotary.h"
#include "Presets.h"
#include "Synth.h"

#define CONTROL_A  3
#define CONTROL_B  4
#define CONTROL_SW 5
#define LONG_MILLIS 1000

typedef enum
{
  CTRL_PRESET,  // for loading ROM or user presets
  CTRL_USER,    // for saving user presets
  CTRL_MIDICLK, // toggling external clock
  CTRL_DEMO,    // play or stop demo
  CTRL_PANIC,   // all MIDI notes off
  CTRL_AFTERTOUCH
} control_state;
#define NUM_STATES 6
#define CTRL_LAST CTRL_AFTERTOUCH

void checkSwitch();
void controlClick();
void controlLongPress();
void controlTurned(int d, bool btnState);
void selectValue();
void selectState();

static control_state ctrl_state = CTRL_LAST;
static int ctrl_val = 0;
static bool state_selected = false;

static bool halfstate = false;
static uint8_t _state;

static Bounce debouncer = Bounce(); 
static long falltimeP = -1;
static bool longpressP = false;

int getLastStateValue()
{
  switch (ctrl_state)
  {
    case CTRL_PRESET:
      return getLastPresetIndex() + 1;
    case CTRL_MIDICLK:
      return synth->getLFOSync() + 1;
    case CTRL_USER:
      return getLastUserPresetIndex();
    case CTRL_DEMO:
      return demoIsPlaying() ? 1 : 0;
    case CTRL_AFTERTOUCH:
      return getAftertouch() ? 1 : 0;
    default:
      return 0;
  }
}

const char *getStateString(control_state state)
{
  switch (state)
  {
    case CTRL_PRESET:
      return "Presets";
    case CTRL_USER:
      return "Save";
    case CTRL_MIDICLK:
      return "LFO Sync";
    case CTRL_DEMO:
      return "Demo";
    case CTRL_PANIC:
      return "Panic";
    case CTRL_AFTERTOUCH:
      return "Aftertouch";
    default:
      return "";
  }
}

Str16 tempStr;
const char *getValueString(control_state state, int ctrl_val)
{
  if (ctrl_val == 0)
    return "Cancel";
  switch (state)
  {
    case CTRL_PRESET:
    {
      if (ctrl_val <= NUM_PRESETS)
        return getPresetName(ctrl_val - 1);
      else
      {
        sprintf(tempStr, "User %d", ctrl_val - NUM_PRESETS);
        return tempStr;
      }
    }
    case CTRL_USER:
    {
      sprintf(tempStr, "User %d", ctrl_val);
      return tempStr;
    }
    case CTRL_MIDICLK:
      return getLFOSyncFactorString((LFOSync_t)((ctrl_val - 1) % NUM_CLOCK_FACTORS));
    case CTRL_DEMO:
      return getDemoDescription(ctrl_val);
    case CTRL_PANIC:
      return "Send notes off";
    case CTRL_AFTERTOUCH:
      return getAftertouch() ? "Turn off" : "Turn on";
  }
  return "<N/A>";
}

void setupControl()
{
  pinMode(CONTROL_A, INPUT_PULLUP);
  pinMode(CONTROL_B, INPUT_PULLUP);
  pinMode(CONTROL_SW, INPUT_PULLUP);
  debouncer.attach(CONTROL_SW);
  debouncer.interval(50);
}

void checkControl()
{
  checkSwitch();
  bool buttonState = debouncer.read();
  int p0 = digitalRead(CONTROL_A);
  int p1 = digitalRead(CONTROL_B);
  uint8_t pinstate = (p0 << 1) | p1;
  if (halfstate)
    _state = ttable_half[_state & 0xf][pinstate];
  else
    _state = ttable[_state & 0xf][pinstate]; 
  if ((_state & 0x30) == DIR_CW)
    controlTurned(1, buttonState);
  else if ((_state & 0x30) == DIR_CCW)
    controlTurned(-1, buttonState);
}

void checkSwitch()
{
  debouncer.update();
  if (debouncer.fell())
  {
    longpressP = false;
    falltimeP = millis();
  }
  else if (falltimeP > 0 && debouncer.rose())
  {
    if (!longpressP) 
      controlClick();
    longpressP = false;
    falltimeP = -1;
  }
  else if (falltimeP > 0 && (millis() - falltimeP > LONG_MILLIS))
  {
    if (!longpressP)
    {
      controlLongPress();
      longpressP = true;
      falltimeP = -1;
    }
  }
}

void controlClick()
{
  lastupdatemillis = millis();
  if (state_selected)
    selectValue();
  else
    selectState();
  state_selected = !state_selected;
}

void controlLongPress()
{
}

void controlTurned(int d, bool btnState)
{
  lastupdatemillis = millis();
  if (state_selected)
  {
    ctrl_val += d;
    switch (ctrl_state)
    {
      case CTRL_PRESET:
        ctrl_val = max(0, min(NUM_USER + NUM_PRESETS, ctrl_val));
        break;
      case CTRL_USER:
        ctrl_val = max(0, min(NUM_USER, ctrl_val));
        break;
      case CTRL_MIDICLK:
        ctrl_val = max(0, min(NUM_CLOCK_FACTORS, ctrl_val));
        break;
      case CTRL_DEMO:
        ctrl_val = max(0, min(demoIsPlaying() ? 2 : 4, ctrl_val));
        break;
      case CTRL_PANIC:
      case CTRL_AFTERTOUCH:
        break;  // nothing to do
    }
    writeParamString(0, " ", getStateString(ctrl_state));
    writeParamString(1, ">", getValueString(ctrl_state, ctrl_val));
  }
  else
  {
    ctrl_state = (control_state)((ctrl_state + d + NUM_STATES) % NUM_STATES); 
    ctrl_val = getLastStateValue();
    writeParamString(0, ">", getStateString(ctrl_state));
    writeParamString(1, " ", getValueString(ctrl_state, ctrl_val));
  }
}

void selectState()
{
  ctrl_val = getLastStateValue();
  writeParamString(0, " ", getStateString(ctrl_state));
  writeParamString(1, ">", getValueString(ctrl_state, ctrl_val));
}

void selectValue()
{
  writeParamString(0, ">", getStateString(ctrl_state));
  writeParamString(1, " ", getValueString(ctrl_state, ctrl_val));
  if (ctrl_val == 0)
    return;  // Cancel
  switch (ctrl_state)
  {
    case CTRL_PRESET:
      loadPreset(ctrl_val - 1);
      break;
    case CTRL_USER:
      if (ctrl_val)
        updateUserPreset((byte)ctrl_val - 1);
      break;
    case CTRL_MIDICLK:
      synth->setLFOSync((LFOSync_t)(ctrl_val - 1));
      break;
    case CTRL_DEMO:
      if (demoIsPlaying())
        stopDemo();
      else
        startDemo(ctrl_val);
      break;
    case CTRL_PANIC:
      resetMIDI();
      break;
    case CTRL_AFTERTOUCH:
      setAftertouch(!getAftertouch());
      break;
  }
  writeParamString(1, " ", getValueString(ctrl_state, ctrl_val));
}
