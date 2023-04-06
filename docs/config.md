# Configuration Files

Optionally included in the Boilerplate is a simple, lightweight configuration
file handler, written in pure C. 

It's deliberately simple in design, maintaining a simple array of `name`/`value`
string pairs that can be read from, and written to, a file on the local filesystem.


## Usage

The configuration file handler is used by the main Boilerplate code; it consists
of a single C file (plus header), so it's simply a matter of

* adding `opt/config.c` to the list of source files in your `CMakeLists.txt`
* including `opt/config.h` in your main source file
* calling `config_load()` in your programs startup
* calling `config_check()` in your programs main loop

(alternatively, you can copy `config.c` and `config.h` to your top level source
directory and use them from there)


## Functions

### `void config_load( const char *, const config_t *, uint16_t )`

Called to load the named configuration file, usually during your program's 
startup. Can be passed an array of default configuration values, but if not
required this can be NULL.

This array must end with an entry with an empty name, so that the library can
determine that the array has ended:

```
  config_t default_config[] = 
  {
    { "WIFI_SSID", "mynetwork" },
    { "WIFI_PASSWORD", "secret123" },
    { "", "" }
  };
```

The final parameter determines how often, in seconds, the configuration file
will be checked to see if any changes have occured.

These details will be saved in the library, and used by subsequent calls. Only
one configuration file can be handled at time; if you wish to change the file
being used simply call this function with a new filename.


### `bool config_save( void )`

Called to save the current configuration state to the configuration file. This
will include all the defaults, so it can be useful to call `config_save()`
immediately after a call to `config_load()`, as this will populate a file with
default values for the user to view / edit, if no file previously existed.


### `bool config_check( void )`

This function checks to see if the configuration file has been modified since
it was last read, and if so will reload any settings within it. 

It would usually be called as part of your program's main loop - it will only 
check at the frequency defined in the original call to `config_load()`, so can
be safely called more often.

A boolean return value will tell you if the configuration has been reloaded -
`true` means it has, whereas `false` indicated no changes have occured.

_Note: this function will *not* delete configuration keys that have been removed
from the file_


### `const char *config_get( const char * )`

Looks up the named configuration key, returning a pointer to the value. If the
key is not defined in the configuration, a NULL is returned.


### `void config_set( const char *, const char * )`

Updates the configuration to set the provided key to the provided value. This
just updates the internal configuration table; to persist these changes to the
filesystem you will need to call `config_save()`

