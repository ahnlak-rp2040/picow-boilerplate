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
#include "lwip/pbuf.h"
#include "lwip/altcp.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"


/* Local header files. */

#include "httpclient.h"


/* Module variables. */

static char         m_wifi_ssid[PCBP_HTTP_SSID_MAXLEN+1];
static char         m_wifi_password[PCBP_HTTP_PASSWORD_MAXLEN+1];


/* Functions. */

/*
 * Internal functions - utility functions and callbacks, only called from this
 *                      file or the network libraries. Listed first as they
 *                      don't have any external declarations to define them
 *                      before use.
 */
 

/*
 * close_pcb - shuts down and de-allocates the pcb; this ends the network
 *             side of the conversation, but leaves the request object available
 *             for clients to extract data from.
 */

static err_t httpclient_close_pcb( httpclient_request_t *p_request )
{
  err_t l_retval = ERR_OK;

  /* Sanity check the request. */
  if ( p_request == NULL )
  {
    return ERR_OK;
  }

  /* Only need to work if the pcb is allocated. */
  if ( p_request->pcb != NULL )
  {
    /* Remove the callback pointers. */
    altcp_arg( p_request->pcb, NULL );
    altcp_recv( p_request->pcb, NULL );
    altcp_err( p_request->pcb, NULL );
    altcp_poll( p_request->pcb, NULL, 0 );

    /* And close the pcb itself */
    if ( altcp_close( p_request->pcb ) != ERR_OK )
    {
      /* Abort the connection, remembering to let the caller know! */
      printf( "Failed to close pcb!\n" );
      altcp_abort( p_request->pcb );
      l_retval = ERR_ABRT;
    }

    /* All cleared. */
    p_request->pcb = NULL;
  }

  /* All done. */
  return l_retval;
}


/*
 * connect_callback - called once the connection is established.
 */

static err_t httpclient_connect_callback( void *p_request, 
                                          struct altcp_pcb *p_pcb, err_t p_err )
{
  httpclient_request_t *l_request = (httpclient_request_t *)p_request;
  int                   l_buflen;
  err_t                 l_retval;

  /* Check that the error code is clear. */
  if ( p_err != ERR_OK )
  {
    printf( "connect callback with error - %d\n", l_retval );
    l_request->status = HTTPCLIENT_FAILED;
    return httpclient_close_pcb( l_request );
  }

  /* Construct our request. */
  l_request->status = HTTPCLIENT_REQUEST;
  l_buflen = snprintf( 
    l_request->header_buffer, PCBP_HEADER_BUFSIZE,
    "GET %s HTTP/1.1\r\n"                               /* URL path */
    "Host: %s\r\n"                                      /* Request host */
    "User-Agent: " PCBP_REQUEST_USER_AGENT "\r\n"       /* Our user agent */
    "Accept: */*\r\n"                                   /* Accept anything */
    "Connection: Close\r\n"                             /* No persistence */
    "\r\n",
    l_request->path, l_request->host
  );

  /* If that got truncated, we can't send it. Bugger. */
  if ( l_buflen >= PCBP_HEADER_BUFSIZE )
  {
    printf( "Request too long - %d >= %d\n", l_buflen, PCBP_HEADER_BUFSIZE );
    l_request->status = HTTPCLIENT_FAILED;
    return httpclient_close_pcb( l_request );
  }

  /* Send it to the remote server. */
  l_retval = altcp_write( l_request->pcb, l_request->header_buffer, 
                          l_buflen, TCP_WRITE_FLAG_COPY );
  if ( l_retval != ERR_OK )
  {
    printf( "altcp_write() failed - %d\n", l_retval );
    l_request->status = HTTPCLIENT_FAILED;
    return httpclient_close_pcb( l_request );
  }

  /* Ask to send (not sure this is necessary, but...) */
  altcp_output( l_request->pcb );

  /* And so we're waiting on the response code. */
  l_request->status = HTTPCLIENT_RESPONSE_STATUS;
  return ERR_OK;
}


/*
 * connect - once we have the host's address, this initiates the connection.
 */

static err_t httpclient_connect( httpclient_request_t *p_request, ip_addr_t *p_addr )
{
  err_t     l_retval;

  /* Fairly simple call into the library call; handle any error. */
  p_request->status = HTTPCLIENT_CONNECT;
  l_retval = altcp_connect( p_request->pcb, p_addr, 
                            p_request->port, httpclient_connect_callback );
  if ( l_retval != ERR_OK )
  {
    printf( "altcp_connect() failed to connect to %s:%d - %d\n", ipaddr_ntoa( p_addr ), p_request->port, l_retval );
    p_request->status = HTTPCLIENT_FAILED;
    return httpclient_close_pcb( p_request );
  }

  return l_retval;
}


/*
 * recv_callback - called whenever data is received from the server.
 */

static err_t httpclient_recv_callback( void *p_request, struct altcp_pcb *p_pcb,
                                       struct pbuf *p_buf, err_t p_error )
{
  httpclient_request_t *l_request = (httpclient_request_t *)p_request;
  u16_t                 l_eolptr, l_clptr, l_eohptr;
  uint16_t              l_line_length, l_data_length, l_to_read, l_bytes_read;
  char                 *l_charptr;
  bool                  l_truncated;

