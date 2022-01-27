/** g++ main_v1.cpp gl_frontEnd.cpp imageIO_TGA.cpp rasterImage.cpp -lm -framework OpenGL -framework GLUT -o focus  */
#include <iostream>
#include <string>
#include <algorithm>
#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<vector>
#include "gl_frontEnd.h"
#include "imageIO_TGA.h"

using namespace std;

/**==================================================================================
*Function prototypes
==================================================================================*/
void myKeyboard(unsigned char c, int x, int y);
void initializeApplication(int argc, char** argv);


/**==================================================================================
*	Application-level global variables
==================================================================================*/

/**	Don't touch. These are defined in the front-end source code*/
extern int	gMainWindow;
int abc = 0;
char* output;
/**	Don't rename any of these variables/constants*/
unsigned int numLiveFocusingThreads = 0;		/**	the number of live focusing threads*/
const int MAX_NUM_MESSAGES = 8;
const int MAX_LENGTH_MESSAGE = 32;
char** message;
int numMessages;
time_t launchTime;

/**------------------------------------------------------------------
*	The variables defined here are for you to modify and add to
------------------------------------------------------------------*/
#define IN_PATH		"./DataSets/Series02/"
#define OUT_PATH	"./Output/"

typedef struct ThreadInfo{
	pthread_t id;
	int index;
	int startRow, endRow;
	ImageStruct* Img;
}ThreadInfo;

/** global var. */
vector<ImageStruct*>ImageStack;
ImageStruct* imgOut;
pthread_mutex_t mutex;
int keepgoing = 0;
/** heard size, to set up the small windows, 3*2+1, 7*7 */
const int hS = 3;
ThreadInfo* info;

/**-------------------------------------------------------------------------+
 from lab6, passing the rgb value then gets the max Grayval */
unsigned char ucmax(unsigned char r, unsigned char g, unsigned char b){
	unsigned char maxVal = b;
	if (r > maxVal)
	{
		if (g> r)
		{
			maxVal = g;
		}
		else
		{
			maxVal = r;
		}
	}
	else if (g > maxVal)
	{
		maxVal = g;
	}
	
	return maxVal;
}
unsigned char toGrayMax_(unsigned char* rgba)
{
	return ucmax(rgba[0], rgba[1], rgba[2]);
}
/**-------------------------------------------------------------------------+
*this function gets the gray value of the select area, push them into vector,
use the max and min gets the bigest and smallest, retrun the subtraction of them which is the contrast.*/
int computeContrast(int k, int row, int col)
{
	int startI = max(row-hS,0), endI = min(row+hS, int(ImageStack[k]->height-1));
	int startJ = max(col-hS, 0), endJ = min(col+hS, int(ImageStack[k]->width-1));
	vector<int>gray_value;
    /**gets the gray value of every piexel. */
	for (int i=startI; i<=endI; i++){
		for (int j=startJ; j<=endJ; j++){ 
			unsigned char* rgbaIn = static_cast<unsigned char*>(ImageStack[k]->raster) + i*ImageStack[k]->bytesPerRow + 4*j;
			gray_value.push_back(int(toGrayMax_(rgbaIn)));
		}
	}
	int smallest_element = *min_element(gray_value.begin(),gray_value.end());
	int largest_element  = *max_element(gray_value.begin(),gray_value.end());
	gray_value.clear(); /** Empty the vector */
	return largest_element-smallest_element;
}
/** threadfunc*/
void *threadfunc(void* arg){
	ThreadInfo* info = (ThreadInfo*) arg;
	int temp=0;
	double sum = 0;
	double add = 1;
	time_t begin, end;
	time(&begin); //for EC1
	// int temp=0;
	for (int row =info->startRow; row<=info->endRow;row++){
		for (int col=0;col<ImageStack[0]->width;col++){
			int best=-1;
			for (int k=0;k<ImageStack.size();k++){
				int cases; 
                /** if the return gray value bigger than the current, make it for the tmep.
                finnal gets the best. */
				cases=computeContrast(k,row,col);
				if (cases>temp){
					temp=cases;
					best=k;
				}
			}
			/**copy the best piexel into the output image*/
			int* pixelIn = static_cast <int*>(ImageStack[best]->raster)+row*ImageStack[best]->width+col;
            int* pixelOut = static_cast <int*>(imgOut->raster)+row*ImageStack[best]->width+col;
            *pixelOut=*pixelIn;
			temp=0;/** reset the temp.*/
		}
	}
	time(&end);
	time_t elapsed = end - begin;
	printf("Time measured: %ld seconds.\n", elapsed);
}
/**==================================================================================
	These are the functions that tie the computation with the rendering.
	Some parts are "don't touch."  Other parts need your intervention
	to make sure that access to critical section is properly synchronized
==================================================================================*/

