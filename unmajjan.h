#ifndef UNMJ_H
#define UNMJ_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>

#include <stdint.h>

#include <png.h>

typedef enum boolean { false, true } bool;
typedef enum orient	 { original, left, upsideDown, right } orient;

XWindowAttributes *windowAttributes;

struct windowInfo {

	uint32_t width;
	uint32_t height;

} windowInfo;


typedef struct image {

	char *name;
	char *id;
	
	int x, y;						/*	window coordinates	*/
	
	uint32_t initX, initY;			/*	image initial (x,y)	*/

	uint32_t endX, endY;			/*	image end (x,y)		*/

	short zDepth;					/*	objects having smaller zDepth are closer */

	uint8_t transparencyColor[3];	/*	RGB color to consider transparent in image	*/

} image;

image unmj_image;


struct imgData {

	char *name;

	uint32_t width, height;

	uint8_t ***bitmap;	/* RGB combination of each pixel of image */
};

struct imageDisplayInfo {

	char *name;
	char *id;
	
	int x, y;	
	uint32_t initX, initY;
	uint32_t endX, endY;
	short zDepth;
	uint8_t transparencyColor[3];

	float scalingFactor;
	uint32_t store_xy[4];

	bool mirror;

	orient orientation;

	struct imgDataNode *imgData;

} *startDisplayInfo;


struct imgDataNode {
	
	struct imgData image;
	struct imgDataNode *next;

} *sData;

struct imgNode {

	struct imageDisplayInfo image;
	struct imgNode *next;

} *sDispInfo;


/* Stores RGB pixel values of entire window */
uint8_t ***frame;

/* Stores position of image(corresponding to RGB values in frame) in linked list */
int **obj_placement;

int **collision_matrix;


typedef struct currentFrame {

	char **imageId;
	uint32_t n;

	uint8_t blankColor[3];
	
} currentFrame;

currentFrame unmj_currentFrame;



typedef struct framePosition {

	int src_x, src_y;		/* Offset w.r.t image origin */

	int dest_x, dest_y;		/* Offset w.r.t window origin */
	
	uint32_t width, height; /* width, height of image to display */

} framePosition;

framePosition unmj_fPos;



/* Return the X and Y dimensions of the window */
int unmj_winWidth();
int unmj_winHeight();


/* Generates random number within a range */
int unmj_random( int min, int max );

/* Open a new graphics window. */
void unmj_openWindow( uint32_t width, uint32_t height, const char *title );

/* Draw a point at (x,y) */
void unmj_drawPoint( int x, int y );

/* Draw a line from (x1,y1) to (x2,y2) */
void unmj_drawLine( int x1, int y1, int x2, int y2 );

/* Draw Rectangle */
void unmj_drawRectangle( int x, int y, int width, int height );
void unmj_drawFillRectangle(  int x, int y, int width, int height );

/* Draw Arc */
void unmj_drawArc( int x, int y, uint32_t width, uint32_t height, int angle1, int angle2 );
void unmj_drawFillArc( int x, int y, uint32_t width, uint32_t height, int angle1, int angle2 );

/* Polygon */
void unmj_drawFillPolygon( XPoint *points, int npoints, int mode );

/* mode = CoordModeOrigin  or CoordModePrevious 

  1. CoordModeOrigin	= treats all coordinates as relative to the origin
  2. CoordModePrevious	= treats all coordinates after the first as relative to the previous point

*/

/* Draw String */

/* Check available fonts from given command :

	xlsfonts -fn '*-*-*-*-*-*-*-*-*-*-*-*'


	Example :

	-val-free helvetian condensed-bold-o-condensed--0-0-0-0-p-0-iso10646-1

	* Here font name is "free helvetian condensed"

*/
void unmj_drawStringFont( char *name, uint32_t size );
void unmj_drawString( int x, int y, const char *string, int length );

/* Change the current drawing color. */
void unmj_color( int red, int green, int blue );

/* Clear the graphics window to the background color. */
void unmj_clearScreen();

/* Change the current background color. */
void unmj_clearColor( int red, int green, int blue );


/* 
	****** Create Frames ******
*/

void unmj_frame();

/* add image to linked list */
void unmj_addImage();

/* Remove image from linked list and free memory occupied by it */
void unmj_removeImage( char *id );

/* returns link to node of linked list containing given image */
struct imgNode *unmj_block( char *id );

/* Draws frame on screen using frame array */
void unmj_renderFrame();

/* Returns position of image in linkedList */
uint unmj_linkedListPos( char *id );

/* Returns id of image */
char *unmj_linkedListID( uint pos );

/* Returns length of linked list containg DisplayInfo */
uint unmj_linkedListLength();


/* Flush all previous output to the window. */
void unmj_flushOutput();

/* closes window and free memory occupied by certian DS */
void unmj_closeWindow();




/*
	****** unmj_move ******

	* move objects by specified number of pixels
	* collision detection
	* only objects having same zDepth will collide

*/
uint32_t unmj_move(char *id, int pixelSteps, bool horizontal, char ***colidedWith);



/* 
	****** Image Manipulation ******	
*/


/* Scales Image */
void unmj_scaleImage(char *id, float scaleFactor);

/* use it everytime before rescaling */
void unmj_undoScale(char *id);

/* Mirrors Image */
void unmj_mirrorImage(char *id);

/*		 Orientation of Image

	original, left, upsideDown, right
	
*/
void unmj_orientImage(char *id, orient orientation);




/* 
	****** Input ******
*/

/* Wait for the user to press a key or mouse button. */

/*	ev = KeyPress		( Key is pressed )
	ev = KeyRelease		( Key is released, used for keys like : ctrl,shift,alt )
	ev = ButtonPress	( Mouse Button Pressed )
	ev = ButtonRelease	( Mouse Button Released )
	ev = MotionNotify	( Mouse Key Moved )

	unmj_waitInput() returns :

	ev = KeyPress or KeyRelease			-> ASCII value of Key
	ev = ButtonPress or ButtonRelease	-> Button1 (LMB) or Button2 (MMB) or Button3 (RMB) or Button4 or Button5 (wheel up or down)


	ev = MotionNotify 					-> 0

	x,y coordinates of mouse key are stored whenever unmj_waitInput() is called

	(true/false)

	kPress				: recognize key Press as input
	kRelease			: recognize key release as input
	bPress				: recognize button Press as input
	bRelease			: recognize mouse button release as input
	trackMousePointer	: recognize mouse pointer movement as input
*/

char unmj_waitInput( int *ev, bool kPress, bool kRelease, bool bPress, bool bRelease, bool trackMousePointer );

/* Check to see if an event is waiting. */

/*	returns 1 when interrupt otherwise 0 

	if unmj_eventWaiting() returns 1 : use unmj_waitInput() after it to reterive input


	Example :

	int input, event;
	while()
		{
			...

			if(unmj_eventWaiting())
				{
					input = unmj_waitInput(&event);
				}

			...
		}
	
*/
bool unmj_eventWaiting( bool kPress, bool kRelease, bool bPress, bool bRelease, bool trackMousePointer );

/* Return the X and Y coordinates of the last event. */
int unmj_xPos();
int unmj_yPos();

#endif