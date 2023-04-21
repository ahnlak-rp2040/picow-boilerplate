# HTTP(S) Client

Optionally included in the Boilerplate is a simple, lightweight HTTP(S) client,
written in pure C. 

It's built to minimise the work required by your program; most of the processing
is done via callbacks but WiFi requires more active handling so regular calls
into our `check()` function will be required in common with other libraries in
the Boilerplate.


## Usage

The configuration file handler is used by the main Boilerplate code; it consists
of a single C file (plus header), so it's simply a matter of

* adding `opt/httpclient.c` to the list of source files in your `CMakeLists.txt`
* including `opt/httpclient.h` in your main source file
* calling `httpclient_open()` to initiate an HTTP request
* calling `httpclient_check()` to check the progress and do any further processing
* calling `httpclient_close()` afterwards, to clean up any allocated resources

(alternatively, you can copy `httpclient.c` and `httpclient.h` to your top
level source directory and use them from there)


## Limitations

Data is stored in memory, which is at something of a premium on the PicoW.
It is _strongly_ recommended, therefore, that API endpoints are used whenever
possible because full web pages are potentially significantly larger than
the entire memory of the device.

For now, only IPv4 is supported.


## Functions


### `void httpclient_set_credentials( const char *, const char * )`

If the PicoW's WiFi is not already connected when a new HTTP request is made,
`httpclient` will attempt to set it up - this function allows you to set your
WiFi credentials once, rather than needing to include them with every `open`
call.

If you have set up / manage your WiFi connection elsewhere in your program,
there is no need to call this function.


### `httpclient_request_t *httpclient_open( const char *, char *, uint16_t )`

Initiates an HTTP request to load the provided URL. A buffer can be provided
to store the resulting data; if this is NULL then a buffer will be allocated.

The URL can include a port number, and HTTPS connections are supported.

The final parameter either defines the size of the provided buffer, or (if no
buffer is provided) the maximum buffer that will be allocated to hold the result.

_Note: if more data is received than fits into this buffer, the data will be
truncated and the status set to HTTPCLIENT_TRUNCATED_


### `httpclient_status_t httpclient_check( const httpclient_request_t * )`

Checks the state of the specified HTTP request, and performs any additional
processing required. 

Returns the current status of the request. This will be changed to reflect the
progress of the request; once it reached `HTTPCLIENT_COMPLETE` (or, if too much
data is received, `HTTPCLIENT_TRUNCATED`), the request has been finished and you
can safely retrieve the response buffer.


### `const char *httpclient_get_response( const httpclient_request_t * )`

Returns a pointer to the buffer (either allocated, or provided to the initial
`httpclient_open()` call), if the request has been completed. NULL will be
returned if the request is still in progress.


### `void httpclient_close( httpclient_request_t * )`

Closes the HTTP request, and frees up any allocated resources.
