/*
 * usbfs/usbfs.cpp - part of the PicoW C++ Boilerplate Project
 *
 * usbfs is the library that handles presenting a filesystem to the host
 * over USB; the main aim is to make it easy to present configuration files
 * to remove the need to recompile (for example, WiFi settings)
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * This file is released under the BSD 3-Clause License; see LICENSE for details.
 */


/* System headers. */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Local headers. */

#include "tusb.h"
#include "ff.h"
#include "diskio.h"
#include "usbfs.h"


/* Module variables. */

static FATFS m_fatfs;


/* Functions.*/

/*
 * init - initialised the TinyUSB library, and also the FatFS handling.
 */

void usbfs_init( void )
{
  FRESULT   l_result;
  MKFS_PARM l_options;

  /* First order of the day, is TinyUSB. */
  tusb_init();

  /* And also mount our FatFS partition. */
  l_result = f_mount( &m_fatfs, "", 1 );

  /* If there was no filesystem, make one. */
  if ( l_result == FR_NO_FILESYSTEM )
  {
    /* Set up the options, and format. */
    l_options.fmt = FM_ANY | FM_SFD;
    l_result = f_mkfs( "", &l_options, m_fatfs.win, FF_MAX_SS );
    if ( l_result != FR_OK )
    {
      return;
    }

    /* Set the label on the volume to something sensible. */
    f_setlabel( UFS_LABEL );

    /* And re-mount. */
    l_result = f_mount( &m_fatfs, "", 1 );
  }

  /* All done. */
  return;
}


/*
 * update - run any updates required on TinyUSB or the filesystem.
 */

void usbfs_update( void )
{
  /* Ask TinyUSB to run any outstanding tasks. */
  tud_task();

  /* All done. */
  return;
}


/*
 * sleep_ms - a replacement for the standard sleep_ms function; this will
 *            run update every millisecond until we reach the requested time,
 *            to ensure that TinyUSB doesn't run into trouble.
 */

void usbfs_sleep_ms( uint32_t p_milliseconds )
{
  absolute_time_t l_target_time;

  /* Work out when we want to 'sleep' until. */
  l_target_time = make_timeout_time_ms( p_milliseconds );

  /* Now enter a busy(ish) loop until that time. */
  while( !time_reached( l_target_time ) )
  {
    /* Run any updates. */
    usbfs_update();

    /* And briefly pause. */
    sleep_ms(1);
  }

  /* All done. */
  return;
}


usbfs_file_t *usbfs_open( const char *p_pathname, const char *p_mode )
{
  return nullptr;
}

bool usbfs_close( usbfs_file_t *p_fileptr )
{
  return true;
}

size_t usbfs_read( void *p_buffer, size_t p_size, usbfs_file_t *p_fileptr )
{
  return 0;
}

size_t usbfs_write( const void *p_buffer, size_t p_size, usbfs_file_t *p_fileptr )
{
  return 0;
}

char *usbfs_gets( char *p_buffer, size_t p_size, usbfs_file_t *p_fileptr )
{
  return nullptr;
}

size_t usbfs_puts( const char *p_buffer, size_t p_size, usbfs_file_t *p_fileptr )
{
  return 0;
}

/* End of file usbfs.cpp */
