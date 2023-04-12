ARK
==============

ARK is a rendering kit for rapid OpenGL(ES) and Vulkan game development.

Focusing on simplicity, the ARK core library provides few functionality.
Some plugins listed below are probably needed if you are tending to start a rapid and feature-rich game development.

Licensed under the Apache 2.0 license.


System requirements
--------------
1. CMake 3.15+
2. Vcpkg
3. Python 3.6+
4. MSVC 2017+
5. GCC 4.7+
6. Clang 3.3+
7. Ninja (Optional)


Building
--------------
Things would be much easier if you've already been familiar with CMake building system.


> cmake -G "Unix Makefiles" path_to_src
>
> make

MSVC
> cmake -G "NMake Makefiles" path_to_src
>
> nmake

Android
> cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL="android-18" path_to_src
>
> ninja


Dependencies
--------------
This project uses Vcpkg to maintain its dependencies. Please visit their [GitHub page](https://github.com/microsoft/vcpkg) for more informations.

* [freetype](http://www.freetype.org)
* [glbinding](http://www.glbinding.org) (Desktop applications only)
* [glm](http://glm.g-truc.net)
* [libpng](http://www.libpng.org/pub/png/libpng.html)
* [libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo.git) (Optional WIN32 only)
* [libzip](https://nih.at/libzip/)
* [SDL2](https://www.libsdl.org) (Desktop applications only)
* [stb](https://github.com/nothings/stb) (Optional)
* [tinyxml2](http://www.grinninglizard.com/tinyxml2/)
* [zlib](http://zlib.net/)


Plugins
--------------
* assimp
* box2d
* bullet
* dear imgui
* PBR
* noise
* optick
* python
* vorbis
* yoga layout
