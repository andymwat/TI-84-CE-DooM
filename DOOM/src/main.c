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
#include "gfx/gfx_group.h"
#include "lookup_table.h"
#include "helpers.h"
typedef struct 
{
	float origin[2];
	float direction[2];

}Ray;
typedef struct 
{
	uint8_t r,g,b;
}Color;
typedef struct 
{
	float point1[2];
	float point2[2];
	uint8_t color;
}Face;
typedef struct{
	float position[2];
	gfx_sprite_t * sprite;	
}Object;

int lineSpacing = 20;
float focalLength = 5.0;
float multiplier = 50;

float fov = M_PI/2;
const bool DISTANCE_FOG = true;
const int RENDER_WIDTH = 100;
const float MAX_DISTANCE = 2;
float lookDirection[] = {0,1};
float playerPosition[] = {0,0};
const int FACEARRAYMAXSIZE = 10;
Face * faceArray;
static int faceArrayCurrentSize = 0;
static int numberOfObjects = 0;
#define OBJECTARRAYMAXSIZE  10

Object * objectArray;


void loadLevel();
void printTextSmall(const char *, uint8_t , uint8_t);
void printText(const char *, uint8_t, uint8_t );
Face * mallocAndGenerateFace(float , float, float, float , uint8_t);
void drawObjects();
void drawMap();

Face *ClosestFace(Ray*, float*, bool*);
void GetRayToFaceIntersection(Ray*, Face*, float * ,bool *);

char * gcvt(float, size_t, char *);
void begin();
void end();
void step();
void draw();
bool partial_redraw;
kb_key_t key;



char * nyiError = "Not Yet Implemented";
//gfx_sprite_t * akSprite;
gfx_UninitedSprite(akSprite, doomak2_width,doomak2_height);
gfx_UninitedSprite(enemy1Sprite, enemy1_width, enemy1_height);
void main() {
	
	int i;
	Ray * testRay = malloc(sizeof(Ray));
	Face * testFace;
	Face * testFace2;	
	Face * testFace3;
	Face * testFace4;
	
	float  * destination  = calloc(2 , sizeof(float));
	//float destination[2];
	bool  hit = false;
	faceArray = calloc(FACEARRAYMAXSIZE ,sizeof(Face));
	testRay->origin[0] = 0;
	testRay->origin[1] = -1;
	testRay->direction[0] = 0;
	testRay->direction[1] = 1;
	testFace = mallocAndGenerateFace(-5,6,5,6, 65);
	testFace2 = mallocAndGenerateFace(-5,-5,5,-5,12);
	testFace3 = mallocAndGenerateFace(-7,7,-7,-7,02);
	testFace4 = mallocAndGenerateFace(4,-9,8,2,196);
	faceArray[0] = *testFace;
	faceArrayCurrentSize++;
	faceArray[1] = *testFace2;
	faceArrayCurrentSize++;
	faceArray[2] = *testFace3;
	faceArrayCurrentSize++;
	faceArray[3] = *testFace4;
	faceArrayCurrentSize++;
	//generateLookupTable(1000, 1);

    loadLevel();
	begin(); // No rendering allowed!
	

	
	//sets up NYI error message
	for (i = 0; i < 20; i++)
	{
		os_AppErr1[i] =nyiError[i];
		
	}
	
	
	//allocates and decompresses gun sprite
	akSprite = gfx_MallocSprite(doomak2_width,doomak2_height);
	zx7_Decompress(akSprite, doomak2_compressed);
	
    gfx_Begin();
    gfx_SetDrawBuffer(); // Draw to the buffer to avoid rendering artifats
	
	kb_Scan();
	key = kb_Data[7];
		
    while (kb_Data[1] !=kb_2nd) { 
		step();// No rendering allowed in step!
        if (partial_redraw) // Only want to redraw part of the previous frame?
            gfx_BlitScreen(); // Copy previous frame as a base for this frame
        draw(); // As little non-rendering logic as possible
        gfx_SwapDraw(); // Queue the buffered frame to be displayed
    }

    gfx_End();
    end();
	
	
	os_ClrHome();
	
	
	while (!os_GetCSC());
	free(destination);
	free(testFace);
	free(testFace2);
	free(testFace3);
	free(testRay);
	free(faceArray);
	free(akSprite);
	//freeLookupTable();
}

