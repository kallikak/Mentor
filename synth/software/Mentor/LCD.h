#pragma once

void setupLCD();

void clearLCD();

void clearRow(int r);

void writeRow(int r, const char *str);

void writeParamString(int r, const char *param, const char *str);

void writeParamValue(int r, const char *param, ccInt cc);
