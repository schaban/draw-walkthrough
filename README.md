[crosscore](https://github.com/schaban/crosscore_dev) library comes with a set of example modules that implement scene management and rendering.

How actual rendering of scene parts works is abstracted away by the use of generic draw-interface declared in [draw.hpp](https://github.com/schaban/crosscore_dev/blob/main/src/draw.hpp).
That is, while there is a default OpenGL implementation in [draw_ogl.cpp](https://github.com/schaban/crosscore_dev/blob/main/src/draw_ogl.cpp), it is possible to add an alternative one, that will use different techniques or will be based on another low-level graphics API such as Vulkan.

The purpose of this project is to walk through a very basic OpenGL implementation of the draw-interface, providing along the way a general idea of the low-level graphics programming details involved in a task like this.
As such this can serve as a sort of tutorial for OpenGL programming, however a number of higher-level conceptual details like the workings of the graphics pipeline or mathematics of transformations will be only mentioned in passing, if at all.
