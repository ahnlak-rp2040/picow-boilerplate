/*
 * main.cpp - part of the PicoW C/C++ Boilerplate Project
 *
 * This file defines the main() function, the entrypoint for your program.
 *
 * After the general setup, it simply blinks the LED on your PicoW as the
 * traditional 'Hello World' of the Pico!
 *
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * This file is released under the BSD 3-Clause License; see LICENSE for details.
 */

/* Standard header files. */

#include <stdio.h>
#include <stdlib.h>


/* SDK header files. */

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"


/* Local header files. */

#include "usbfs.h"
#include "opt/config.h"
#include "opt/httpclient.h"


/* Functions. */

/*
 * main() - the entrypoint of the application; this is what runs when the PicoW
 *          starts up, and would never normally exit.
 */

int main()
{
  int blink_rate = 250;
  httpclient_request_t *http_request;


  /* Initialise stdio handling. */
  stdio_init_all();

  /* Initialise the WiFi chipset. */
  if ( cyw43_arch_init() )
  {
    printf( "Failed to initialise the WiFI chipset (cyw43)\n" );
    return 1;
  }

  /* And the USB handling. */
  usbfs_init();

  /* Declare some default configuration details. */
  config_t default_config[] = 
  {
    { "BLINK_RATE", "250" },
    { "WIFI_SSID", "my_network" },
    { "WIFI_PASSWORD", "my_password" },
    { "", "" }
  };

  /* Set up the initial load of the configuration file. */
  config_load( "config.txt", default_config, 10 );

  /* Save it straight out, to preserve any defaults we put there. */
  config_save();

  /* See if we have a blink rate in there. */
  const char *blink_rate_string = config_get( "BLINK_RATE" );
  if ( ( blink_rate_string != NULL ) &&
       ( atoi( blink_rate_string ) > 0 ) )
  {
    blink_rate = atoi( blink_rate_string );
  }

  /* Set up a simple web request. */
  httpclient_set_credentials( config_get( "WIFI_SSID" ), config_get( "WIFI_PASSWORD" ) );
  http_request = httpclient_open( "https://httpbin.org/get", NULL, 1024 );

  /* Enter the main program loop now. */
  while( true )
  {
    /* Monitor the configuration file. */
    if ( config_check() )
    {
      /* This indicates the configuration has changed - handle it if required. */
      blink_rate_string = config_get( "BLINK_RATE" );
      if ( ( blink_rate_string != NULL ) &&
           ( atoi( blink_rate_string ) > 0 ) )
      {
        blink_rate = atoi( blink_rate_string );
      }

      /* Switch to potentially new WiFi credentials. */
      httpclient_set_credentials( config_get( "WIFI_SSID" ), config_get( "WIFI_PASSWORD" ) );
    }

    /* Service the http request, if still active. */
    if ( http_request != NULL )
    {
      httpclient_status_t l_status = httpclient_check( http_request );
      if ( l_status == HTTPCLIENT_COMPLETE )
      {
        printf( "HTTP response code %d\n", http_request->http_status );
        printf( "Response:\n%s\n", httpclient_get_response( http_request ) );
        httpclient_close( http_request );
        http_request = NULL;
      }
    }

    /* The blink is very simple, just toggle the GPIO pin high and low. */
    printf( "Blinking at a rate of %d ms\n", blink_rate );
    cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 );
    usbfs_sleep_ms( blink_rate );

    cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
    usbfs_sleep_ms( blink_rate );
  }

  /* We would never expect to reach an end....! */
  return 0;
}

/* End of file main.cpp */
