# Optional Libraries

The files here are all single-file, single purpose lightweight libraries that
can be used if useful, but ignored if not.

They're implemented in this way to make it as easy as possible to drop into
your own projects; it's hard to justify fully inflated libraries for relatively
small tasks.

* `config.c/.h` provides basic handling for configuration files stored on the
  internal filesystem provided by USBFS.
* `httpclient.c/.h` provides a simple mechanism for retrieving files from a
  web server.
  