#pragma once

void setupControls();
void checkControls(int loopcount);
void handleParameterUpdate(ccInt param, int value);
void setFrequencyMode(bool set, float f);

extern int lastupdatemillis;
extern int selected;
