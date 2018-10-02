/*
-----lookup_table.h-----
Header for the sqrt lookup table functions.
Used to generate a table of sqrts that can quickly
be used instead of the slow sqrt() function.


*/
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

//Generates a list of sqrts
//ignore the bad variable name...
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

//Finds the square root of x, using interpolation to approximate it. 
//If it is beyond the lookup table size, it uses the slow square root
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
//Deletes the lookup table, freeing up memory.
void freeLookupTable()
{
    free(lut);
    lookupTableSize = 0;
}