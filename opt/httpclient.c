/*
 * opt/httpclient.c - part of the PicoW C/C++ Boilerplate Project
 *
 * An optional, simple HTTP Client, supporting HTTPS too.
 * This is provided as part of the boilerplate, but if you don't require it
 * (or have built your own) you can safely delete this file (and httpclient.c)
 * and remove it from your CMakeLists.txt file.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * This file is released under the BSD 3-Clause License; see LICENSE for details.
 */

/* Standard header files. */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* SDK header files. */

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"


/* Local header files. */

#include "httpclient.h"


/* Module variables. */

static char         m_wifi_ssid[PCBP_HTTP_SSID_MAXLEN+1];
static char         m_wifi_password[PCBP_HTTP_PASSWORD_MAXLEN+1];


/* Functions. */

/* Internal functions - used only in this file. */


/*
 * start_request - called when the network is available, in order to intiate
 *                 the communications to the web server.
 */

static void httpclient_start_request( httpclient_request_t *p_request )
{
  printf( "sending request!\n" );
  /* All done. */
  return;
}


/* Public functions. */

/*
 * set_credentials - save WiFi credentials, in case we need to bring the WiFi
 *                   connection up to service a request.
 */

void httpclient_set_credentials( const char *p_ssid, const char *p_password )
{
  /* All we do here is to save these details in case we need them. */
  strncpy( m_wifi_ssid, p_ssid, PCBP_HTTP_SSID_MAXLEN );
  m_wifi_ssid[PCBP_HTTP_SSID_MAXLEN] = '\0';
  strncpy( m_wifi_password, p_password, PCBP_HTTP_PASSWORD_MAXLEN );
  m_wifi_password[PCBP_HTTP_PASSWORD_MAXLEN] = '\0';

  /* All done. */
  return;
}


/*
 * open - initiates a new HTTP request; if the WiFi is not available, it will
 *        be brought up with credentials if available. 
 *        A new request structure will be allocated, and a pointer to this is
 *        returned. On error, no allocation will be kept and NULL returned.
 */

httpclient_request_t *httpclient_open( const char *p_url, 
                                       char *p_buffer, uint16_t p_buffer_size )
{
  uint_fast8_t          l_index;
  const char           *l_charptr;
  httpclient_request_t *l_request;

  /* Sanity check that we have a request we understand. */
  if ( p_url == NULL )
  {
    return NULL;
  }

  /* Allocate the request structure we'll be using then. */
  l_request = (httpclient_request_t *)malloc(sizeof(httpclient_request_t));
  if ( l_request == NULL )
  {
    return NULL;
  }

  /* Work through the URL, work out what we have - scheme first. */
  if ( strncmp( p_url, "http://", 7 ) == 0 )
  {
    l_charptr = p_url + 7;
    l_request->port = 80;
    l_request->tls = false;
  }
  else if ( strncmp( p_url, "https://", 8 ) == 0 )
  {
    l_charptr = p_url + 8;
    l_request->port = 443;
    l_request->tls = true;
  }
  else
  {
    /* This is an unknown scheme, so we can't proceed. */
    free( l_request );
    return NULL;
  }

  /* Host up next, which may include a port number. */
  for ( l_index = 0; l_index < PCBP_HTTP_HOST_MAXLEN; l_index++ )
  {
    /* Host will either end in slash, or colon if port is also provided. */
    if ( ( l_charptr[l_index] == ':') || ( l_charptr[l_index] == '/' ) )
    {
      /* Save the hostname. */
      strncpy( l_request->host, l_charptr, l_index-1 );
      l_request->host[l_index-1] = '\0';

      /* And, if provided, port. */
      if ( l_charptr[l_index] == ':' )
      {
        l_request->port = atoi( l_charptr+l_index+1 );
      }

      /* All done! */
      break;
    }
  }

  /* Move the pointer onto the path. */
  l_charptr = strchr( l_charptr, '/' );
  if ( l_charptr == NULL )
  {
    /* No path provided, so default to / */
    strcpy( l_request->path, "/" );
  }
  else
  {
    /* Save the rest of the string as path, then. */
    strncpy( l_request->path, l_charptr, PCBP_HTTP_PATH_MAXLEN );
    l_request->path[PCBP_HTTP_PATH_MAXLEN] = '\0';
  }

  /* If we've been provided a buffer, save that too. */
  l_request->response = p_buffer;
  l_request->response_size = p_buffer_size;

  /* Lastly, see if we have a network; if we do then we can get on with it. */
  if ( cyw43_tcpip_link_status( &cyw43_state, CYW43_ITF_STA ) == CYW43_LINK_UP )
  {
    /* Just kick off the request. */
    httpclient_start_request( l_request );
  }
  else
  {
    /* Network needs to be set up; just kick it off and wait. */
    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_async( m_wifi_ssid, m_wifi_password, CYW43_AUTH_WPA2_AES_PSK );
    l_request->status = HTTPCLIENT_WIFI;
  }

  /* All done, so return our request structure to the caller. */
  return l_request;
}