/* Put other functions here */
void begin() {	
		gfx_SetTransparentColor(gfx_group_transparent_color_index);
	
}

void end() {
    /* Implement me! */
}
Ray movementRay;
void step() {
  	float distanceFromCam;
	  bool hit;
	movementRay.origin[0] = playerPosition[0];
	movementRay.origin[1] = playerPosition[1];
	//keypad input
	kb_Scan();
	key = kb_Data[7];
	//back
	 if (kb_Data[4] & kb_2) {
		 	movementRay.direction[0] =  -lookDirection[0];
			movementRay.direction[1] =  -lookDirection[1];
			ClosestFace(&movementRay, &distanceFromCam, &hit);
			if  ( !hit || distanceFromCam > 0.5 )
			{
           		playerPosition[0] = playerPosition[0] + lookDirection[0] * -0.5;
				playerPosition[1] = playerPosition[1] + lookDirection[1] * -0.5;
			}
			
        } 
		//forward
        if (kb_Data[4] & kb_8) {
			movementRay.direction[0] =  lookDirection[0];
			movementRay.direction[1] =  lookDirection[1];
			ClosestFace(&movementRay, &distanceFromCam, &hit);
			if  ( !hit || distanceFromCam > 0.5 )
			{
            	playerPosition[0] = playerPosition[0] + lookDirection[0] * 0.5;
				playerPosition[1] = playerPosition[1] + lookDirection[1] * 0.5;
			}
			
        } 
		//left
        if (kb_Data[3] & kb_4) {
			movementRay.direction[0] =  -lookDirection[1];
			movementRay.direction[1] =  lookDirection[0];
			ClosestFace(&movementRay, &distanceFromCam, &hit);
			if  ( !hit || distanceFromCam > 0.5 )
			{
            	playerPosition[0] = playerPosition[0] - lookDirection[1] * 0.5;
				playerPosition[1] = playerPosition[1] + lookDirection[0] * 0.5;
			}
        } 
		//right
        if (kb_Data[5] & kb_6) {
			movementRay.direction[0] =  lookDirection[1];
			movementRay.direction[1] =  -lookDirection[0];
			ClosestFace(&movementRay, &distanceFromCam, &hit);
			if  ( !hit || distanceFromCam > 0.5 )
			{
            	playerPosition[0] = playerPosition[0] + lookDirection[1] * 0.5;
				playerPosition[1] = playerPosition[1] - lookDirection[0] * 0.5;
			}
			
        } 
		if (kb_Data[3] & kb_7)
		{
			rotateVector(&lookDirection, 0.2);
		}
		if (kb_Data[5] & kb_9)
		{
			rotateVector(&lookDirection, -0.2);
		}
		if (kb_Data[7] & kb_Up)
		{
			fov+= 0.1;
		}
		if (kb_Data[7] & kb_Down)
		{
			fov-= 0.1;
		}
		if (kb_Data[7] & kb_Left)
		{
			lineSpacing++;
		}
		if (kb_Data[7] & kb_Right)
		{
			if (lineSpacing > 1)
			{
				lineSpacing--;
			}
		}
		if (kb_Data[6] & kb_Add)
		{
			multiplier+= 5;
		}
		if (kb_Data[6] & kb_Sub)
		{
			if (multiplier > 5)
			{
				multiplier-= 5;
			}
		}
		if (kb_Data[6] & kb_Mul)
		{
			focalLength += 0.5;
		}
		if (kb_Data[6] & kb_Div)
		{
			if (focalLength > 0.5)
			{
				focalLength -= 0.5;
			}
		}
}

