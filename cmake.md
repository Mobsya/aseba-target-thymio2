# Experimental Build with CMAKE

## Setup
* Clone the repository. if you have an old version of git, you may need to pass `--recursive` to git.
* Download Microchip XC16 compiler from http://microchipdeveloper.com/xc16:installation
* Download the Peripheral Libraries (PLIBS) for "PIC24/dsPIC® DSCs" from http://www.microchip.com/mplab/compilers#pic24
and install them in the same folder
* Make sure the directory of the xc16-gcc compiler in in the PATH
* make sure you have a recent version of CMake - 3.5 or better.
* You will also have the master branch of aseba cloned

## Build
* Create a build directory `mkdir build && cd build`
* invoke cmake : `cmake -D<path/to/aseba>  ..`
* build: `make`

## Ninja
I recommend you use `ninja` instead of make.
The mast 2 steps then become

* invoke cmake: `cmake -D<path/to/aseba>  -GNinja ..`
* build: `ninja`


