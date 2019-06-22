#include "unmajjan.h"

static Display 	*display = 0;
static Window  	window;
static GC      	gc;
static Colormap colormap;
static int      fast_color_mode = 0;


/* unmj_frame */
XImage *ximage;
Visual *visual; 
char *image32;

/* These values are saved by unmj_waitInput then retrieved later by unmj_xpos and unmj_ypos */

static int saved_xpos = 0;
static int saved_ypos = 0;


int unmj_random(int min, int max)
{
	return rand()%(max-min+1)+min;
}

/* Open a new graphics window */

void unmj_openWindow( uint32_t windowWidth, uint32_t windowHeight, const char *title )
{
	display = XOpenDisplay(0);

	if(!display) 
		{
			fprintf(stderr, "unmj_openWindow : unable to open the graphics window.\n");
			
			exit(1);
		}

	Visual *visual = DefaultVisual(display, 0);

	if(visual && visual->class == TrueColor) 
		{
			fast_color_mode = 1;
		} 
	else 
		{
			fast_color_mode = 0;
		}

	int blackColor = BlackPixel(display, DefaultScreen(display));
	int whiteColor = WhitePixel(display, DefaultScreen(display));

	window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, windowWidth, windowHeight, 0, blackColor, blackColor);

	/* save window width and window height in struct windowInfo */

	windowInfo.height 	= windowHeight;
	windowInfo.width	= windowWidth;

	XSetWindowAttributes attr;
	attr.backing_store = Always;

	XChangeWindowAttributes(display, window, CWBackingStore, &attr);

	XStoreName(display, window, title);

	XSelectInput(display, window, StructureNotifyMask|KeyPressMask|ButtonPressMask|PointerMotionMask|KeyReleaseMask|ButtonReleaseMask);

	XMapWindow(display, window);

	gc = XCreateGC(display, window, 0, 0);

	colormap = DefaultColormap(display, 0);

	XSetForeground(display, gc, whiteColor);


	visual = DefaultVisual(display, 0);

	/* Memory allocation for frame */
	frame = malloc(windowInfo.height * sizeof(uint8_t **));

		for(int i=0; i < windowInfo.height; i++)
			{
				frame[i] = malloc(windowInfo.width * sizeof(uint8_t *));

				for(int j=0; j<windowInfo.width; j++)
					{
						frame[i][j] = malloc(3 * sizeof(uint8_t));
					}
			}

	image32 = malloc(windowInfo.width * windowInfo.height * 4);

	/* initialise frame position */
	unmj_fPos = (struct framePosition){0,0,0,0,windowInfo.width,windowInfo.height};

	sData = 0;
	sDispInfo = 0;

	/* Memory allocation for obj_placement */
	obj_placement = malloc(windowInfo.height * sizeof(int *));

		for(int i=0; i < windowInfo.height; i++)
			{
				obj_placement[i] = malloc(windowInfo.width * sizeof(int));
			}

	/* Memory allocation for collision_matrix */
	collision_matrix = malloc(windowInfo.height * sizeof(int *));
			
		for(int i=0; i < windowInfo.height;  i++)
			{
				collision_matrix[i] = malloc(windowInfo.width * sizeof(int));
			}

	/* Wait for the MapNotify event */

	for(;;) 
		{
			XEvent e;

			XNextEvent(display, &e);

			if (e.type == MapNotify)
				break;
		}

	windowAttributes = malloc(sizeof(XGetWindowAttributes));
	
}


int unmj_winWidth()
{
	XGetWindowAttributes(display, window, windowAttributes);
	return windowAttributes->width;
}

int unmj_winHeight()
{
	XGetWindowAttributes(display, window, windowAttributes);
	return windowAttributes->height;
}

void abort_(const char * s, ...)
{
    va_list args;

    va_start(args, s);

    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");

    va_end(args);

    abort();
}




/*
	****** Drawing Functions ******
*/


/* Draw a single point at (x,y) */

void unmj_drawPoint( int x, int y )
{
	XDrawPoint(display, window, gc, x, y);
}


/* Draw a line from (x1,y1) to (x2,y2) */

void unmj_drawLine( int x1, int y1, int x2, int y2 )
{
	XDrawLine(display, window, gc, x1, y1, x2, y2);
}


/* Draw Rectangle */

