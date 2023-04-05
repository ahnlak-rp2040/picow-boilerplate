# USBFS

This is a library for providing an easy-to-use USB mounted drive to a Pico
based application. The 'drive' is accessible to your project via `fopen`-like
functions, as well as from the host machine.

It is designed to provide a relatively easy way to provide configuration 
details to an application (such as WiFi details) that would otherwise require
recompilation and redeployment.

It also preserves the normal stdio USB interface provides by the Raspberry Pi
Pico C/C++ SDK.

The library is written in pure C, so can be safely used with C or C++ based
projects.

> *Note:* Due to the limitations of FAT (related to sector size and count),
> this library requires the use of the top 512kb of flash memory. This is
> probably worth keeping in mind if you have a large application.


## Usage

The library is baked into the [Picow C/C++ Boilerplate Project](https://github.com/ahnlak-rp2040/picow-boilerplate),
however it's possible to simply copy the entire `usbfs` directory into your
own project, and include it in your `CMakeList.txt` file, and then simply
`#include "usbfs.h"`:

```
add_subdirectory(usbfs)
target_link_libraries(${NAME} 
    usbfs
)
```

(where `${NAME}` is the name of your project).


## Filename Limitations

Long filenames are NOT supported, which means all filenames must conform to
the ancient `8.3` DOS naming rules.


## Utility Functions

These functions are primarily focused on handling the USB connection to the 
host machine.


### `void usbfs_init( void )`

Must be called before any other `usbfs_` function, usually during your program's
startup. 

It initialised the TinyUSB library and the USB connection, as well as mounting
(and, if required, creating) the filesystem stored in the Pico's flash memory.


### `void usbfs_update( void )`

Polls the TinyUSB library, to perform any processing required to maintain the 
USB connection. As such, this must be run regularly within your main loop. 

If you use the `usbfs_sleep_ms()` function to manage your timings, it shouldn't
be necessary to call this function directly.


### `void usbfs_sleep_ms( uint32_t )`

A drop-in replacement for the regular `sleep_ms()` API call. This will sleep
for the required number of milliseconds, but will repeatedly call `usbfs_update()`
for you to maintain the USB connection correctly.

This is the prefered method of managing USB updates.


## File Functions

These functions provide access to the filesystem created for you in the Pico's
flash memory.

Their parameters and returns are designed to be as close as possible to their
standard C equivalents (`fopen()`, `fgets()` et al), to make the transition
as simple as possible.


### `usbfs_file_t *usbfs_open( const char *pathname, const char *mode )`

Opens the named file, with the appropriate permissions. Only a subset of the
standard permissions are supported:

|mode|permission|
|----|----------|
|`r`|Read only, file must exist.|
|`r+`|Read and write, file must exist.|
|`w`|Write only, file trunctated if if exists, created otherwise.|
|`w+`|Read and write, file trunctated if if exists, created otherwise.|
|`a`|Write only at the end of the file, file created if it does not exist.|
|`a+`|Read and writeat the end of the file, file created if it does not exist.|

Any other mode, or if the conditions are not otherwise met, will result in
a NULL pointer being returned. 

Otherwise, a `usbfs_file_t` structure will be allocated and a pointer returned
to it - this can then be passed to the other file functions withing USBFS.


### `bool usbfs_close( usbfs_file_t *filepointer )`

Closes the file specified by the `usbfs_file_t` pointer. Once closed, the
allocated `usbfs_file_t` structure will be freed, and must not be further used.


### `size_t usbfs_read( void *buffer, size_t size, usbfs_file_t *filepointer )`

Reads at most the specified number of bytes from the file, and puts it into
the buffer provided. The buffer *must* have enough space allocated to it to be
able to accomodate all this data.

The actual number of bytes read is returned; this may be less than requested,
if the end of the file is encountered.

In case of error, zero is returned.


### `size_t usbfs_write( const void *buffer, size_t size, usbfs_file_t *filepointer )`

Writes the specified number of bytes from the provided buffer to the file. The
buffer *must* contain at least as much data as specified.

The actual number of bytes written is returned; this may be less than requested,
if the filesystem is full.

In case of error, zero is returned.


### `char *usbfs_gets( char *buffer, size_t size, usbfs_file_t *filepointer )`

Reads a line of text from the file, up to a maximum of the size provided. The
line is either terminated by a newline, or by the end of the file.

If neither a newline or the end of the file has been encounted by the time that
the maximum number of bytes has been read, the data read so far is returned. The
buffer *must* have enough space alocated to it to be able to accomodate all
this data.

The provided buffer is returned on sucess; in case of error a NULL pointer is
returned.


### `size_t usbfs_puts( const char *buffer, usbfs_file_t *filepointer )`

Writes the provided string to the file; a newline is *not* added to this string,
so if you want a newline-terminated string you must provide it.

The actual number of bytes written is returned; in case of error this value
will be negative.


### `uint32_t usbfs_timestamp( const char *pathname )`

Returns the modification timestamp of the named file. This timestamp is encoded
in a 32 bit value thusly:

|bits|meaning|
|----|-------|
|25-31|year, minus 1980|
|21-24|month|
|16-20|day|
|11-15|hours|
|5-10|minutes|
|0-4|seconds, divided by 2|

However, in normal usage it is sufficient to compare two timestamps to identify
when a file has been modified and may need to be re-read.
