
# Stack-focusing
#### Program Description

- This program is about to implement a multithreaded version of a “stack focusing” application. The input a set of images of the same scene, taken at different focusing distances. The objective is to produce an output image that combines all the input images so that all parts of the output image are in focus. 

#### Introduction to Stack Focusing
- A dataset consists of a series of images of the same scene, taken from the same point and for the same aperture and shutter speed settings, but for different focusing distance settings. If you look at all the images of a same sequence (Figure 1), you can see that, as the focusing distance changes, different parts of the image come into, then out of focus. For any part of the image, there is an image of the stack where that part is “most in focus.” The purpose of our application is to produce an output image that is in “best focus” at every single of its points.




![figure1](https://github.com/YuHaoLao/Stack-focusing/blob/main/README_IMG/figure1.png?raw=true)
- If consider a small rectangular window of our image. Since we have multiple images, what this window defines is really a small volume of pixels, a small rectangular cylinder of pixels that “punches” through our stack of images. we can look a bit closer at our data within this window and see that different parts of the small windows are in focus as the focus distance changes. 
![figure2](https://github.com/YuHaoLao/Stack-focusing/blob/main/README_IMG/figure2.png?raw=true)
#### Focus detection
-  When can we consider that a small region is more “in focus” than another?  the chart of the histogram shows us how frequent each image intensity value is. We observe that the shape of the histogram changes as the image gets blurred. that, as the degree of blur increases, the minimum value observed gets higher, while the maximum value observed gets lower. This makes sense, since a perfectly blurred image would be a uniform rectangle of some shade of gray. This gives us a very simple “measure” of focus: The difference between the max and min values over an image window: The higher the difference, the more in focus the image window is. 

   ![figure2](https://github.com/YuHaoLao/Stack-focusing/blob/main/README_IMG/figure3.png?raw=true) 
   that is, given a area of the image

      ```bash
      Focus= max gray value - min gray value.
      
      ```



## Installation

To install the developer versions of MESA and FreeGlut, execute:

```bash
sudo apt-get install mesa-common-dev
```

```bash
sudo apt-get install freeglut3-dev
```

#### Output 
- An output image name out.tga in the 'Output folder, which combines all the input images so that all parts of the output image are in focus.
#### Compile and Execution 

With the GUI:
```bash
g++ main_v1.cpp gl_frontEnd.cpp imageIO_TGA.cpp rasterImage.cpp -lm -framework OpenGL -framework GLUT -o focus
``` 
without the GUI:
```bash
g++ -std=c++17 main.cpp imageIO_TGA.cpp rasterImage.cpp -o focus 
```
with the GUI and Multithreaded with a single lock:
```bash
 g++ main_v2.cpp gl_frontEnd.cpp imageIO_TGA.cpp rasterImage.cpp -lm -framework OpenGL -framework GLUT -o focus
```
then give the input with the number of threads and the location of the images we want to process:
```bash
./focus 12 ./Output/out.tga ./Data/_MG_6291.tga ./Data/_MG_6293.tga ./Data/_MG_6294.tga ./Data/_MG_6296.tga ./Data/_MG_6297.tga ./Data/_MG_6298.tga ./Data/_MG_6299.tga ./Data/_MG_6300.tga ./Data/_MG_6301.tga ./Data/_MG_6302.tga
``` 
### Preview 
![git](https://github.com/YuHaoLao/Stack-focusing/blob/main/README_IMG/focus.gif?raw=true)





    