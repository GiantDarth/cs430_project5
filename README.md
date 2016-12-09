# ezview
### CS430 - Project 5 (Image Viewer)
**Christopher Robert Philabaum**

**Northern Arizona University (Fall 2016)**

ezview is a image tool that allows one to load in a P3 or P6 PPM file and perform
various transformations on in it such as shear, scale, translation, scale, and
rotate.

**Note:**
* This program chooses to output the PPM file as a P6 raw binary format.

## Usage
`ezview /path/to/input.ppm`

### parameters:
1. `inputFile`: A valid path, absolute or relative (to *pwd*), to the input ppm file.
Must be P3 or P6 only.

All parameters are *required* and not optional. All parameters must be used in the exact order provided above.

### controls:
1. Reset Image: `Enter` key
1. Exit: `Esc` key
1. Zoom In x2 (Scale): `=` key
1. Zoom Out x2 (Scale): `-` key
1. Pan Up/Down/Left/Right: `Up`, `Down`, `Left`, `Right` arrow keys
1. Rotate Right 90°: `]` key
1. Rotate Left 90°: `[` key
1. Shear Right along _x_-axis: `'` key
1. Shear Left along _x_-axis: `;` key
1. Shear Up along _y_-axis: `/` key
1. Shear Down along _y_-axis: `,` key

## Requirements
1. Visual Studio 2015 (Any Edition)
2. Uses OpenGL ES 2.0 and GLFW, which are provided.

## Compile
In Developer Command Prompt for VS2015, run:
`nmake`: Compiles the program into the current directory as `ezview.exe`

## Grader Notes
* `nmake` compiles `ezview` to the project folder, so in order to run it properly it should be used as `ezview /path/to/input.ppm` where the project folder is the working directory.
