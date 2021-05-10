#include <Wire.h>
#include <ctype.h>

#include "Config.h"
#include "Control.h"
#include "Controls.h"
#include "Demo.h"
#include "LCD.h"
#include "MIDIManager.h"
#include "Presets.h"
#include "Synth.h"
#include "Voice.h"

#define LFO_LED 6

Synth *synth;

void setup() 
{
  Serial.begin(9600);
  delay(1000);
  srand((analogRead(A0) << 10) + analogRead(A1));  // unused pins
  
  setupLCD();
  setupControls();
  setupControl();
  initDemo();
  
  pinMode(LFO_LED, OUTPUT);
  digitalWrite(LFO_LED, LOW);

  synth = new Synth();
  loadPreset(getSavedStartupPreset());
  delay(500);
  writeParamString(1, "Loading", getPresetName(getSavedStartupPreset()));
  delay(1000);
  synth->loadConfig(config);
  
  setupMIDI();
  printConfig(config);
  
  Serial.println("Setup done");
}

int loopcount = 0;
bool monitorlevel = false;
void monitorPeak();
void loop() 
{
  static bool led = false;
  checkControls(loopcount++);
  checkControl();
  
  if (monitorlevel && loopcount % 200 == 0)
  {
    monitorPeak();
  }
  
  checkMIDI();
  if (synth->checkFlashLFO())
  {
    led = !led;
    digitalWrite(LFO_LED, led);
  }
  checkDemo();
  if (Serial.available())
  {
    char c = Serial.read();
    if (iscntrl(c))
    {
      // ignore control data 
    }
    else if (c == 'p')
    {
      printConfig(config);
    }
    else if (c == '1')
    {
      Serial.println("Playing 1 note");
      playNote(55, 127);
    }
    else if (c == '2')
    {
      Serial.println("Playing 2 notes");
      playNote(55, 127);
      playNote(62, 127);
    }
    else if (c == '5')
    {
      Serial.println("Playing 5 notes");
      playNote(55, 127);
      playNote(59, 127);
      playNote(62, 127);
      playNote(67, 127);
      playNote(69, 127);
    }
    else if (c == '0')
    {
      Serial.println("Lifting notes");
      releaseNote(55);
      releaseNote(59);
      releaseNote(62);
      releaseNote(67);
      releaseNote(69);
    }
    else if (c == 'm')
    {
      Serial.print("Max memory usage: ");
      Serial.println(AudioMemoryUsageMax());
    }
    else if (c == 't')
    {
      Serial.print("MCU Temperature: ");
      Serial.println(tempmonGetTemp());
    }
    else if (c == 'v')
    {
      monitorlevel = !monitorlevel;
      Serial.print("Output level monitor ");
      Serial.println(monitorlevel ? "on" : "off");
    }
    else if (c == 'h')
    {
      Serial.println("========");
      Serial.println("Commands");
      Serial.println("========");
      Serial.println("p:\tPrint current config");
      Serial.println("m:\tPrint max audio memory usage");
      Serial.println("v:\tToggle output volume level monitor");
      Serial.println("t:\tPrint MCU temperature");
      Serial.println("h:\tPrint this message");
    }
    else if (c != '\r' && c != '\n' && !isspace(c))
    {
      Serial.println("Available commands: pmvth");
      Serial.println("Type h for help");
    }
    while (Serial.available())
      Serial.read();
  }
}

/*
 * BUGS
 *  - Something strange happens when amp env amount is 0
 * TODO
 *  - Refine levels
 */
