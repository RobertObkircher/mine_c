mine_c
======

A simple Minecraft clone using C and OpenGL/GLFW.

To simplify things only Linux (inotify) and OpenGL 4.6 are supported for now.

Goals
------

Project:
- Render a block world
- Terrain generation
- Place and remove blocks
- Maybe support for Windows and older OpenGL versions

Personal:
- Learn C
- Better understanding of OpenGL


General Structure
-----------------

Write documentation...


Changelog
---------

- Decided to use libraries: glfw, glad, log.c, mathc

- Render first triangle

- Use inotify to reload files

- Hotreload shaders

- Functions to allocate memory or exit (allocorexit)


Motivation
----------

[Jonathan Blow's Videos](https://www.youtube.com/user/jblow888) about the Jai language made me realise that I've been focusing too much on functional languages, so I've decided to learn C.

I've already used C for small exercises in school but this is my first real project.

My previous experience with 3d graphics is limited to a Java (jMonkeyEngine) Minecraft clone.
I've also created a "game-engine" using Kotlin and OpenGL/GLFW which was just a convoluted way of rendering the same block world.