void unmj_drawRectangle( int x, int y, int width, int height )
{
	XDrawRectangle( display, window, gc, x, y, width, height); 
}

void unmj_drawFillRectangle(  int x, int y, int width, int height )
{
	XFillRectangle( display, window, gc, x, y, width, height);
}


/* Draw Arc */

void unmj_drawArc( int x, int y, uint32_t width, uint32_t height, int angle1, int angle2)
{
	XDrawArc(display, window, gc, x, y, width, height, angle1 * 64, angle2 * 64);
}

void unmj_drawFillArc( int x, int y, uint32_t width, uint32_t height, int angle1, int angle2 )
{
	XFillArc(display, window, gc, x, y, width, height, angle1 * 64, angle2 * 64);
}


/* Polygon */
void unmj_drawFillPolygon( XPoint *points, int npoints, int mode )
{
	XFillPolygon(display, window, gc, points, npoints, Complex, mode);
}


/* Draw String */

void unmj_drawStringFont( char *name, uint32_t size )
{
	XFontStruct *font;

	char *fontname = malloc((37+strlen(name)) * sizeof(char));
	sprintf(fontname, "-*-%s-*-r-*-*-%u-*-*-*-*-*-*-*", name, size);

	font = XLoadQueryFont (display, fontname);
	free(fontname);

	XSetFont (display, gc, font->fid);
}

void unmj_drawString( int x, int y, const char *string, int length)
{
	XDrawString(display, window, gc, x, y, string, strlen(string));	
}


/* Change the current drawing color */

void unmj_color( int R, int G, int B)
{
	XColor color;

	if(fast_color_mode) 
		{
			/* If this is a truecolor display, we can just pick the color directly */

			color.pixel = ((B&0xff) | ((G&0xff)<<8) | ((R&0xff)<<16) );
		} 

	else 
		{
			/* Otherwise, we have to allocate it from the colormap of the display */

			color.pixel = 0;
			color.red 	= R << 8;
			color.green = G << 8;
			color.blue 	= B << 8;

			XAllocColor(display, colormap, &color);
		}

	XSetForeground(display, gc, color.pixel);
}


/* Clear the graphics window to the background color */

void unmj_clearScreen()
{
	XClearWindow(display, window);
}


/* Change the current background color */

void unmj_clearColor( int R, int G, int B )
{
	XColor color;

	color.pixel = 0;
	color.red 	= R << 8;
	color.green = G << 8;
	color.blue 	= B << 8;

	XAllocColor(display, colormap, &color);

	XSetWindowAttributes attr;
	attr.background_pixel = color.pixel;

	XChangeWindowAttributes(display, window, CWBackPixel, &attr);
}




/* General functions */

uint32_t unmj_linkedListPos(char *id)
{
	struct imgNode *block = sDispInfo;
	int c = 0;

	while(block != 0)
		{
			c++;
			if(strcmp(block->image.id,id)==0)
				return c;

			else 
				block = block->next;
		}

	return 0;
}

char *unmj_linkedListID(uint32_t pos)
{
	struct imgNode *block = sDispInfo;
	int c = 0;

	while(block != 0)
		{
			c++;
			if(c == pos)
				return block->image.id;

			else 
				block = block->next;
		}

	return "\0";
}

uint32_t unmj_linkedListLength()
{
	struct imgNode *block = sDispInfo;
	
	uint32_t len=0;
	while(block != 0)
		{
			len++;
			block = block->next;
		}

	return len;
}

int maximum(int *arr, uint32_t n)
{
	int max = 0;
	for(int i=0; i < n; i++)
		{
			if(*(arr+i) > *(arr+max))
				{
					max = i;
				}
		}

	return max;
}

int minimum(int *arr, uint32_t n)
{
	int min = 0;
	for(int i=0; i < n; i++)
		{
			if(*(arr+i) < *(arr+min))
				{
					min = i;
				}
		}
		
	return min;
}




/*
	****** Create Frames ****** 
*/