void displayImage(GLfloat scaleX, GLfloat scaleY)
{
	/**==============================================
	*	This is OpenGL/glut magic.  Don't touch
	==============================================*/
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPixelZoom(scaleX, scaleY);

	/**--------------------------------------------------------
	*	stuff to replace or remove.
	*	Here I assign a random color to a few random pixels
	--------------------------------------------------------*/

	/**==============================================
	*	This is OpenGL/glut magic.  Don't touch
	==============================================*/
	glDrawPixels(imgOut->width, imgOut->height,
				  GL_RGBA,
				  GL_UNSIGNED_BYTE,
				  imgOut->raster);

}


void displayState(void)
{
	/**==============================================
	*	This is OpenGL/glut magic.  Don't touch
	==============================================*/
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	/**--------------------------------------------------------
	*	stuff to replace or remove.
	*--------------------------------------------------------
	*	Here I hard-code a few messages that I want to see displayed in my state
	*	pane.  The number of live focusing threads will always get displayed
		(as long as you update the value stored in the.  No need to pass a message about it.*/
	time_t currentTime = time(NULL);
	numMessages = 4;
	sprintf(message[0], "System time: %ld", currentTime);
	sprintf(message[1], "Time since launch: %ld", currentTime-launchTime);
	sprintf(message[2], "Jason Lao version 1, press esc to quit");
	sprintf(message[3], "it takes 30 second to load image to GUI");
	
	/**---------------------------------------------------------
	*	This is the call that makes OpenGL render information
	*	about the state of the simulation.
	*	You may have to synchronize this call if you run into
	*	problems, but really the OpenGL display is a hack for
	*	you to get a peek into what's happening.
	---------------------------------------------------------*/
	drawState(numMessages, message);
}

/**	This callback function is called when a keyboard event occurs
	You can change things here if you want to have keyboard input
*/
void handleKeyboardEvent(unsigned char c, int x, int y)
{
	int ok = 0;
	
	switch (c)
	{
		/**	'esc' to quit */
		case 27:
	    for(int k =0; k<abc;k++){
		    void* useless;
			/*show in front end.*/
		    pthread_join(info[k].id, &useless);
	}
            writeTGA(output,imgOut);
			exit(0);
			break;

		/**	Feel free to add more keyboard input, but then please document that
			in the report.*/
		
		
		default:
			ok = 1;
			break;
	}
	if (!ok)
	{
		
	}
}

/**------------------------------------------------------------------------
*	You shouldn't have to change anything in the main function.
------------------------------------------------------------------------*/
int main(int argc, char** argv)
{

    initializeApplication(argc,argv);

	initializeFrontEnd(argc, argv, imgOut);
	
	/**==============================================
	*	This is OpenGL/glut magic.  Don't touch
	*==============================================
	*	Now we enter the main loop of the program and to a large extend
	*	"lose control" over its execution.  The callback functions that
	*	we set up earlier will be called when the corresponding event
		occurs*/
	glutMainLoop();
	
	/**	Free allocated resource before leaving (not absolutely needed, but
	*	just nicer.  Also, if you crash there, you know something is wrong
		in your code. */
	for (int k=0; k<MAX_NUM_MESSAGES; k++)
		free(message[k]);
	free(message);
	return 0;
}


/**==================================================================================
*call initializeApplication in main
==================================================================================*/

void initializeApplication(int argc, char** argv){   
    
    int numThreads;
	int tempthread = stoi(argv[1]);
    abc=tempthread;
    output=argv[2];
	if(tempthread>argc-3){
		numThreads= argc-3;
	}else{
		numThreads= tempthread;
	}
    for(int i=3;i<argc;i++){
        const string hardCodedInput =argv[i];
		ImageStack.push_back(readTGA(hardCodedInput.c_str()));
        launchTime = time(NULL);
	}

    imgOut = new ImageStruct(ImageStack[0]->width, ImageStack[0]->height, ImageStack[0]->type, 1);
	int p = ImageStack[0]->height % numThreads;
	int m = ImageStack[0]->height / numThreads;
	int startRow = 0,endRow = m-1;
	info = (ThreadInfo*) calloc(numThreads, sizeof(ThreadInfo));
    for(int k =0; k<numThreads;k++){	
		if(k<p){
		    endRow++;
		}
        /** init */
		info[k].index = k;
		info[k].startRow = startRow;
		info[k].endRow = endRow;
		info[k].Img = ImageStack[k];
		startRow = endRow + 1; /** 0,6,11,16 */
		endRow +=m; /** {5,5,5,5}=5+5+5+5 */
	}
    for(int k =0; k<numThreads;k++){
        /**every threads runs the code below.*/
		pthread_create(&info[k].id,nullptr,threadfunc,info+k);
		numLiveFocusingThreads+=1;
	}

	message = (char**) malloc(MAX_NUM_MESSAGES*sizeof(char*));
	for (int k=0; k<MAX_NUM_MESSAGES; k++)
		message[k] = (char*) malloc((MAX_LENGTH_MESSAGE+1)*sizeof(char));
	
	srand((unsigned int) time(NULL));
}



