# PicoW C++ Boilerplate Project

This is a template repository, inspired by and based on Pimoroni's 
[Pico C++ Boilerplate Project](https://github.com/pimoroni/pico-boilerplate). 
It is specifically designed to help leverage the additional features of the
PicoW, but obviously parts of it that do not require the WiFi / Bluetooth
functionality will be transferable to a regular Pico.

It is intended as a useful starting point for working with PicoW-based projects
using the Raspberry Pi Pico C/C++ SDK; it also bakes in support for the Pimoroni
Pico Libraries, although it does not depend on these at all.


## Features

* ready to build project framework, including Github Actions for builds and releases
* support for a USB Mass Storage mode, to make it easy to provide a configuration
  file to your PicoW project (for example, providing WiFi settings) without the
  need to recompile.


## Documentation

I've attempted to keep everything as comprehensively documented as possible;
this can be found in the `/docs` folder of this repository, and on
[PicoW C++ Boilerplate GitHub Pages](https://ahnlak.github.io/picow-boilerplate).


## Further Examples

These small projects are all based on this Boilerplate, and demonstrate something
specific in a hopefully well-explained way.

* tba


## License

This template is nominally released under the BSD 3-Clause License, to match that
used in the SDK and to be as permissive as possible.


Share and Enjoy