void unmj_renderFrame()
{
    char *p = image32;

	for(int i=0; i < windowInfo.height; i++)
		{
			for(int j=0; j < windowInfo.width; j++)
				{
					*p++ = frame[i][j][0]; /* B */
					*p++ = frame[i][j][1]; /* G */
					*p++ = frame[i][j][2]; /* R */

					p++;
				}
		}

	ximage = XCreateImage(display, visual, 24, ZPixmap, 0, image32, windowInfo.width, windowInfo.height, 32, 0);

	XPutImage(display, window, gc, ximage, unmj_fPos.src_x, unmj_fPos.src_y, unmj_fPos.dest_x, unmj_fPos.dest_y, unmj_fPos.width, unmj_fPos.height);
}

short imageInFrame(char *id)
{
	for(uint32_t i=0; i < unmj_currentFrame.n; i++)
		{
			if(strcmp(unmj_currentFrame.imageId[i], id)==0)
				return true;
		}
	
	return false;
}


void swap(int *x, int *y)
{
	int tmp = *x;
	(*x) = *y;
	(*y) = tmp;
}
/* Select image to display at a particular (x,y) window coordinate */

void setPixelValues(int x, int y, uint8_t *color)
{

	struct imgNode *i = sDispInfo;
	struct imgNode *minZDepth;

	short firstFlag = 0;
	float scale;

	int bx = 0, by  = 0;
	for(int j, bitmap_x, bitmap_y, x1, y1, x2, y2; i != NULL; i=i->next)
		{
			x1 = i->image.y, y1 = i->image.x;

			if(i->image.orientation == left || i->image.orientation == right)
				{
					x2 = i->image.endX - i->image.initX + i->image.y;
					y2 = i->image.endY - i->image.initY + i->image.x;
					
					bitmap_x = y - i->image.x + i->image.initX;
					bitmap_y = x - i->image.y + i->image.initY;

					switch(i->image.orientation)
						{
							case original   : break;
							case left       : bitmap_x = i->image.endY - bitmap_x; break;
							case upsideDown : break;
							case right      : bitmap_y = i->image.endX - bitmap_y; break;
						}
				}

			else 
				{
					x2 = i->image.endY - i->image.initY + i->image.y;
					y2 = i->image.endX - i->image.initX + i->image.x;

					bitmap_x = x - i->image.y + i->image.initY;
					bitmap_y = y - i->image.x + i->image.initX;

					if(i->image.orientation == upsideDown)
						{
							bitmap_x = i->image.endY - bitmap_x;
						}
				}

			/* point within rectangular region of image */

			if(x >= x1 && x <= x2 && y >= y1 && y <= y2 && imageInFrame(i->image.id))
				{
					if(i->image.mirror)
						{
							bitmap_y = i->image.endX - bitmap_y;
						}

					scale = i->image.scalingFactor;

					bitmap_x /= scale;
					bitmap_y /= scale;

					for(j=0; j < 3; j++)
						{
							if(i->image.imgData->image.bitmap[bitmap_x][bitmap_y][j] != i->image.transparencyColor[j])
								break;
						}
						
					if(j==3)
						continue;
						
					else
						{
							if(firstFlag == 0)
								{
									firstFlag = 1;
									minZDepth = i;
									bx = bitmap_x;
									by = bitmap_y;
								}
							else 
								{
									if(i->image.zDepth < minZDepth->image.zDepth)
										{
											minZDepth = i;
											bx = bitmap_x;
											by = bitmap_y;
										}
								}
						}
				}
		}

	/* No image has any pixel to be drawn in given (x,y) */
	if(firstFlag == 0)
		{
			color[0] = unmj_currentFrame.blankColor[2];
			color[1] = unmj_currentFrame.blankColor[1];
			color[2] = unmj_currentFrame.blankColor[0];

			obj_placement[x][y] = 0;
		}
		
	else 
		{
			color[0] = minZDepth->image.imgData->image.bitmap[bx][by][0];
			color[1] = minZDepth->image.imgData->image.bitmap[bx][by][1];
			color[2] = minZDepth->image.imgData->image.bitmap[bx][by][2];

			obj_placement[x][y] = unmj_linkedListPos(minZDepth->image.id);
		}
}


/* insert appropriate pixel values ( RGB ) in frame */

void createFrame()
{
	uint8_t color[3];
	for(int i=0; i < windowInfo.height; i++)
		{
			for(int j=0; j < windowInfo.width; j++)
				{
					/* Select image to display at a particular (x,y) window coordinate */
					setPixelValues(i, j, color);

					frame[i][j][0] = color[0];
					frame[i][j][1] = color[1];
					frame[i][j][2] = color[2];
				}
		}
}



