# GLES2 Starter Kit for WIN32

[OpenGL for Embedded Systems](http://www.khronos.org/opengles/)
(OpenGL ES) is a rendering API for 2D and 3D graphics.  The API is
cross-language and multiplatform.  The OpenGL ES2 (GLES2)
specification is available for Apple iOS 5+ (e.g., iPad, iPhone, and
iPod Touch), Android 2.0+, the Raspberry Pi, WebGL, Windows XP -
Windows 10 (via ANGLE), and Mac OS 10.9+ (via OpenGL 4.1 which is 
API compatibility with OpenGL ES 2).

OpenGL ES 2 is generally very pleasant to program with but it only
provides a graphics API.  Working with windows and any kind of user
input isn't part of the API.  For Windows, OS X, and Unix-like systems
[GLFW](http://www.glfw.org/) can provide an abstraction layer.

## What's this then?

This is a starter project that puts some of the pieces together.

There is no standard OpenGL ES2 development kit for Windows but one
of the best implementation is Google's
[ANGLE](http://angleproject.org/) which provides an OpenGL ES2 API
that translates ES2 calls to hardware accelerated Direct3D calls.
You can see this technology in action within Google Chrome's WebGL
support.  Unfortunately Google doesn't provide precompiled binaries
for GLESv2.

But this starter kit does!

## What's included?

This package includes:

* ANGLE compiled with Visual Studio 2015 (10/31/2016)
    * libEGL.dll
    * libEGL.lib
    * libGLESv2.dll
    * libGLESv2.lib
    * EGL and GLES header files
* GLFW 3.2.1 compiled with Visual Studio 2015 (08/18/2016)
* d3dcompiler_47.dll for support on older Windows systems
* An example with an NMake compatible Makefile.

ANGLE has been compiled with Visual Studio 2015 using the 32-bit target
using [these directions](https://code.google.com/p/angleproject/wiki/DevSetup).

All files are copyright their respective authors and licensed respectively 
under the New BSD License (ANGLE), zlib/libpng (GLFW), royalty free distribution
(Khronos headers), and the DirectX SDK EULA (DirectX).

Royalty free redistribution of d3dcompiler_47.dll is allowed per 
https://blogs.msdn.microsoft.com/chuckw/2012/05/07/hlsl-fxc-and-d3dcompile/

## How do I use this kit?

You might start by compiling the demo.

1. Launch the "Visual Studio Command Prompt"
2. CD to this directory
3. Type "nmake" to compile the demo
4. Type "demo" to test the demo

You can also build your own Visual C++ solution - just make sure to
set the include and library paths such that they include this
directory.

## Where do I get it?

[GLES2StarterKit-2.0.zip]()
