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
#include "helpers.h"
int root(int x)
{
	
	 int a,b;
	b = x;
	a = x = 0x3f;
	x = b/x;
	a = x = (x+a)>>1;
	x = b/x;
	a = x = (x+a)>>1;
	x = b/x;
	x = (x+a)>>1;
	return(x); 
	
}
void rotateVector(float * vector, float angle)
{
	float oldX,oldY;
	//oldX =(vector[0] *cos(angle)) - (vector[1] * sin(angle));
	//oldY = (vector[0] * sin(angle)) + (vector[1] *cos(angle));
	oldX =(vector[0] *fastCos(angle)) - (vector[1] * fastSin(angle));
	oldY = (vector[0] * fastSin(angle)) + (vector[1] *fastCos(angle));
	vector[0] = oldX;
	vector[1] = oldY;
}
void normalizeVector(float * vector)
{
	float dist = sqrt((vector[0] * vector[0]) + (vector[1]	* vector[1]));	
	vector[0] = vector[0] / dist;
	vector[1] = vector[1] /dist;
}

float distanceBetween(float * vec1, float * vec2)
 {
	//return  lookupSqrt( (vec1[0] - vec2[0]) * (vec1[0] - vec2[0]) + (vec1[1] - vec2[1]) * (vec1[1] - vec2[1]) );
	return  sqrt( (vec1[0] - vec2[0]) * (vec1[0] - vec2[0]) + (vec1[1] - vec2[1]) * (vec1[1] - vec2[1]) );
	
 }
 float fastCos(float x)
 {
	 float cos = 0;
	 while ( x < -3.14159265 )
	 {
		  x += 6.28318531;
	 }
	 while (x > 3.14159265)
	 {
		  x -= 6.28318531;
	 }
	 
	 x += 1.57079632;
	if (x >  3.14159265)
	{
		x -= 6.28318531;
	}
	if (x < 0)
	{
    cos = 1.27323954 * x + 0.405284735 * x * x;
    
    if (cos < 0)
        cos = .225 * (cos *-cos - cos) + cos;
    else
        cos = .225 * (cos * cos - cos) + cos;
	}
	else
	{
    cos = 1.27323954 * x - 0.405284735 * x * x;

    if (cos < 0)
        cos = .225 * (cos *-cos - cos) + cos;
    else
        cos = .225 * (cos * cos - cos) + cos;
	}
	return cos;
 }
 float fastSin(float x)
 {
	 float sin = 0;
	 while ( x < -3.14159265 )
	 {
		  x += 6.28318531;
	 }
	 while (x > 3.14159265)
	 {
		  x -= 6.28318531;
	 }
	  
	if (x < 0)
	{
    sin = 1.27323954 * x + .405284735 * x * x;
    
    if (sin < 0)
        sin = .225 * (sin *-sin - sin) + sin;
    else
        sin = .225 * (sin * sin - sin) + sin;
	}
	else
	{
    sin = 1.27323954 * x - 0.405284735 * x * x;
    
    if (sin < 0)
        sin = .225 * (sin *-sin - sin) + sin;
    else
        sin = .225 * (sin * sin - sin) + sin;
	}
	return sin;
 }
 float fastSqrt( float n )
{
	// double a = (eventually the main method will plug values into a)
	//double a = (double) n;
	int i;
	float a =n;
	float x = 1;
 
	// For loop to get the square root value of the entered number.
	for(  i = 0; i < n; i++)
	{
		x = 0.5 * ( x+a / x );
	}
 
	return x;
} 
float fastACos(float x)
{
	return (-0.69813170079773 * x * x - 0.87266462599716) * x + 1.5707963267948;
}
float floatAbs(float x)
{
	if (x<0)
	{
		return -x;
	}
	return x;
}
float max(float x, float y)
{
	if (x>y)
	{
		return x;
	}
	return y;
}
float min(float x, float y)
{
	if (x<y)
	{
		return x;
	}
	return y;
}