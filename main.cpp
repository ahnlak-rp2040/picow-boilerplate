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


/* Functions. */

/*
 * main() - the entrypoint of the application; this is what runs when the PicoW
 *          starts up, and would never normally exit.
 */

int main()
{
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

  /* Enter the main program loop now. */
  while( true )
  {
    /* The blink is very simple, just toggle the GPIO pin high and low. */
    cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 );
    printf( "Light on...\n" );
    usbfs_sleep_ms( 250 );

    cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
    printf( "Light off...\n" );
    usbfs_sleep_ms( 250 );
  }

  /* We would never expect to reach an end....! */
  return 0;
}