  /* A NULL pbuf indicates the connection is terminating. */
  if ( p_buf == NULL )
  {
    l_request->status = HTTPCLIENT_COMPLETE;
    return httpclient_close_pcb( l_request );
  }

  /* Process appropriately then, depending on where we are in the datastream. */
  if ( l_request->status == HTTPCLIENT_RESPONSE_STATUS )
  {
    /*
     * A status of RESPONSE_STATUS means we've sent the request, so we expect
     * to get the HTTP response code next.
     */

    /* Do we have a complete line? */
    l_eolptr = pbuf_memfind( p_buf, "\r\n", 2, 0 );
    if ( l_eolptr != 0xFFFF )
    {
      /* Pull this line into the buffer. */
      l_line_length = ( l_eolptr > PCBP_HEADER_BUFSIZE ) ? PCBP_HEADER_BUFSIZE : l_eolptr;
      pbuf_copy_partial( p_buf, l_request->header_buffer, l_line_length, 0 );
      l_request->header_buffer[l_line_length] = '\0';

      /* Try and parse it; it should be the standard format! */
      l_charptr = strtok( l_request->header_buffer, " " );
      if ( l_charptr != NULL )
      {
        /* Look for the next word. */
        l_charptr = strtok( NULL, " " );
        if ( l_charptr != NULL )
        {
          /* This word is the HTTP response code. */
          l_request->http_status = atoi( l_charptr );

          /* Consume the line. */
          altcp_recved( p_pcb, l_line_length+2 );

          /* And update the status to indicate we now want headers. */
          l_request->status = HTTPCLIENT_HEADERS;
        }
      }
    }
  }

  if ( l_request->status == HTTPCLIENT_HEADERS )
  {
    /*
     * A status of HEADERS means we have a response code, so now we wait
     * to receive the whole header block.
     */
    l_eohptr = pbuf_memfind( p_buf, "\r\n\r\n", 4, 0 );
    if ( l_eohptr != 0xFFFF )
    {
      /* Excellent; we have the headers so look for the content length. */
      l_clptr = pbuf_memfind( p_buf, "Content-Length", 14, 0 );
      if ( l_clptr != 0xFFFF )
      {
        /* Find the end of that particular header line. */
        l_eolptr = pbuf_memfind( p_buf, "\r\n", 2, l_clptr );
        if ( l_eolptr != 0xFFFF )
        {
          /* Extract that line. */
          pbuf_copy_partial( p_buf, l_request->header_buffer, l_eolptr-l_clptr, l_clptr );
          l_request->header_buffer[l_eolptr-l_clptr] = '\0';
          printf( "Found header: '%s'\n", l_request->header_buffer );

          /* Find the dividing colon; the length follows. */
          l_charptr = strchr( l_request->header_buffer, ':' );
          if ( l_charptr != NULL )
          {
            l_request->content_length = atoi( l_charptr+1 );
          }
        }
      }

      /* Check we have a content length; if not, we're not going to get far. */
      if ( l_request->content_length == 0 )
      {
        printf( "Content-length not received\n" );
        l_request->status = HTTPCLIENT_FAILED;
        return httpclient_close_pcb( l_request );
      }

      /* If the response buffer is not allocated, allocate it. */
      if ( l_request->response == NULL )
      {
        /* Work out how much data we will read; potentially capped. */
        l_data_length = ( l_request->response_max_size < l_request->content_length ) ?
                          l_request->response_max_size : l_request->content_length;
        /* Grab the memory. */
        l_request->response = (char *)malloc( l_data_length+1 );
        if ( l_request->response == NULL )
        {
          printf( "Unable to allocated %d bytes for response buffer\n", l_data_length+1 );
          l_request->status = HTTPCLIENT_FAILED;
          return httpclient_close_pcb( l_request );
        }

        /* And remember we did so. */
        l_request->response_allocated = true;
        l_request->response_length = 0;
      }

      /* We have, so consume the headers and start working on the data. */
      altcp_recved( p_pcb, l_eohptr+4 );
      p_buf = pbuf_free_header( p_buf, l_eohptr+4 );
      l_request->status = HTTPCLIENT_DATA;
    }
  }

  if ( l_request->status == HTTPCLIENT_DATA )
  {
    /*
     * A status of DATA means we're now receiving the body data. We keep reading
     * until we hit our content length, or the maximum response size if smaller.
     */
    l_truncated = false;
    l_to_read = p_buf->tot_len;
    if ( ( l_request->response_length + l_to_read ) > l_request->response_max_size )
    {
      l_truncated = true;
      l_to_read = l_request->response_max_size - l_request->response_length;
    }
    if ( ( l_request->response_length + l_to_read ) > l_request->content_length )
    {
      l_truncated = true;
      l_to_read = l_request->content_length - l_request->response_length;
    }

    /* Phew; now read ... however much we think we want. */
    l_bytes_read = pbuf_copy_partial( p_buf, l_request->response+l_request->response_length,
                                      l_to_read, 0 );
    l_request->response_length += l_bytes_read;

    /* Consume that. */
    altcp_recved( p_pcb, l_bytes_read );
    p_buf = pbuf_free_header( p_buf, l_bytes_read );

    /*
     * And lastly, if we reached our limit it's complete, and if we would have
     * exceed it then we truncated.
     */
    if ( ( l_request->response_length == l_request->content_length ) ||
         ( l_request->response_length == l_request->response_max_size ) )
    {
      l_request->status = HTTPCLIENT_COMPLETE;
    }
    if ( l_truncated )
    {
      l_request->status = HTTPCLIENT_TRUNCATED;
    }
  }

