# Rasterisation
## Introduction
This rasterisation application displays several rendering techniques, including heightmapping, billboarding and skyboxes.

## Compilation
This application was created using Visual Studio 2022.
See more at: https://visualstudio.microsoft.com/vs/

To generate the solution, execute the premake file.
For example, on Windows type the following command into the terminal in the project directory.

        .\premake5.exe vs2022

This generates a Visual Studio solution file (called `oglShadersRelease.sln`). Open the solution in Visual Studio, and run the program.

![Image](/assets/rasterisation.jpg)


## Controls

### Camera
- `Up` - Move Camera Forward
- `Down` - Move Camera Backward
- `Left` = Move Camera to the Left
- `Right` - Move Camera to the Right

### Rendering
- `Space` - Toggle Wireframe Rendering
- `R` - Reload Shaders
- `T` - Increase Scale Value
- `G` - Decrease Scale Value

NOTE - These settings can also be changed on the application using a UI window

### Light Direction
- `W` and `S` - Rotate Directional Light around X Axis
- `A` and `D` - Rotate Directional Light around Z Axis

### Other
- `Right Click` - Toggle Mouse between camera
- `Escape`- Close Window
