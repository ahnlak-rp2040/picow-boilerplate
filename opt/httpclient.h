/*
 * opt/httpclient.h - part of the PicoW C/C++ Boilerplate Project
 *
 * Header for an optional, simple HTTP Client, supporting HTTPS too.
 * This is provided as part of the boilerplate, but if you don't require it
 * (or have built your own) you can safely delete this file (and httpclient.c)
 * and remove it from your CMakeLists.txt file.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * This file is released under the BSD 3-Clause License; see LICENSE for details.
 */

#pragma once


/* Constants. */

#define PCBP_HTTP_SSID_MAXLEN       32
#define PCBP_HTTP_PASSWORD_MAXLEN   64
#define PCBP_HTTP_WIFI_RETRY_MS     5000
#define PCBP_HTTP_HOST_MAXLEN       63
#define PCBP_HTTP_PATH_MAXLEN       127


/* Enumerations. */

typedef enum
{
  HTTPCLIENT_NONE,
  HTTPCLIENT_WIFI_INIT,
  HTTPCLIENT_WIFI,
  HTTPCLIENT_DNS,
  HTTPCLIENT_HEADERS,
  HTTPCLIENT_DATA,
  HTTPCLIENT_TRUNCATED,
  HTTPCLIENT_COMPLETE
} httpclient_status_t;


/* Structures */

typedef struct
{
  char                  host[PCBP_HTTP_HOST_MAXLEN+1];
  char                  path[PCBP_HTTP_PATH_MAXLEN+1];
  uint16_t              port;
  bool                  tls;
  httpclient_status_t   status;
  char                 *response;
  uint16_t              response_size;
  absolute_time_t       wifi_retry_time;

} httpclient_request_t;


/* Function prototypes. */

#ifdef __cplusplus
extern "C" {
#endif

void                  httpclient_set_credentials( const char *, const char * );
httpclient_request_t *httpclient_open( const char *, char *, uint16_t );
httpclient_status_t   httpclient_check( httpclient_request_t * );
const char           *httpclient_get_response( const httpclient_request_t * );
void                  httpclient_close( httpclient_request_t * );

#ifdef __cplusplus
}
#endif


/* End of file opt/httpclient.h */