  /* All done, return we are happy. */
  return ERR_OK;
}


/*
 * poll_callback - called after a timeout period within lwIP, to allow us to
 *                 abort if things are taking too long. 
 */

static err_t httpclient_poll_callback( void *p_request, struct altcp_pcb *p_pcb )
{
  httpclient_request_t *l_request = (httpclient_request_t *)p_request;

  /* We *might* have some data, so let's call it 'truncated' */
  l_request->status = HTTPCLIENT_TRUNCATED;
  return httpclient_close_pcb( l_request );
}


/*
 * err_callback - called to inform us of any errors within lwIP
 */

static void httpclient_err_callback( void *p_request, err_t p_error )
{
  httpclient_request_t *l_request = (httpclient_request_t *)p_request;

  /* All we can really do is close it and flag it. */
  printf( "Error callback - %d\n", p_error );
  l_request->status = HTTPCLIENT_FAILED;
  httpclient_close_pcb( l_request );

  /* No return code here. */
  return;  
}


/*
 * dns_callback - called once a DNS lookup has completed; if the host is found,
 *                this is where we initiate the connection.
 */

void httpclient_dns_callback( const char *p_name, 
                              const ip_addr_t *p_address, void *p_request )
{
  httpclient_request_t *l_request = (httpclient_request_t *)p_request;

  /* If the address wasn't found, we abort. */
  if ( p_address == NULL )
  {
    printf( "DNS callback did not find address\n" );
    l_request->status = HTTPCLIENT_FAILED;
    httpclient_close_pcb( l_request );
    return;
  }

  /* Otherwise, save the address and initiate the connection. */
  memcpy( &l_request->host_addr, p_address, sizeof( ip_addr_t ) );
  httpclient_connect( l_request, &l_request->host_addr );

  /* All done. */
  return;
}


/* Internal functions - used only in this file. */


/*
 * start_request - called when the network is available, in order to intiate
 *                 the communications to the web server.
 */

static void httpclient_start_request( httpclient_request_t *p_request )
{
  err_t               l_retval;

  /* Sanity check the request. */
  if ( p_request == NULL )
  {
    return;
  }

  /* Good; now, allocate a suitable pcb, depending on the request type. */
  if ( p_request->tls )
  {
    p_request->pcb = altcp_tls_new( altcp_tls_create_config_client( NULL, 0 ), IPADDR_TYPE_V4 );
    mbedtls_ssl_set_hostname( altcp_tls_context( p_request->pcb ), p_request->host );
  }
  else
  {
    p_request->pcb = altcp_new( NULL );
  }

  /* Now configure this pcb with our various state objects and callbacks. */
  altcp_arg( p_request->pcb, p_request );
  altcp_recv( p_request->pcb, httpclient_recv_callback );
  altcp_err( p_request->pcb, httpclient_err_callback );
  altcp_poll( p_request->pcb, httpclient_poll_callback, PCBP_HTTP_TIMEOUT_SECS *2 );

  /* Now we lookup the hostname in DNS - lwIP functions need wrapping. */
  p_request->status = HTTPCLIENT_DNS;
  cyw43_arch_lwip_begin();
  l_retval = dns_gethostbyname( p_request->host, &p_request->host_addr, 
                                httpclient_dns_callback, p_request );
  cyw43_arch_lwip_end();

  /* If the lookup returned OK, we already have the address. */
  if ( l_retval == ERR_OK )
  {
    /* Can directly initiate the next step then. */
    httpclient_connect( p_request, &p_request->host_addr );
  }
  else if ( l_retval != ERR_INPROGRESS )
  {
    /* Something failed, so we need to abort the whole connection. */
    printf( "dns_gethostbyname() failed - %d\n", l_retval );
    p_request->status = HTTPCLIENT_FAILED;
    httpclient_close_pcb( p_request );
    return;
  }

  /* All done - everything else is handled by callbacks. */
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
      strncpy( l_request->host, l_charptr, l_index );
      l_request->host[l_index] = '\0';

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
  l_request->response_max_size = p_buffer_size;
  l_request->response_allocated = false;

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
  /* If the response has been allocated, free that. */
  if ( p_request->response_allocated && ( p_request->response != NULL ) )
  {
    free( p_request->response );
    p_request->response = NULL;
  }

  /* Last thing, free the actual request. */
  free( p_request );

  /* All done. */
  return;
}


/* End of file opt/httpclient.c */
