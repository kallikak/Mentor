#pragma once

#define NUM_PRESETS 13
#define NUM_USER 10

mentor_config getPreset(int i);

int getSavedStartupPreset();

const char *getPresetName(int i);

int getLastPresetIndex();

void loadPreset(int i);

void updateUserPreset(byte i);

int getLastUserPresetIndex();

mentor_config readUserPreset(byte i);