/* Allocate memory to bitmap */

void mallocBitmap(struct imgDataNode *i)
{
	i->image.bitmap = malloc(i->image.height * sizeof(uint8_t **));

	for(int x=0; x < i->image.height; x++)
		{
			i->image.bitmap[x] = malloc(i->image.width * sizeof(uint8_t *));

			for(int y=0; y < i->image.width; y++)
				{
					i->image.bitmap[x][y] = malloc(3 * sizeof(uint8_t));
				}
		}
}

/* determine required attributes of file */

void readPngFile(struct imgDataNode *i)
{
	png_structp png_ptr;
	png_infop info_ptr;

	png_bytep *row_pointers;
	
	/* 8 is the maximum size that can be checked */
	png_byte header[8];

	/* open file and test for it being a png */

	char *file_name = malloc(strlen(i->image.name)+strlen(".png")+1);
	strcpy(file_name, i->image.name);
	strcat(file_name, ".png");

	FILE *fp = fopen(file_name, "rb");

	free(file_name);


    if (!fp)
    	abort_("[readPngFile] File %s.png could not be opened for reading", i->image.name);

    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8))
		abort_("[readPngFile] File %s.png is not recognized as a PNG file", i->image.name);


	/* initialize stuff */

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        abort_("[readPngFile] png_create_read_struct failed");

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
        abort_("[readPngFile] png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[readPngFile] Error during init_io");

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    i->image.width	= png_get_image_width  (png_ptr, info_ptr);
	i->image.height	= png_get_image_height (png_ptr, info_ptr);

    png_read_update_info(png_ptr, info_ptr);


    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[readPngFile] Error during read_image");

    row_pointers = malloc(sizeof(png_bytep) * i->image.height); //(png_bytep*)

	for (int y=0; y < i->image.height; y++)
		{
			row_pointers[y] = malloc(png_get_rowbytes(png_ptr, info_ptr)); //(png_byte*)
		}

    png_read_image(png_ptr, row_pointers);

	fclose(fp);

	if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
        abort_("[readPngFile] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
                "(lacks the alpha channel)");

    if(png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
        {
			abort_("[readPngFile] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
				PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));
		}

	mallocBitmap(i);

	/* Assign values to bitmap */

	for(int x=0; x < i->image.height; x++)
		{
			png_byte* row = row_pointers[x];

			for(int y=0; y < i->image.width; y++)
				{
					png_byte* ptr = &(row[y*4]);

					i->image.bitmap[x][y][0] = ptr[2]; /* B */
					i->image.bitmap[x][y][1] = ptr[1]; /* G */
					i->image.bitmap[x][y][2] = ptr[0]; /* R */
				}
		}
	
	free(row_pointers);
}

void copyBitmap(struct imgDataNode *i, uint8_t ***bitmap, uint32_t x, uint32_t y)
{
	mallocBitmap(i);

	/* Assign values to bitmap */

	for(int x=0; x < i->image.height; x++)
		{
			for(int y=0; y < i->image.width; y++)
				{
					i->image.bitmap[x][y][0] = bitmap[x][y][0]; /* B */
					i->image.bitmap[x][y][1] = bitmap[x][y][1];	/* G */
					i->image.bitmap[x][y][2] = bitmap[x][y][2];	/* R */
				}
		}
}

void addDataNode(char *name, char *id, bool readImg, uint8_t ***bitmap, uint32_t x, uint32_t y)
{
	struct imgDataNode *block;

	block = sData;
	while(block != NULL)
		{
			if(strcmp(block->image.name, name)==0)
				{
					unmj_block(id)->image.imgData = block;
					break;
				}

			block = block->next;
		}

	struct imgDataNode *new;
	if(block == NULL)
		{
			new = malloc(sizeof(struct imgDataNode));

			new->image.name = name;

			switch(readImg)
				{
					case true  : readPngFile(new);				break;
					case false :
								new->image.height = x;
								new->image.width  = y;
								copyBitmap (new, bitmap, x, y);	break;
				}

			new->next = NULL;

			if(sData == NULL)
				{
					sData = new;
				}

			else 
				{
					block = sData;
					
					while(block->next != NULL)
						block = block->next;

					block->next = new;
				}

			unmj_block(id)->image.imgData = new;
		}
}

