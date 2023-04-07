# PicoW C/C++ Boilerplate Project

This is a template repository, inspired by and based on Pimoroni's 
[Pico C++ Boilerplate Project](https://github.com/pimoroni/pico-boilerplate). 
It is specifically designed to help leverage the additional features of the
PicoW, but obviously parts of it that do not require the WiFi / Bluetooth
functionality will be transferable to a regular Pico.

These pages attempt to document everything you'll need to know to get developing;
it does *not* attempt to teach you C/C++; there are way better books available to
do that.

The primary assumption is also that you're working on a Linux box, or Raspian,
or at least WSL under Windows. Although in theory it's possible to build 
'natively' under Windows, I no longer really have a Windows desktop to test it
on.


## The Overall Process

* [Setting Up Your Environment](setting-up-env.md) explains what you need to have
  installed and configured to be able to build projects based on this template.
* [Creating Your Project](creating-projects.md) walks you through setting up a
  new repository based on this template, and preparing to build.
* [Building Your Project](building-projects.md) explains how to actually run
  the build process, and gives your your `uf2` file.


## Boilerplate Features

* Pre-built Github Actions to automatically compile your project with every 
  check-in, and generate ZIP package files with every release.
* [USB Mass Storage Device](usbfs.md) presents a filesystem to the host computer,
  along with functions to allow your project to read and write files to that
  in-flash filesystem. This is done while preserving the usual `stdio` USB
  functionality of the Raspberry Pi Pico C/C++ SDK.


### Optional Extras 

These extras can be found in the `opt/` directory; they are each single-file
single-purpose packages that you use if you need them, and ignore them if you
don't!

* [Configuration File Handling](config.md) is a drop-in example of how you can
  leverage the USB Mass Storage Device filesystem to easily offer users a way
  to edit a local configuration file.
* [HTTP(S) Client](httpclient.md) offers a simple way of fetching requests from
  websites, as cleanly as possible.

## Further Examples

* tba


Share and Enjoy!