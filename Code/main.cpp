#include <iostream>
#include <algorithm>
#include <string>
#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include<vector>
#include "imageIO_TGA.h"
#include "gl_frontEnd.h"
using namespace std;

 /*-------------------------------------------------------------------------+
 |	A program that takes a sets of photo and outputs the best piexel of 	|
 |	that position, to make the most concentration picture.				    | 
 |  command to runs.                                                        |
//  g++ -std=c++17 main.cpp imageIO_TGA.cpp rasterImage.cpp -o focus        |
 |                                                                          |
 |                                                                          |
 +-------------------------------------------------------------------------*/
// ./focus 12 ./Output/out.tga ./Data/_MG_6291.tga ./Data/_MG_6293.tga ./Data/_MG_6294.tga ./Data/_MG_6296.tga ./Data/_MG_6297.tga ./Data/_MG_6298.tga ./Data/_MG_6299.tga ./Data/_MG_6300.tga ./Data/_MG_6301.tga ./Data/_MG_6302.tga 



// define the Thread 


typedef struct ThreadInfo{
	pthread_t id;
	int index;
	int startRow, endRow;
	ImageStruct* Img;
}ThreadInfo;

// global var. 
vector<ImageStruct*>ImageStack;
ImageStruct* imgOut;
pthread_mutex_t mutex;
int keepgoing = 0;
// heard size, to set up the small windows, 3*2+1, 7*7
const int hS = 3;

const int MAX_NUM_MESSAGES = 8;
const int MAX_LENGTH_MESSAGE = 32;
char** message;
int numMessages;
time_t launchTime;

//-------------------------------------------------------------------------+
// from lab6, passing the rgb value then gets the max Grayval
unsigned char ucmax(unsigned char r, unsigned char g, unsigned char b)
{
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
//-------------------------------------------------------------------------+

//-------------------------------------------------------------------------+
//this function gets the gray value of the select area, push them into vector,
// use the max and min gets the bigest and smallest, retrun the subtraction of them which is the contrast.
int computeContrast(int k, int row, int col)
{
	int startI = max(row-hS,0), endI = min(row+hS, int(ImageStack[k]->height-1));
	int startJ = max(col-hS, 0), endJ = min(col+hS, int(ImageStack[k]->width-1));
	vector<int>gray_value;
    //gets the gray value of every piexel.
	for (int i=startI; i<=endI; i++){
		for (int j=startJ; j<=endJ; j++){ 
			unsigned char* rgbaIn = static_cast<unsigned char*>(ImageStack[k]->raster) + i*ImageStack[k]->bytesPerRow + 4*j;
			gray_value.push_back(int(toGrayMax_(rgbaIn)));
		}
	}
	int smallest_element = *min_element(gray_value.begin(),gray_value.end());
	int largest_element  = *max_element(gray_value.begin(),gray_value.end());
	gray_value.clear(); // Empty the vector
	return largest_element-smallest_element;
}
//-------------------------------------------------------------------------+

//-------------------------------------------------------------------------+
// threadfunc
void *threadfunc(void* arg){
	ThreadInfo* info = (ThreadInfo*) arg;
	int temp=0;
	double sum = 0;
	double add = 1;
	time_t begin, end;
	time(&begin); //for EC1
	for (int row =info->startRow; row<=info->endRow;row++){
		for (int col=0;col<ImageStack[0]->width;col++){
			int best=-1;
			for (int k=0;k<ImageStack.size();k++){
				int cases; 
                // if the return gray value bigger than the current, make it for the tmep.
                //finnal gets the best. 
				cases=computeContrast(k,row,col);
				if (cases>temp){
					temp=cases;
					best=k;
				}//if 
			}
			//copy the best piexel into the output image
			int* pixelIn = static_cast <int*>(ImageStack[best]->raster)+row*ImageStack[best]->width+col;
            int* pixelOut = static_cast <int*>(imgOut->raster)+row*ImageStack[best]->width+col;
            *pixelOut=*pixelIn;
			temp=0;// reset the temp.
		}
	}
	time(&end);
	time_t elapsed = end - begin;
	printf("Time measured: %ld seconds.\n", elapsed);
}


int main(int argc, char** argv)
{

	int numThreads;
	int tempthread = stoi(argv[1]);
	if(tempthread>argc-3){
		numThreads= argc-3;
	}else{
		numThreads= tempthread;
	}
    // read image in the image vector.
	for(int i=3;i<argc;i++){
		ImageStack.push_back(readTGA(argv[i]));
	}
	imgOut = new ImageStruct(ImageStack[0]->width, ImageStack[0]->height, ImageStack[0]->type, 1);
	int p = ImageStack[0]->height % numThreads;
	int m = ImageStack[0]->height / numThreads;
	int startRow = 0,endRow = m-1;
	ThreadInfo* info = (ThreadInfo*) calloc(numThreads, sizeof(ThreadInfo));
	for(int k =0; k<numThreads;k++){	
		if(k<p){
			endRow++;
		}
        // init 
		info[k].index = k;
		info[k].startRow = startRow;
		info[k].endRow = endRow;
		info[k].Img = ImageStack[k];
		startRow = endRow + 1; //0,6,11,16
		endRow +=m; // {5,5,5,5}=5+5+5+5
	}
	for(int k =0; k<numThreads;k++){
        //every threads runs the code below.
		pthread_create(&info[k].id,nullptr,threadfunc,info+k);
	}
	for(int k =0; k<numThreads;k++){
		void* useless;
		pthread_join(info[k].id, &useless);
	}

	writeTGA(argv[2],imgOut); // write image. 
	return 0;
}


	
