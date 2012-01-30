Build/Deploy using BlackBerry Native SDK
====================================
Below are the  instructions for building the Quake3 (id tech 3) game engine for BlackBerry QNX based platforms (PlayBook). 
The game engine source code included in this repository was ported from the open-source (under GPL license) directly from id software.
The Quake3 maps/data files are not covered under the open-source agreement and must be provided by the developers wishing
to use the code.

Download the BlackBerry Native SDK 2.0
--------------------------------------
[Download] (https://bdsc.webapps.blackberry.com/native/beta/)

Add your Quake Maps
-------------------
- Copy the .pk3 map file(s) to the resource/baseq3 directory from your Quake3 CD (testing only) or custom maps you have made 

Building from Momentics IDE
---------------------------
- Run BlackBerry Momentics IDE.
- Import the project from the repo directory "Quake3".
- Right click Project -> Build Configurations -> Set Active Congiguration -> Device-Release or Device-Debug.
- Build the project.
- Debug or Run the project.

Build from Makefile
-------------------
- Run cmd.exe and execute the following commands:

  `> cd [bbndk]` -- where [bbndk] is where the QNX NDK was installed (i.e. C:\bbndk-2.0.0)
  
  `> bbndk-env[.bat/.sh]`
  
  `> cd [Quake3]/qnx` -- where [Quake3] is the directory where Quake3 was extracted to
  
  `> make`

- Following this, the relevant binaries should reside in the following subdirectories:
 - [quake3]/qnx/nto/arm/o.le.v7/quake3       (Release)
 - [quake3]/qnx/nto/arm/o.le.v7.g/quake3_g   (Debug)
        
- Run the following to build the Quake3 bar file. Add your additional parameters for signing keys and debug tokens.

  `> blackberry-nativepackager -package Quake3.bar bar-descriptor.xml -e qnx/nto/arm/o.le.v7/quake3 quake3 -e resource/baseq3/pak0.pk3 baseq3/pak0.pk3 icon.png`

