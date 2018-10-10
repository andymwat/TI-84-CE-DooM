/*
-----helpers.h-----
Header with declarations of some helper functions.
*/
#include <tice.h>
#ifndef HELPERS_H
#define HELPERS_H


float min(float, float);
float max(float, float);
float floatAbs(float );
float fastACos(float);
float fastCos(float);
float fastSin(float);
float fastSqrt(float);
int root(int);

float distanceBetween(float *, float *);

void normalizeVector(float *);
void rotateVector(float *, float);

void printTextSmall(const char *, uint8_t , uint8_t);
void printText(const char *, uint8_t, uint8_t );
#endif