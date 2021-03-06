# TI-84-CE-DooM
##### A version of DooM for the TI-84 CE, written in C.

![alt text](https://raw.githubusercontent.com/andymwat/TI-84-CE-DooM/master/gameplay.gif "A preview of some gameplay.")

### Requires the ['Standard' CE Libraries](https://github.com/CE-Programming/libraries/releases).

### About
A "port" of the classic game DooM to the TI-84 CE, written in pure C. Use the numberpad to move and rotate the camera, and 2nd to quit the game. Up and down change the FOV, while left and right change the resolution. Plus and minus change the wall height. The code is quite similar to my C# raycaster with a few changes, but renders at a fraction of the resolution. The only things implemented right now are rendering, movement, and simple collisions. The gun sprite used is from The Terminator: Rampage (I think), and was found [here](https://www.realm667.com/index.php/en/armory-mainmenu-157/947-ak47#credits). The enemy sprite is from DOOM itself.

### Todo
* Fix holes that appear randomly on the back side of walls.
* Implement AI/gameplay.
* Implement map loading from AppVars
* Implement saving and loading from file.
* Interpolate between rays to smooth out render.
* Make a map editor.
* Possibly add sound if a suitable library for sound over the USB port exists.
* Optimize.
* Refactor, comment.
### Compiling
Building requires the [CE C Software Development Kit](https://github.com/CE-Programming/toolchain/releases). To compile, cd to the directory and run "make" from the command line. Assuming that all of the headers are in the right place and also assuming the SDK has been installed correctly, you should see:
``` 
C CE SDK Version 8.3

Input File: DOOM.bin
--------------------
Output File: [DOOM] DOOM.8xp
Archived: No
Size: 13302 bytes
--------------------
Success!

```
Copy the resulting DOOM.8xp file to the calculator (or emulator, CEmu works well for testing), and run it with Asm(prgmDOOM).
