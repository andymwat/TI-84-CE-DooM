#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphx.h>
#include <debug.h>
#include <keypadc.h>
#include <assert.h>
#include <compression.h>
float * lut;
static int lookupTableSize = 0;
static int skip = 0;
void generateLookupTable(int size, int ski)
{
   
    int i;
     lookupTableSize = size;
     skip = ski;
    lut = malloc(sizeof(float) * size);
    for (i = 0; i< size; i++)
    {
        lut[i] = sqrt((float)(i * skip));
    }

}

float lookupSqrt(float x)
{
    int lower;
    if (x >= lookupTableSize-1)
    {
        return sqrt(x);
    }
    lower = (int)x;
    return (lut[lower* skip] + lut[lower*skip +1])/2;

    
}
void freeLookupTable()
{
    free(lut);
    lookupTableSize = 0;
}