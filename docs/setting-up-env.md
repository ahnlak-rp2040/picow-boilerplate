# Setting Up Your Environment

As mentioned before, these instructions are generally assuming you are working
on the command line in a Linux-like environment (either Linux, a Raspberry Pi
or at least WSL).


## Before you start

It's easier if you make a `pico` directory or similar in which you keep the SDK,
optional Pimoroni Libraries and your projects alongside each other. This makes
it easier to include libraries.


## Install required packages

This should install everything you need to get going:

```bash
sudo apt update
sudo apt install build-essential gcc-arm-none-eabi cmake git python3
```

* `build-essential` contains all the 'normal' C/C++ development tools (compiler et al)
* `gcc-arm-none-eabi` is the cross-compiler that allows you to build code which
  will run on the processor on the PicoW
* `cmake` is the tool that you will use to run the build process
* `git` will allow you to fetch all the repositories you'll require, and manage
  your own work too
* `python3` is needed for some of the build operations with the Pico SDK


# Fetch the Pico SDK

You will need to have the Pico SDK. Note that it's important to run the 
`submodule update` command, to ensure that you have all the third-party libraries
included in the SDK - this includes the libraries we'll use to talk to WiFi
and Bluetooth:

```
git clone https://github.com/raspberrypi/pico-sdk
cd pico-sdk
git submodule update --init
export PICO_SDK_PATH=`pwd`
cd ../
```

The `PICO_SDK_PATH` set above will only last the duration of your session. You
should should ensure your `PICO_SDK_PATH` environment variable is set by `~/.profile`:

```
export PICO_SDK_PATH="/path/to/pico-sdk"
```

## Grab the Pimoroni libraries (optional)

```
git clone https://github.com/pimoroni/pimoroni-pico
```

Although not necessary for general PicoW development, if you're using any of
Pimoroni's astounding range of breakouts, boards and displays (and if not, why
not?!) this will make their drivers easily available to you.