void draw() {
	int row = LCD_WIDTH/2;
	int fillSpacing = 0;
	int temp = 0;
	float closeDist = -1;
	float fardist = 2;
	float res[2];
	char buf[10];
	bool hmm = false;
	Face * hit ;
	Ray  lookRay ;
	float newClose;
	float angleFromCenter = 0;
	//145,156,255,221,205,190, or 165
	gfx_FillScreen(255);
	
	lookRay.direction[0] = lookDirection[0];
	lookRay.direction[1] = lookDirection[1];
	lookRay.origin[0] = playerPosition[0];
	lookRay.origin[1] = playerPosition[1];
	//rotate the ray to the very left in preparation for spinning it around
	rotateVector(&lookRay.direction, fov/2);
	angleFromCenter = fov/2;
	
	//loop for each vertical line
	
	for (row = 0; row <LCD_WIDTH; row+= lineSpacing){
	
		//rotate ray by a small amount and then cast again
		rotateVector(&(lookRay.direction), -((float)fov / LCD_WIDTH) * lineSpacing);
		angleFromCenter += -((float)fov / LCD_WIDTH) * lineSpacing;
		hit = ClosestFace(&lookRay, &closeDist, &hmm);
		
		//if hit
		if (hmm && hit != 0)
		{
			
			gfx_SetColor(hit->color);
			closeDist = closeDist * fastCos(angleFromCenter);
			temp =(focalLength/closeDist) * multiplier;
			if (temp < 0)
			{
				temp = 0;
			}
			if (temp > LCD_HEIGHT/2 -1)
			{
				temp = LCD_HEIGHT/2 -1;
			}
			/*
				for (fillSpacing = 1; fillSpacing < lineSpacing && fillSpacing + row < LCD_WIDTH; fillSpacing++)
				{
					gfx_Line_NoClip(row+ fillSpacing, LCD_HEIGHT/2.0 + temp, row+ fillSpacing, LCD_HEIGHT/2.0 - temp);
				}
			*/
			fillSpacing = lineSpacing;
			if (row + fillSpacing >= LCD_WIDTH)
			{
				fillSpacing = LCD_WIDTH-row;
			}
			
			gfx_FillRectangle_NoClip(row,LCD_HEIGHT/2 - temp,fillSpacing, 2 * temp );
		}
		
		
		
	}
		//draw position and rotation text
		gcvt(playerPosition[0], 3, &buf);
		gfx_SetTextFGColor(245);
		gfx_SetTextXY(0,0);
		gfx_PrintString(&buf);
		gcvt(playerPosition[1], 3, &buf);
		gfx_SetTextXY(75,0);
		gfx_PrintString(&buf);
		gcvt(lookDirection[0], 3, &buf);
		gfx_SetTextXY(0,20);
		gfx_PrintString(&buf);
		gcvt(lookDirection[1], 3, &buf);
		gfx_SetTextXY(75,20);
		gfx_PrintString(&buf);
		
		//draw objects
		drawObjects();
		
		//draw gun sprite
		gfx_SetPalette(gfx_group_pal, sizeof_gfx_group_pal,0);
		gfx_ScaledTransparentSprite_NoClip(akSprite, (LCD_WIDTH /2) - 68 , LCD_HEIGHT - doomak2_height * 2, 2,2);
		
		//draw crosshair
		gfx_SetColor(245);
		gfx_Line_NoClip(LCD_WIDTH/2 - 2, LCD_HEIGHT/2, LCD_WIDTH/2 + 2, LCD_HEIGHT/2);
		gfx_Line_NoClip(LCD_WIDTH/2 , LCD_HEIGHT/2 - 2, LCD_WIDTH/2 , LCD_HEIGHT/2 + 2);
		

		//draw the overhead map
		drawMap();
		

	
}
Face *ClosestFace(Ray *ray, float *distance, bool * hitOrNot)
{
	float closestDist;
	int i;
	float hitpoint[2];
	//float * hitpoint  = malloc(2 * sizeof(float));
	float dist = 0;
	bool  didhit;
	//pointer to the face in facearray that was hit
	Face * outface = NULL;
	*distance = -1;
	closestDist = 999999;
	*hitOrNot = false;
	
	//check if passed null pointers
	if (ray == NULL || distance == NULL || hitOrNot == NULL)
	{
		return outface;
	}
	
	for (i = 0; i<faceArrayCurrentSize; i++)
	{
		
		GetRayToFaceIntersection(ray, &(faceArray[i]), &hitpoint, &didhit);
		
		//if successfully hit something
		if (didhit)
		{
			
			*hitOrNot = true;
			dist = distanceBetween(ray->origin, hitpoint);
			//if closer than closest face so far, set face that its currently checking to be the closest face
			if (dist < closestDist)
			{
				closestDist = dist;
				outface = &(faceArray[i]);
			}
		}
		
		//*distance = closestDist;
		//if closest dist was changed
		if (closestDist < 999999)
		{
			
			*hitOrNot = true;
			*distance = closestDist;
			return outface;
		}
	}
	
	return outface;
}

