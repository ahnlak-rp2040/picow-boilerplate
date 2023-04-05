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

#include "config.h"
#include "usbfs.h"


/* Functions. */

/*
 * main() - the entrypoint of the application; this is what runs when the PicoW
 *          starts up, and would never normally exit.
 */

int main()
{
  int blink_rate = 250;

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
