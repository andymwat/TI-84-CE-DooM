/*
-----helpers.h-----
Header with declarations of some helper functions.
*/
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
#endif