/* add image to linked list */
void unmj_addImage() 
{
	struct imgNode *block, *new;

	new = malloc(sizeof(struct imgNode));

	memcpy(&(new->image), &unmj_image, sizeof(unmj_image));

	new->image.scalingFactor = 1;
	new->image.mirror = false;
	new->image.orientation = original;


    new->next = NULL;

    if (sDispInfo == 0)
        {
            sDispInfo = new;
        }
    
    else
        {
            block = sDispInfo;

            while (block->next != NULL)
                block = block->next;

            block->next = new;
		}

	addDataNode(unmj_image.name, unmj_image.id, true, 0,0,0);
}

	



/* Remove image from linked list and free memory occupied by it */
short removeDispInfoNode(char *id)
{
	struct imgNode *block, *old;
    
	block = sDispInfo;
	while(block != 0)
        {
            if(strcmp(block->image.id,id)==0)
                {
                    if(block == sDispInfo)
                        {
							sDispInfo = block->next;
							free(block);
							
                            return 0;
                        }
                    
                    else
                        {
							old->next = block->next;
							free(block);

                            return 0;
                        }
                }
            
            else
                {
					old = block;
					block = block->next;
                }
        }

    printf("\n[unmj_removeImage] No entry found for id = %s\n",id);

    return 0;               
}

short removeDataNode(char *name)
{
	struct imgDataNode *block, *old;
    
	block = sData;
	while(block != 0)
        {
            if(strcmp(block->image.name,name)==0)
                {
                    if(block == sData)
                        {
							sData = block->next;

							free(block->image.bitmap);
							free(block);
							
                            return 0;
                        }
                    
                    else
                        {
							old->next = block->next;

							free(block->image.bitmap);
							free(block);

                            return 0;
                        }
                }
            
            else
                {
					old = block;
					block = block->next;
                }
        }

    return 0;               
}

bool multipleCopies(char *name)
{
	struct imgNode *block = sDispInfo;

	int copies = -1;
	while(block != 0)
		{
			if(strcmp(block->image.name,name)==0)
				{
					copies++;
				}

			block = block->next;
		}

	if(copies == 0)
		return false;
	else 
		return true;
}

void unmj_removeImage(char *id)
{
	struct imgNode *block = sDispInfo;
	char *name;

	while(block != 0)
	{
		if(strcmp(block->image.id,id)==0)
			{
				name = block->image.name;
				break;
			}

		block = block->next;
	}

	removeDispInfoNode(id);

	if(!multipleCopies(name))
		removeDataNode(name);
}


/* returns link to node of linked list containing given image */

struct imgNode *unmj_block(char *id)
{
	struct imgNode *block = sDispInfo;

	while(block != NULL)
		{
			if(strcmp(block->image.id, id) == 0)
				{
					return block;
				}

			block = block->next;
		}

	return 0;
}


void unmj_frame()
{
	createFrame();
	unmj_renderFrame();
}




/* Flush all previous output to the window */

void unmj_flushOutput()
{
	XFlush(display);
}

/* Closes Window */

void unmj_closeWindow()
{
	XDestroyImage(ximage);

	free(windowAttributes);
	
	XWithdrawWindow(display, window, 0);
	
	free(frame);
	free(obj_placement);
	free(collision_matrix);

	struct imgNode *i = sDispInfo, *block;
	while(i != 0)
		{
			block = i;
			i=i->next;

			free(block);
		}

	struct imgDataNode *j = sData, *dataBlock;
	while(j != 0)
		{
			dataBlock = j;
			j=j->next;

			free(dataBlock->image.bitmap);
			free(dataBlock);
		}
}





/* Collision Detection */

