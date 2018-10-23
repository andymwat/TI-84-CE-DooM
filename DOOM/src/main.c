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

//A ray is defined by two float tuples, its origin and its direction.
typedef struct 
{
	float origin[2];
	float direction[2];

}Ray;
//An rgb color.
//Unusued right now
typedef struct 
{
	uint8_t r,g,b;
}Color;
//A face is defined by two points and a color index.
typedef struct 
{
	float point1[2];
	float point2[2];
	uint8_t color;
}Face;
//An object (or sprite) is defined by its position and a pointer to the graphics sprite that it uses.
typedef struct{
	float position[2];
	gfx_sprite_t * sprite;	
}Object;

//The distance between cast rays
int lineSpacing = 20;
float focalLength = 5.0;
//Height of walls
float multiplier = 50;

//The horizontal FOV, measured in radians. The default is 0.5pi, or 90 degrees
float fov = M_PI/2;
//Whether or not father faces are rendered darker than closer ones. Not yet implemented
const bool DISTANCE_FOG = true;
//Idk why I made this variable, and it isnt used.
const int RENDER_WIDTH = 100;
//Ditto.
const float MAX_DISTANCE = 2;
//The vector that defines what direction the camera is looking at. 
//Should be normalized, but its not the end of the world if it isn't.
float lookDirection[] = {0,1};
//The vector that defines where the camera is.
float playerPosition[] = {0,0};

const int FACEARRAYMAXSIZE = 10;
//A pointer to the list of faces. It is dynamically allocated on the beginning of the program, with a size of FACEARRAYMAXSIZE
Face * faceArray;
//Current number of faces in the list of faces.
static int faceArrayCurrentSize = 0;
//Current number of objects in the list of objects.
static int numberOfObjects = 0;

#define OBJECTARRAYMAXSIZE  10

//A pointer to the list of objects. It is dynamically allocated on the beginning of the program, with a size of OBJECTARRAYMAXSIZE
Object * objectArray;


void loadLevel();
Face * mallocAndGenerateFace(float , float, float, float , uint8_t);
void drawObjects();
void drawMap();
Face *ClosestFace(Ray*, float*, bool*);
void GetRayToFaceIntersection(Ray*, Face*, float * ,bool *);
void begin();
void end();
void step();
void draw();
bool partial_redraw;
kb_key_t key;


//The string displayed when the Not Yet Implemented error is thrown.
char * nyiError = "Not Yet Implemented";
//Creates sprites for enemy1 and the gun.
gfx_UninitedSprite(akSprite, doomak2_width,doomak2_height);
gfx_UninitedSprite(enemy1Sprite, enemy1_width, enemy1_height);
void main() {
	
	int i;
	Face * testFace;
	Face * testFace2;	
	Face * testFace3;
	Face * testFace4;
	
	float  * destination  = calloc(2 , sizeof(float));
	bool  hit = false;
	faceArray = calloc(FACEARRAYMAXSIZE ,sizeof(Face));

	//create test walls
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
    loadLevel();
	begin(); // No rendering allowed!

	
	//sets up not yet implemented error message
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
		
	//runs until 2nd is pressed
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
	//free memory
	free(destination);
	free(testFace);
	free(testFace2);
	free(testFace3);
	free(faceArray);
	free(akSprite);
}

/* Put other functions here */
void begin() {	
		gfx_SetTransparentColor(gfx_group_transparent_color_index);
	
}

void end() {
    /* Implement me! */
}



void step() {
  	float distanceFromCam;
	bool hit;
	Ray movementRay;
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
	//rotation
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

//does the main drawing
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
	
	//clear the screen
	gfx_FillScreen(255);
	
	//center lookray
	lookRay.direction[0] = lookDirection[0];
	lookRay.direction[1] = lookDirection[1];
	lookRay.origin[0] = playerPosition[0];
	lookRay.origin[1] = playerPosition[1];
	//rotate the ray to the very left in preparation for spinning it around
	rotateVector(&lookRay.direction, fov/2);
	angleFromCenter = fov/2;
	
	//loop for each vertical line
	
	for (row = 0; row <LCD_WIDTH; row+= lineSpacing){
	
		//rotate ray by a small amount
		rotateVector(&(lookRay.direction), -((float)fov / LCD_WIDTH) * lineSpacing);
		angleFromCenter += -((float)fov / LCD_WIDTH) * lineSpacing;
		//cast the ray
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

//returns the closest face a ray hits, and sets the distance and whether or not the ray hit anything
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

//outputs whether or not a ray hit a certain face, and if so the distance
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



 //draws the map in the top right corner
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
	gfx_SetColor(245);
	rotateVector(&lookview, fov/2);
	gfx_Line_NoClip(playerPosition[0]* scale + LCD_WIDTH-25,  playerPosition[1] * -scale + 25,   playerPosition[0]* scale + LCD_WIDTH-25 + lookview[0] * scale * 4 ,  playerPosition[1]* -scale + 25 + lookview[1] * -scale * 4);
	rotateVector(&lookview, -fov);
	gfx_Line_NoClip(playerPosition[0]* scale + LCD_WIDTH-25,  playerPosition[1] * -scale + 25,   playerPosition[0]* scale + LCD_WIDTH-25 + lookview[0] * scale * 4 ,  playerPosition[1]* -scale + 25 + lookview[1] * -scale * 4);
	
 }

//draws the sprites on the screen (not the gun sprite)
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

//creates and mallocs a face, and returns a pointer to it
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

//frees all of the objects' sprites
void unloadLevel()
{
	int j =0;
	for (j=0;j<numberOfObjects;j++)
	{
		free(objectArray[j].sprite);
	}
	free(objectArray);
}

