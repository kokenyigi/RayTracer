# RayTracer
This Application is a demonstration of a self-built Ray-Tracer framework.
 The App lets the user explore the spectacular reflections, refractions, 
 shadows and glints that the Raytracing technology provides for graphical programming, 
 doing all of this in Real time, with good FPS results on mid-high end machines.

## Guide To Building & Running
The App requires Cmake & MSVC toolchain present for building and compilation. After cloning the repository, use the following cmd script guide: <br><br>
First, create a build directory:<br>
```mkdir build```<br>
```cd build```<br>
<br>
Then, run this script in order to generate the necessary build files: <br>
```cmake .. -G "Visual Studio 17 2022" -A Win32```<br>
<br>
After this, in order to fully compile the application use the following: <br>
```cmake --build .```<br>
<br>
Now, there should be an .exe executable file inside the working directory, running it will result in the application launching.

## Guide to Controls
With the basic WASD, SPACE, LSHIFT key combinations one can freely move around in all 6 directions using the built-in free camera. With CTRL also pressed, movement is accelerated. <br>
By pressing the key "F" the user can enable/disable the mouse cursor, this is required if the user wants to interact with the GUI present in the app. <br>
By pressing LCTRL and clicking with the left mouse button, an object can be selected, and from that point on the given object can be freely moved, rotated and rescaled. <br>
Optionally, pressing T toggles between OpenGL's rasterized graphical pipeline rendering, and the base Ray-Traced Rendering. While using the OpenGl mode, pressing E or Q will display the BVH structures of the given demo entities.