void initCollisionMatrix(int zDepth)
{
	struct imgNode *n;
	uint32_t pos;

	float scale;
	for(int x=0; x < windowInfo.height; x++)
		{
			for(int y=0; y < windowInfo.width; y++)
				{
					/* Select image to display at a particular (x,y) window coordinate */
					n = sDispInfo;
					pos = 0;
					for(int i, bitmap_x, bitmap_y, x1, y1, x2, y2; n != NULL; n=n->next)
						{
							pos++;

							x1 = n->image.y, y1 = n->image.x;

							if(n->image.orientation == left || n->image.orientation == right)
								{
									x2 = n->image.endX - n->image.initX + n->image.y;
									y2 = n->image.endY - n->image.initY + n->image.x;

									
									bitmap_x = y - n->image.x + n->image.initX;
									bitmap_y = x - n->image.y + n->image.initY;

									switch(n->image.orientation)
										{
											case original   : break;
											case left       : bitmap_x = n->image.endY - bitmap_x; break;
											case upsideDown : break;
											case right      : bitmap_y = n->image.endX - bitmap_y; break;
										}
								}

							else 
								{
									x2 = n->image.endY - n->image.initY + n->image.y;
									y2 = n->image.endX - n->image.initX + n->image.x;

									bitmap_x = x - n->image.y + n->image.initY;
									bitmap_y = y - n->image.x + n->image.initX;

									if(n->image.orientation == upsideDown)
										{
											bitmap_x = n->image.endY - bitmap_x;
										}
								}
							
							/* point within rectangular region of image */

							if(n->image.zDepth==zDepth && x >= x1 && x <= x2 && y >= y1 && y <= y2 && imageInFrame(n->image.id))
								{
									if(n->image.mirror)
										{
											bitmap_y = n->image.endX - bitmap_y;
										}

									scale = n->image.scalingFactor;

									bitmap_x /= scale;
									bitmap_y /= scale;

									for(i=0; i < 3; i++)
										{
											if(n->image.imgData->image.bitmap[bitmap_x][bitmap_y][i] != n->image.transparencyColor[i])
												break;
										}

									if(i==3)
										continue;

									else 
										break;
								
								}	
						}

					if(n == 0)
						{
							collision_matrix[x][y] = 0;
						}

					else 
						{
							collision_matrix[x][y] = pos;
						} 
				}
		}
}


void imageRegion(int pos, int *xmin, int *ymin, int *xmax, int *ymax)
{
	bool first = true;
	for(int i=0; i < windowInfo.height; i++)
		{
			for(int j=0; j < windowInfo.width; j++)
				{
					if(collision_matrix[i][j] == pos)
						{
							if(first)
								{
									(*xmin) = (*xmax) = i;
									(*ymin) = (*ymax) = j;

									first = false;
								}

							else 
								{
									if(i < *xmin)
										(*xmin) = i;
									
									if(i > *xmax)
										(*xmax) = i;

									if(j < *ymin)
										(*ymin) = j;

									if(j > *ymax)
										(*ymax) = j;
								}
						}
				}
		}
}

bool update_collision_matrix(int i, int j, int pos, bool horizontal,int pixelSteps)
{
	if(horizontal)
		{
			if((collision_matrix[i][j] == pos) && (collision_matrix[i][j+pixelSteps] == 0 || collision_matrix[i][j+pixelSteps]==pos))
				return true;
			else 
				if(collision_matrix[i][j] != pos)
					return true;
			else 
				return false;
		}


	else
		{
			if((collision_matrix[i][j] == pos) && (collision_matrix[i+pixelSteps][j] == 0 || collision_matrix[i+pixelSteps][j]==pos))
				return true;
			else 
				if(collision_matrix[i][j] != pos)
					return true;
			else 
				return false;
		}
}

void detObstacles(int min, int max, int pos, int *obstacle, int *n, int obstacleLine, bool horizontal)
{
	int i, j;

	if(horizontal)
		{
			for(i=min; i<=max; i++)
				{
					if((collision_matrix[i][obstacleLine] != 0) && (collision_matrix[i][obstacleLine] != pos))
						{
							for(j=0; j <= *n; j++)
								{
									if(obstacle[j] == collision_matrix[i][obstacleLine])
										break;
								}

							if(j > *n)
								{
									obstacle[++(*n)] = collision_matrix[i][obstacleLine];
								}
						}
				}
		}

	else 
		{
			for(i=min; i<=max; i++)
				{
					if((collision_matrix[obstacleLine][i] != 0) && (collision_matrix[obstacleLine][i] != pos))
						{
							for(j=0; j <= *n; j++)
								{
									if(obstacle[j] == collision_matrix[obstacleLine][i])
										break;
								}

							if(j > *n)
								{
									obstacle[++(*n)] = collision_matrix[obstacleLine][i];
								}
						}
				}
		}
}

