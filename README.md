# BaseRenderer
This is a software renderer that is a redo of a very old codebase that I made a long time ago.
This renderer only supports Windows but only needs something that can use MSVC to run (No SDL or any other kind of library).
This project uses it's own in-house text mesh format, which is more readable than .obj files, and 
more importantly, supports triangles having a solid color without UV (Implementing it is fairly easy, however).
The only lights supported are point lights, directional lights, and ambient lights (It doesn't take into account obstructions, however).
Matrix Multiplication and the scanline drawing are accelerated with SIMD (Partial AVX, the rest is SSE, my computer is bad).

Demo of this software is here:

[![Link to YouTube Video](https://img.youtube.com/vi/lhK7VMfnZK8/maxresdefault.jpg)](https://youtu.be/lhK7VMfnZK8)