void GetRayToFaceIntersection(Ray * ray, Face * face, float * result, bool * hit)
{
	float v1[2] = {0,0};
	float v2[2]= {0,0};
	float v3[2]= {0,0};
	float tOne = 0;
	float tTwo = 0;
	float dot = 0;
	v1[0] =(ray->origin[0] - face->point1[0]);
	v1[1] = (ray->origin[1] - face->point1[1]);
	v2[0] = (face->point2[0] - face->point1[0]);
	v2[1]= (face->point2[1] - face->point1[1]);
	v3[0] = -(ray->direction[1]);
	v3[1] = (ray->direction[0]);
	dot = (v2[0] * v3[0]) + (v2[1] * v3[1]);
	
	*hit = false;
	if (abs(dot) < 0.000001)
	{
		return;
	}
	tOne = ((v2[0] * v1[1]) - (v2[1] * v1[0])) / dot;
	tTwo = ((v1[0] * v3[0])+ (v1[1] * v3[1])) / dot;
	
	if (tOne>0.0 && (tTwo >=0.0 && tTwo <= 1.0))
	{
		
		*hit = true;
		
		//tOne is distance to point1
		//multiply by normalized direction and add to origin to get point
		(result)[0] = ray->origin[0] + (ray->direction[0] * tOne);
		(result)[1] = ray->origin[1] + (ray->direction[1] * tOne);
		
	}
	
}


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

 
 void drawMap()
 {
	 float lookview[2];
	int face = 0;
	float scale = 1.25;
	lookview[0] = lookDirection[0];
	lookview[1] = lookDirection[1];
	//draw map box
	gfx_SetColor(245);
	gfx_Line_NoClip(LCD_WIDTH -5, 5, LCD_WIDTH - 5, 50);
	gfx_Line_NoClip(LCD_WIDTH -5, 5, LCD_WIDTH - 50, 5);
	gfx_Line_NoClip(LCD_WIDTH -50, 5, LCD_WIDTH - 50, 50);
	gfx_Line_NoClip(LCD_WIDTH -50, 50, LCD_WIDTH - 5, 50);

	//draw faces
	for (face = 0; face < faceArrayCurrentSize; face++)
	{
		gfx_SetColor(faceArray[face].color);
		gfx_Line_NoClip(faceArray[face].point1[0] * scale + LCD_WIDTH-25,  faceArray[face].point1[1]* -scale + 25,  faceArray[face].point2[0]* scale+ LCD_WIDTH-25,  faceArray[face].point2[1]* -scale + 25 );
	
	}

	//draw player
	gfx_Line_NoClip(playerPosition[0]* scale + LCD_WIDTH-25 - 1,  playerPosition[1]* -scale +25,  playerPosition[0]* scale + LCD_WIDTH-25 + 1, playerPosition[1]* -scale +25);
	gfx_Line_NoClip(playerPosition[0]* scale + LCD_WIDTH-25,  playerPosition[1]* -scale +25 -1,  playerPosition[0]* scale + LCD_WIDTH-25, playerPosition[1]* -scale +25 +1);
	
	//draw view
	//
	gfx_SetColor(245);
	rotateVector(&lookview, fov/2);
	gfx_Line_NoClip(playerPosition[0]* scale + LCD_WIDTH-25,  playerPosition[1] * -scale + 25,   playerPosition[0]* scale + LCD_WIDTH-25 + lookview[0] * scale * 4 ,  playerPosition[1]* -scale + 25 + lookview[1] * -scale * 4);
	rotateVector(&lookview, -fov);
	gfx_Line_NoClip(playerPosition[0]* scale + LCD_WIDTH-25,  playerPosition[1] * -scale + 25,   playerPosition[0]* scale + LCD_WIDTH-25 + lookview[0] * scale * 4 ,  playerPosition[1]* -scale + 25 + lookview[1] * -scale * 4);
	
 }