/*
	****** unmj_move ******

	* move objects by specified number of pixels
	* collision detection
	* only objects having same zDepth will collide

*/

uint32_t unmj_move(char *id, int pixelSteps, bool horizontal, char colidedWith[][20])
{
	struct imgNode *img = unmj_block(id);
	initCollisionMatrix(img->image.zDepth);

	int pos = unmj_linkedListPos(id);

	int xmin, ymin, xmax, ymax;
	imageRegion(pos, &xmin, &ymin, &xmax, &ymax);

	int i, j;
	int minMove = 0;

	int *obstacle, nObstacles;

	nObstacles = -1;

	if(pixelSteps > 0)
		{
			if(horizontal)
				{
					obstacle = malloc((xmax-xmin) * sizeof(int));

					for(minMove=1; (minMove <= pixelSteps) && (ymax+minMove < windowInfo.width); minMove++)
						{
							for(i = ymax; i >= ymin;  i--)
								{
									for(j = xmin; j <= xmax; j++)
										{
											if(!update_collision_matrix(j,i,pos,horizontal,minMove))
													break;
										}
									
									if(j != xmax+1) 
										break;
								}

							if(i != ymin-1) 
								break;
						}

						
					if((minMove-1 != pixelSteps) && (i+minMove < windowInfo.width) && minMove != 1)
						{
							detObstacles(xmin, xmax, pos, obstacle, &nObstacles, i+minMove, horizontal);
						}

					img->image.x += minMove-1;
				}


			else
				{
					obstacle = malloc((ymax-ymin) * sizeof(int));

					for(minMove=1; (minMove <= pixelSteps) && (xmax+minMove < windowInfo.height); minMove++)
						{
							for(i = xmax; i >= xmin;  i--)
								{
									for(j = ymin; j <= ymax; j++)
										{
											if(!update_collision_matrix(i,j,pos,horizontal,minMove))
												break;
										}
									
									if(j != ymax+1) 
										break;
								}

							if(i != xmin-1)
								break;
						}

					if(minMove-1 != pixelSteps && i+minMove < windowInfo.height && minMove != 1)
						{
							detObstacles(ymin, ymax, pos, obstacle, &nObstacles, i+minMove, horizontal);
						}

					img->image.y += minMove-1;
				}
		}

	else 
		{
			if(horizontal)
				{
					obstacle = malloc((xmax-xmin) * sizeof(int));

					for(minMove=1; (minMove <= -pixelSteps) && (ymin-minMove >= 0); minMove++)
						{
							for(i = ymin; i <= ymax;  i++)
								{
									for(j = xmin; j <= xmax; j++)
										{
											if(!update_collision_matrix(j,i,pos,horizontal,-minMove))
													break;
										}
									
									if(j != xmax+1) 
										break;
								}

							if(i != ymax+1) 
								break;
						}

					if(minMove-1 != pixelSteps && i-minMove >= 0 && minMove != 1)
						{
							detObstacles(xmin, xmax, pos, obstacle, &nObstacles, i-minMove, horizontal);
						}

					img->image.x -= minMove-1;
				}


		else
			{
				obstacle = malloc((ymax-ymin) * sizeof(int));

				for(minMove=1; (minMove <= -pixelSteps) && (xmin-minMove >= 0); minMove++)
					{
						for(i = xmin; i <= xmax;  i++)
							{
								for(j = ymin; j <= ymax; j++)
									{
										if(!update_collision_matrix(i,j,pos,horizontal,-minMove))
											break;
									}
								
								if(j != ymax+1) 
									break;
							}

						if(i != xmax+1)
							break;
					}

				if(minMove-1 != pixelSteps && i-minMove >= 0 && minMove != 1)
					{
						detObstacles(ymin, ymax, pos, obstacle, &nObstacles, i-minMove, horizontal);
					}

				img->image.y -= minMove-1;
			}
		}
/*
	if(nObstacles != -1)
		{
			(*colidedWith) = malloc((nObstacles+1) * sizeof(char *));

			for(i=0; i <= nObstacles; i++)
				{
					(*colidedWith)[i] = malloc(strlen(unmj_linkedListID(obstacle[i])) * sizeof(char));
					strcpy((*colidedWith)[i], unmj_linkedListID(obstacle[i]));
				}
		}
*/

	for(i=0; i<=nObstacles; i++)
		{
			strcpy(colidedWith[i], unmj_linkedListID(obstacle[i]));
		}

	free(obstacle);

	return nObstacles+1;
}




