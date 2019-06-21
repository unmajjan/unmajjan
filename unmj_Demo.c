#include "unmajjan.h"

#define Esc 27

void setEndPts(char *id)
{
    unmj_block(id)->image.endX = unmj_block(id)->image.imgData->image.width     -1;
    unmj_block(id)->image.endY = unmj_block(id)->image.imgData->image.height    -1;
}

void message()
{
    unmj_color(255,255,255);
    unmj_drawFillRectangle(10,5, 200,110);

    unmj_color(255,0,0);
    unmj_drawStringFont("free times",15);

    unmj_drawString(20,20,  "w     :  Move Forward",     17);
    unmj_drawString(20,50,  "s     :  Move Backward",    18);
    unmj_drawString(20,80,  "x     :  Change Direction", 21);
    unmj_drawString(20,110, "Esc :  Quit", 11);

}

int car_x   = 0,      car_y   = 400;
int w1_x    = 9,      w2_x    = 176;
int w_y     = 448;


void addBackground()
{
    /* Background */

    /*
        Name : background
        ID   : bg

        (x,y) of window : (0,0)

        starting (x,y) of image : (0,0) (from beginning of image)
        ending (x,y) of image   : set to (0,0) and setEndPts() is called to set end points of image to last pts of image

        zDepth : 1

        transparency color : {0,0,0} i.e pixel with RGB combination - R=0, G=0, B=0, will be considered transparent and not be drawn
        
    */
    unmj_image = (image) { "Demo_images/background","bg", 0,0,0,0,0,0,1,{0,0,0} };
    unmj_addImage();

    setEndPts("bg");

    /* original image has much larger size than required so scale it */
    unmj_scaleImage("bg", 0.55); 

    /* I want sun at right side instead of left side (as in original image) */
    unmj_mirrorImage("bg");
}

void addCar()
{
    /* Car image */
    unmj_image = (image) { "Demo_images/car", "car", car_x,car_y,    0,0,0,0,0,   {255,255,255} };
    unmj_addImage();

    setEndPts("car");
    unmj_scaleImage("car", 0.4);

    /* wheel */
    /* Same image with different IDs */

    /* back wheel */
    unmj_image = (image) { "Demo_images/wheel", "wheel_1", w1_x, w_y, 0,0,0,0,-1, {255,255,255} };
    unmj_addImage();

    setEndPts("wheel_1");
    unmj_scaleImage("wheel_1",0.15);

    /* front wheel */
    unmj_image = (image) { "Demo_images/wheel", "wheel_2", w2_x, w_y, 0,0,0,0,-1, {255,255,255} };
    unmj_addImage();

    setEndPts("wheel_2");
    unmj_scaleImage("wheel_2",0.15);

}

void addObstacles()
{
    /* roadblock */

    /* zDepth = 0 */
    unmj_image = (image) { "Demo_images/roadblock","rblk", 700,350,0,0,0,0,0,{255,255,255} };
    unmj_addImage();

    setEndPts("rblk");

    /* railing */

    /* zDepth = -2 */
    unmj_image = (image) { "Demo_images/railing","railing", 100,250,0,0,0,0,-2,{255,255,255} };
    unmj_addImage();

    setEndPts("railing");
}


/* Move Car */

short direction = 1, steps = 50;
short rotation  = 0;
char **colidedWith;


void rotateWheel()
{
    struct imgNode *car = unmj_block("car");

    struct imgNode *wheel_1 = unmj_block("wheel_1");
    struct imgNode *wheel_2 = unmj_block("wheel_2");

    unmj_orientImage("wheel_1", rotation);
    unmj_orientImage("wheel_2", rotation);

    wheel_1->image.x = car->image.x + w1_x;
    wheel_2->image.x = car->image.x + w2_x;

    if(rotation < 3)
        rotation++;
    else 
        rotation = 0;
}


void moveCar(bool forward)
{
    struct imgNode *car = unmj_block("car");

    short f = forward ? 1: -1;

    if(car->image.x < 400 && car->image.x > steps)
        car->image.x += direction * steps *f; 
    else
        unmj_move("car", direction * steps *f, true, &colidedWith);

    rotateWheel();
}

void changeDirection()
{
    struct imgNode *car = unmj_block("car");

    if(car->image.x > 400)
        car->image.x -= 10;

    unmj_mirrorImage("car");

    rotateWheel();
    
    direction = -direction;
}


int main()
{
    unmj_openWindow(1024, 600, "UNMAJJAN DEMO");

    addBackground();
    addCar();
    addObstacles();
    

    /* Parameters for current frame */
    unmj_currentFrame = (currentFrame) { (char *[]){"bg","car","wheel_1","wheel_2","rblk","railing"}, 6, {0,0,0} };

    /* create and display frame */
    unmj_frame();
    
    /* Display Controls */
    message();

    int ev;
    char input = 0;

    while(input != Esc)
        {
            if(unmj_eventWaiting(1,0,0,0,0))
                {
                    input = unmj_waitInput(&ev,1,0,0,0,0);

                    switch(input)
                    {
                        case 'w' : moveCar(1);          break;
                        case 's' : moveCar(0);          break;
                        case 'x' : changeDirection();   break;
                    }

                    unmj_frame();
                    message();
                }
        }
        
    unmj_closeWindow();
}