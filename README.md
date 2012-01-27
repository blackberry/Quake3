Building using BlackBerry NDK 2.0
=================================
- Either copy the .pk3 files from your official Quake3 CD(testing only) or any custom packages (pk3 files) you have made to the resource/baseq3 directory.

Building from Momentics IDE
--------------------------------------
- Run BlackBerry Momentics IDE.
- Import the project from the Quake3 directory.
- If you want to be able to debug, select the project and go to Project -> Build Configurations -> Set Active -> Device-Debug
- Build the project.
- Debug or Run the project.

Build from Makefile
-------------------------------
- Run cmd.exe and execute the following commands:

  `> cd [bbndk]` -- where [bbndk] is where the QNX NDK was installed (i.e. C:\bbndk-2.0.0)

  `> bbndk-env[.bat/.sh]`

  `> cd [Quake3]/qnx` -- where [Quake3] is the directory where Quake3 was extracted to

  `> make`

- Following this, the relevant binaries should reside in the following subdirectories:

 - [quake3]/qnx/nto/arm/o.le.v7/quake3       (ARM release)
 - [quake3]/qnx/nto/arm/o.le.v7.g/quake3_g   (ARM debug)
        
- Run the following to build the Quake3 bar file. Add the additional parameters for signing keys and debug tokens.

  `> blackberry-nativepackager -package Quake3.bar bar-descriptor.xml -e qnx/nto/arm/o.le.v7/quake3 quake3`

  `-e resource/baseq3/pak0.pk3 baseq3/pak0.pk3 icon.png`

