# Senior Design Project: Virtual Fitting Room
Written by Anthony Mansur.
This is the second portion of our two-part pipeline (simulation and rendering).

Senior Design Poster: https://drive.google.com/file/d/1x3a35UVORNeP4AWRU7Qqcjrqk10oundO/view

Senior Design Demo: https://www.youtube.com/watch?v=SEVx0VuM56E

## Description
The goal of this project for the first half of the semester is to render yarn with fiber-level detail for added realism. What has been accomplished so far is a base render of a straight piece of yarn given most of the fiber parameters provided. A GUI has been added to adjust these parameters, change the type of fiber, and see the core fiber texture. 

https://drive.google.com/file/d/14w5Hl_V-kXffvdaFFo7Uyu8Fj25Ko6JV/view?usp=sharing


### Pipeline
The current pipeline to render fiber-level yarn is as follows:
1. Get the control points of each strand of yarn (currently a straight line along the x-axis for this implementation).
2. Send to the GPU the fiber parameters associated with the yarn of choice (cotton, polyester, rayon, silk, etc.)
3. Render one pass of the algorithm to generate core fiber and store the height map, 2D surface normal, and alpha channel as a texture.
4. Render second pass of the algorithm with the texture to create the final result.

The algorithm itself consists of several stages
1. Send the control points to the tessellation shaders to create a bezier curve. This is the yarn center.
2. From the yarn center, create the ply curves that loop around the yarn center. This is the ply center.
3. From the ply center, create the fibers that loop around the ply center. This is the fiber center.
4. Pass the fiber center (plus it's adjacent neighbors) to the geometry shader.
5. From the geometry shader, create camera-facing triangle strips to represent the fibers. Make the core Fiber significantly thicker and pass in its UV coordinates
6. From the fragment shader, if rendering the core fiber portion onto the yarn, extract the texture information for that fragment. 
7. Perform lambertian shading for the final render.

## Beta Review
- Edit the entire algorithm in order to support 3D rendering
	- Add camera and basic lighting
	- Restructure codebase
	- Updating the core algorithm with proper parametrization
	- Update the camera-facing triangle strip algorithm 
- Create off-screen rendering of the core fiber and pass to the main render
	- Map uv coordinates to the core fiber strand
	- Create framebuffer to store render to textures (height map, 2D surface normal, alpha channel)
	- apply this texture to the main render
-Allow for editing of parameters based on type of yarn (polyester, silk, etc.) and switching between these types
	- Creation of GUI
	- Updating Fiber class to support these edits
	- Saving the parameters of each type of yarn

## Alpha Review
- Setup a coding environment in Visual Studio using OpenGL and shaders
- Passed yarn parameters (for cotton) into shaders and two control points representing endpoint of the yarn
- Built tessellation shader pipeline to generate the following elements, all in one pass: [10]
- Bezier curve representing the yarn center, subdivided into 64 subsections given two control points
- Given the yarn center, calculate the (n=3) ply centers that loop around yarn center as helices
- Given the ply center, generate (m=floor(64/n)=21) isolines that loop around the ply center as helices
- Modified isolines to add loop and hair fibers which are accidental fibers strands giving noise to the yarn for realism
-Built geometry shader pipeline that takes in subsegments of the isolines, as well as their adjacent neighbors, and generated triangle strips giving the fibers a certain width
- Updated pipelines to identify the core fiber which will have a pregenerated texture [11]
- Started creating requirements for textures


## Acknowledgements
This part of our senior design project an implementation attempt at this research paper: http://people.csail.mit.edu/kuiwu/rtfr.html#rtfr which is directly related to https://www.cs.cornell.edu/~kb/publications/SIG16ProceduralYarn.pdf (also found here: https://www.cs.cornell.edu/projects/ctcloth/). 
In order to create this solution, some code from outside sources were either used or looked at. Links are attached in the relevant portion of the code. These two links were used heavily for implementation details: (https://learnopengl.com/ and http://www.opengl-tutorial.org/). 
Special thanks to Enoch Tsang for tutorial on tessellation shaders (https://enochtsang.com/articles/tessellation-shaders-isolines). 

## How to run 
This code could be downloaded at https://github.com/anthonymansur/yarn-rendering. Please contact amansur@seas.upenn.edu for repository access.

To run this code, please have Visual Studio downloaded on your machine. Next, set the include path to a directory that has glad, GLFW, glm, and glew installed. Set the library path to a directory with glfw3.lib installed. In the linker setting, add the follow two lines (glfw3.lib and opengl32.lib). Finally, add glad.c into the source directory. 