void drawObjects()
{
	
	int num =0;
	float dot;
	float dist;
	float distToFace;
	bool hit;
	int scale;
	float angleFromCenter=0;
	Ray testRay;
	float toObject[2];
	for (num = 0; num<numberOfObjects; num++)
	{
		
		toObject[0] = objectArray[num].position[0] - playerPosition[0];
		toObject[1] = objectArray[num].position[1] - playerPosition[1];
		dot = (lookDirection[0] * toObject[0]) + (lookDirection[1] * toObject[1]);

		//get the angle of the sprite from the center 
		angleFromCenter = -atan2((lookDirection[0]) *toObject[1] - (lookDirection[1]) * toObject[0], (lookDirection[0]) * toObject[0] + (lookDirection[1]) * toObject[1]);
		
		if (floatAbs(angleFromCenter) <= (fov/2))
		{
			//get distance to object
			dist = (distanceBetween(&playerPosition, &objectArray[num].position));
			//cast ray to nearest face and compare with distance to object
			testRay.origin[0] = playerPosition[0];
			testRay.origin[1] = playerPosition[1];
			testRay.direction[0] = toObject[0];
			testRay.direction[1] = toObject[1];
			ClosestFace(&testRay, &distToFace, &hit);
			if (hit && dist >distToFace)
			{
				//if the ray hit a face and that face is closer than the object, skip drawing it (it's obscured).
				continue;
			}

			//change distance to the scale factor
			dist =abs(1/((dist+3) *0.025)-2);
			//limit dist to a minimum scale of 1
			dist = max(dist,1);
			//set the color pallete
			gfx_SetPalette(gfx_group_pal, sizeof_gfx_group_pal,0);
			//draw the scaled sprite.
			gfx_ScaledTransparentSprite_NoClip(objectArray[num].sprite, (LCD_WIDTH/2) + (angleFromCenter *100 * fov) - (enemy1_width *dist)/2 ,LCD_HEIGHT/2 - (enemy1_height *dist)/2 ,dist,dist);
			
		}
	}
	
}


Face * mallocAndGenerateFace(float  point1x, float  point1y, float point2x, float point2y, uint8_t colorIndex)
{
	Face * newFace = malloc(sizeof(Face));
	newFace->point1[0] = point1x;
	newFace->point1[1] = point1y;
	newFace->point2[0] = point2x;
	newFace->point2[1] = point2y;
	newFace->color = colorIndex;
	return newFace;
}

void loadLevel()
{
	Object newObj;
	numberOfObjects++;
	//alloc object array
	objectArray = malloc(sizeof(Object) * numberOfObjects);
	//malloc sprite image
	enemy1Sprite = gfx_MallocSprite(enemy1_width,enemy1_height);
	//decompress sprite image
	zx7_Decompress(enemy1Sprite, enemy1_compressed);
	newObj.sprite =  enemy1Sprite;
	
	//set position
	newObj.position[0] = -2;
	newObj.position[1] = 2;
	//add to array
	objectArray[0] =newObj;
	
	
}

void unloadLevel()
{
	int j =0;
	for (j=0;j<numberOfObjects;j++)
	{
		free(objectArray[j].sprite);
	}
	free(objectArray);
}

void printText(const char *text, uint8_t xpos, uint8_t ypos) {
    os_SetCursorPos(ypos, xpos);
    os_PutStrFull(text);
}

/* Draw small text at the given X/Y location */
void printTextSmall(const char *text, uint8_t xpos, uint8_t ypos) {
    os_FontSelect(0); // sets small font (1 == big, see docs)
    os_FontDrawText(text, xpos, ypos);
}

