/*
-----helpers.c-----
Declaration of helper functions such as
integer square root, vector rotation and normalization,
and fast trig functions.
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


void printText(const char *text, uint8_t xpos, uint8_t ypos) {
    os_SetCursorPos(ypos, xpos);
    os_PutStrFull(text);
}

// Draw small text at the given X/Y location 
void printTextSmall(const char *text, uint8_t xpos, uint8_t ypos) {
    os_FontSelect(0); // sets small font (1 == big, see docs)
    os_FontDrawText(text, xpos, ypos);
}



//copied from online, changes a float into a string
#include <stdio.h>
#define PSH(X) (*(buf++)=(X))
#define PSH1(X) (*(buf--)=(X))
#define PEEK() buf[-1]
#define POP() *(--buf) = '\0'
#define PLUS 1
#define SPACE 2
char * gcvt(double f, size_t ndigit, char * buf)
{
  int i;
	unsigned long z,k;
  int exp = 0;
  char *c = buf;
  double f2,t,scal;
  int   sign = 0;

  if((int)ndigit == -1)
    ndigit = 5;

  /* Unsigned long long only allows for 20 digits of precision
   * which is already more than double supports, so we limit the
   * digits to this.  long double might require an increase if it is ever
   * implemented.
   */
  if (ndigit > 20)
	  ndigit = 20;
  
  if (f < 0.0) {
    sign = 1;
    f = -f;
	 buf++;
  }

  scal = 1;
  for (i=ndigit; i>0; i--)
	  scal *= 10;
  k = f + 0.1 / scal;
  f2 = f - k;
  if (!f) {
    PSH('0');
    if(ndigit > 0)
      PSH('.');
    for (i=0;i<ndigit;i++)
      PSH('0');
  	   PSH(0);
  	 return c;
  }

  i = 1;
  while (f >= 10.0) {
  	f /= 10.0;
  	i++;
  }

  buf += i + ndigit + 1; 	

  PSH1(0);

  if(ndigit > 0) {	
	  t = f2 * scal;
	 z = t + 0.5;
    for (i = 0;i < ndigit;i++)
    {
      PSH1('0'+ (z % 10));
	   z /= 10;
    }
    PSH1('.');
  }
  else
    PSH1(0);

  do {
    PSH1('0'+ (k % 10));
    k /= 10;
  }while (k);

  if (sign)
    PSH1('-');
  return c;
}