/*
 *  check - looks to see if any processing is required for a request; usually
 *          work is handled through callbacks, but the WiFi connection is special.
 */

httpclient_status_t httpclient_check( httpclient_request_t *p_request )
{
  int l_link_status;

  /* Sanity check that the request is valid. */
  if ( p_request == NULL )
  {
    return HTTPCLIENT_NONE;
  }

  /*
   * Most processing is handled through callbacks, but if the WiFi link is
   * still being set up we have to actively monitor that - connecting first.
   */
  if ( p_request->status == HTTPCLIENT_WIFI )
  {
    /* Check the status of the link. */
    l_link_status = cyw43_tcpip_link_status( &cyw43_state, CYW43_ITF_STA );

    /* If it's a setup failure, flag it to re-attempt after a timeout. */
    if ( ( l_link_status == CYW43_LINK_FAIL ) || 
         ( l_link_status == CYW43_LINK_BADAUTH ) ||
         ( l_link_status == CYW43_LINK_NONET ) )
    {
      /* Set a timer to retry after a period. */
      printf( "CYW43: WiFi login failed (%d), retrying in %d seconds\n", 
              l_link_status, PCBP_HTTP_WIFI_RETRY_MS / 1000 );
      p_request->wifi_retry_time = make_timeout_time_ms( PCBP_HTTP_WIFI_RETRY_MS );
      p_request->status = HTTPCLIENT_WIFI_INIT;
      return p_request->status;
    }

    /* If the link is active, we can initiate the request. */
    if ( l_link_status == CYW43_LINK_UP )
    {
      httpclient_start_request( p_request );
    }
  }

  /* Also, check if there's a retry on the WiFi connection required. */
  if ( ( p_request->status == HTTPCLIENT_WIFI_INIT ) && 
       ( time_reached( p_request->wifi_retry_time ) ) )
  {
    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_async( m_wifi_ssid, m_wifi_password, CYW43_AUTH_WPA2_AES_PSK );
    p_request->status = HTTPCLIENT_WIFI;    
  }

  /* Lastly, return the status contained within the request. */
  return p_request->status;
}


/*
 * get_response - passed a pointer to the response buffer, if the request
 *                has completed - NULL if not.
 */

const char *httpclient_get_response( const httpclient_request_t *p_request )
{
  /* Fairly simple status check - give the buffer if we're complete. */
  if ( ( p_request->status == HTTPCLIENT_COMPLETE ) ||
       ( p_request->status == HTTPCLIENT_TRUNCATED ) )
  {
    return p_request->response;
  }

  /* And return NULL if we're not. */
  return NULL;
}


/*
 * close - free up any resources associated with the request, including the
 *         request itself.
 */

void httpclient_close( httpclient_request_t *p_request )
{
  /* Last thing, free the actual request. */
  free( p_request );

  /* All done. */
  return;
}


/* End of file opt/httpclient.c */
