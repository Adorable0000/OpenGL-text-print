## Description 
 
This is an example of simple way to render text with OpenGL, made with Qt
 
## Requirements 

C++11

Qt, version higher than 5

Hardware support for OpenGL version 3.3 or higher

## Usage 

Check if you have supported OpenGL version

Linux

```
glxinfo | grep OpenGL
```

If you don't have it installed, install OpenGL dev files and tools

```
sudo apt-get install libgl1-mesa-dev mesa-utils 
```

On Windows you can use viewer, such as 
<https://www.realtech-vr.com/home/?page_id=142> 

When you use qmake, you have to link OpenGL manually, for example

```QMake
QT += opengl
CONFIG += opengl

win32: LIBS += -lopengl32
```

If you use CMake, you can find an example CMakeLists.txt file in the folder.

If you want to use any other OpenGL library, you can find some information 
about using FreeType in FreeType use.md file

## Get started

If you want to learn OpenGL basics, check out  <https://learnopengl.com/>
This simple project was inspired by that lessons.
