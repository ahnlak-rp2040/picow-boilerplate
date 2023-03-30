# Build Your Project

Projects based on this boilerplate use CMake to handle all the stages of 
compiling your work and creating a `uf2` file suitable for putting on your PicoW.


## Create Your Build Directory

By convention, all your compiling will take place in a 'build' directory; this
is good practice, as it keeps your build artifacts neatly tucked away from your
actual source code (and also why you'll find `build/*` already included in your
.gitignore file):

```
cd awesome-project-1
mkdir build
cd build
```


## Prepare Your Makefiles

The CMake tool needs to generate a lot of configuration and scripts which will
be used to compile your project. This will be based on all the settings in your
`CMakeLists.txt` file:

```
cmake ..
```


# Build Your Project

Once CMake has finished it's work, it's time to build your `uf2` file:

```
make
```

It's as simple as that; all being well, you'll be left with a new file called
`awesome-project-1.uf2` that you can copy onto your PicoW in the usual way:

* hold down the BOOTSEL button on your PicoW
* plug the USB cable in (or, if you have a device with a reset button, use that)

This should cause your PicoW to appear as a drive. This will contain two files
by default - INDEX.HTM and INFO_UF2.TXT - that you can safely ignore. Copy your
`uf2` file to that drive, and your PicoW will reboot and start running your code!
