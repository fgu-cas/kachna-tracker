Kachna Tracker
==============

Tracker for AAPA tasks

## Hardware

* PCI-DIO24 by *Measurement Computing* 
  * For dealing out shocks
* Sony XCDX710CR
  * Firewire camera for arena monitoring

## Build instructions

Prerequisites:  
* OpenCV (tested with 2.4.8-1)
* Qt 5 (qmake etc)
* (Universal Library for interfacing with the DIO24 card for shocks)
  * Should be on the CD bundled with the DIO24

### Linux
(Doesn't support the DIO24)

    cd kachna-tracker
    qmake . LIBS+="`pkg-config opencv --libs`"
    make

### Windows (from Linux)

1. Install MXE
  1. (to add later)
1. Place the .h and .lib C files from the Universal Library in the same directory as the source for Kachna Tracker

(Assuming $MXE contains the path to your MXE install)

    cd kachna-tracker
    $MXE/usr/i686-pc-mingw32.static/qt5/bin/qmake . LIBS+="`$MXE/usr/bin/i686-pc-mingw32.static-pkg-config opencv --libs`" LIBS+="-L . -lcbw32 -lstdc++"
    PATH=$MXE/usr/bin:$PATH make