/* Scales image */
void unmj_scaleImage(char *id, float scaleFactor)
{
	struct imgNode *img = unmj_block(id);

	img->image.scalingFactor = scaleFactor;

	img->image.store_xy[0] = img->image.initX;
	img->image.store_xy[1] = img->image.initY;
	img->image.store_xy[2] = img->image.endX;
	img->image.store_xy[3] = img->image.endY;

	img->image.initX *= scaleFactor;
	img->image.initY *= scaleFactor;
	img->image.endX	 *= scaleFactor;
	img->image.endY	 *= scaleFactor;
}

/* use it everytime before rescaling */
void unmj_undoScale(char *id)
{
	struct imgNode *img = unmj_block(id);

	img->image.initX = img->image.store_xy[0];
	img->image.initY = img->image.store_xy[1];
	img->image.endX	 = img->image.store_xy[2];
	img->image.endY  = img->image.store_xy[3];

	img->image.scalingFactor = 1;
}

/* Mirrors image */
void unmj_mirrorImage(char *id)
{
	struct imgNode *img = unmj_block(id);

	img->image.mirror = !(img->image.mirror);
}

/*		 Orientation of Image

	original, left, upsideDown, right
	
*/
void unmj_orientImage(char *id, orient orientation)
{
	unmj_block(id)->image.orientation = orientation;
}




/*
	****** Input Functions ******
*/


/* Wait for the user to press a key or mouse button */

char unmj_waitInput(int *ev, bool kPress, bool kRelease, bool bPress, bool bRelease, bool trackMousePointer)
{
	XEvent event;

	unmj_flushOutput();

	for(;;)
		{
			XNextEvent(display,&event);

			(*ev) = event.type;

			if(event.type==KeyPress && kPress) 
				{
					saved_xpos = event.xkey.x;
					saved_ypos = event.xkey.y;

					return XLookupKeysym(&event.xkey,0);
				}
			else 
				if(event.type==KeyRelease && kRelease)
					{
						saved_xpos = event.xkey.x;
						saved_ypos = event.xkey.y;

						return XLookupKeysym(&event.xkey,0);
					}
				
			else 
				if(event.type==ButtonPress && bPress) 
					{
						saved_xpos = event.xkey.x;
						saved_ypos = event.xkey.y;
			
						return event.xbutton.button;
					}
			else 
				if(event.type == ButtonRelease && bRelease)
					{
						saved_xpos = event.xkey.x;
						saved_ypos = event.xkey.y;

						return event.xbutton.button;
					}

			else 
				if(event.type==MotionNotify && trackMousePointer)
					{
						saved_xpos = event.xmotion.x;
						saved_ypos = event.xmotion.y;

						return 0;
					}
			else
				return 0;
		}
}

bool unmj_eventWaiting(bool kPress, bool kRelease, bool bPress, bool bRelease, bool trackMousePointer)
{
	XEvent event;

    unmj_flushOutput();

    for(;;)
	   	{
            if(XCheckMaskEvent(display, -1, &event)) 
				{
                    if(event.type == KeyPress && kPress) 
					   	{
                            XPutBackEvent(display, &event);
                               
							return 1;
						}

					else 
						if(event.type == KeyRelease && kRelease)
							{
								XPutBackEvent(display, &event);
                               
								return 1;
							}
						    
						
					else
						if(event.type == ButtonPress && bPress) 
							{
                               	XPutBackEvent(display, &event);
                               		
								return 1;
							}
					else 
						if(event.type == ButtonRelease && bRelease)
							{
								XPutBackEvent(display, &event);
									
								return 1;
							}
					else
						if(event.type == MotionNotify && trackMousePointer)
							{
								XPutBackEvent(display, &event);

								return 1;
							}

					else
						{
                            return 0;
                       	}
               	}

			else 
				{
                    return 0;
               	}
       	}
}

/* Return the X and Y coordinates of the last event */

int unmj_xPos()
{
	return saved_xpos;
}

int unmj_yPos()
{
	return saved_ypos;
}