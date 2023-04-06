/*
 * opt/config.c - part of the PicoW C/C++ Boilerplate Project
 *
 * An optional configuration file handler; this is provided as part of the
 * boilerplate, but if you don't require it (or have built your own) you can
 * safely delete this file (and config.c) and remove it from CMakeLists.txt
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


/* Local header files. */

#include "config.h"
#include "usbfs.h"


/* Module variables. */

static config_t        *m_config_settings;
static uint8_t          m_config_count;
static char             m_config_filename[PCBP_CONFIG_FILENAME_MAXLEN+1];
static uint32_t         m_config_timestamp;
static uint32_t         m_check_ms;
static absolute_time_t  m_next_check;


/* Functions. */

/* Internal functions - used only in this file. */

/*
 * fetch - (re)reads the currently stored file, updating any entries. Note
 *         that deleted entries will *not* be removed.
 */

static void config_fetch( void )
{
  usbfs_file_t *l_fileptr;
  char          l_buffer[128];
  char         *l_nameptr, *l_valueptr, *l_endptr;

  /* Open up the file (if we can) */
  l_fileptr = usbfs_open( m_config_filename, "r" );
  if ( l_fileptr == NULL )
  {
    /* Can't open our config file, so we're done with our defaults. */
    return;
  }

  /* Work through the file one line at a time. */
  while( usbfs_gets( l_buffer, 127, l_fileptr ) != NULL )
  {
    /* Skip any lines which begin with a comment marker. */
    if ( l_buffer[0] == '#' )
    {
      continue;
    }

    /* See if we can spot the field name / value divider. */
    l_valueptr = strchr( l_buffer, ':' );
    if ( l_valueptr == NULL )
    {
      /* No divider, it's a badly formed line so skip it. */
      continue;
    }

    /* Excellent; so, NULL out that divider, and then clip off any whitespace. */
    *l_valueptr = '\0';
    while( isspace( *(++l_valueptr) ) )
    {
      *l_valueptr = '\0';
    }

    /* We also need to trim whitespace from the end. */
    l_endptr = l_valueptr + strlen( l_valueptr );
    while( isspace( *(--l_endptr) ) )
    {
      *l_endptr = '\0';
    }

    /* Phew! We also need to apply the same logic to the name. */
    l_nameptr = l_buffer;
    while( isspace( *l_nameptr ) )
    {
      *(l_nameptr++) = '\0';
    }

    /* We also need to trim whitespace from the end. */
    l_endptr = l_nameptr + strlen( l_nameptr );
    while( isspace( *(--l_endptr) ) )
    {
      *l_endptr = '\0';
    }

    /* And after all that cleaning, we have good values for both. */
    config_set( l_nameptr, l_valueptr );
  }

  /* Must close up the file before we are done. */
  usbfs_close( l_fileptr );

}


/* Public functions. */

/*
 * load - loads the provided configuration file, with any optional defaults.
 *        Also saves the configuration filename and frequency of future checks.
 *        Loading a new configuration will erase any previous one.
 */

void config_load( const char *p_filename, 
                  const config_t *p_defaults, uint16_t p_frequency )
{
  uint_fast8_t    l_index;
  const config_t *l_default;

  /* Clear out any existing configuration. */
  if ( m_config_settings != NULL )
  {
    free( m_config_settings );
    m_config_settings = NULL;
    m_config_count = 0;
  }

  /* If there are any defaults, apply them. */
  if ( p_defaults != NULL )
  {
    /* Loop through the defaults array; an empty name marks the end. */
    l_default = p_defaults;
    while( l_default->name[0] != '\0' )
    {
      config_set( l_default->name, l_default->value );
      l_default++;
    }
  }

  /*
   * Save the configuration filename, and record it's current timestamp, so
   * that we can monitor it for changes later.
   */
  strncpy( m_config_filename, p_filename, PCBP_CONFIG_FILENAME_MAXLEN );
  m_config_filename[PCBP_CONFIG_FILENAME_MAXLEN] = '\0';
  m_config_timestamp = usbfs_timestamp( m_config_filename );
  m_check_ms = p_frequency * 1000;
  m_next_check = make_timeout_time_ms( m_check_ms );

  /* Now just load up the details in that file. */
  config_fetch();

  /* And that's it! */
  return;
}


/*
 * save - writes the configuration settings to the stored file. These new values
 *        will overwrite any existing content, and will include any defaults.
 */

bool config_save( void )
{
  usbfs_file_t *l_fileptr;
  char          l_buffer[128];
  uint_fast8_t  l_index;

  /* Open up the file (if we can) */
  l_fileptr = usbfs_open( m_config_filename, "w" );
  if ( l_fileptr == NULL )
  {
    /* Can't open our config file, so we're done. */
    return false;
  }

  /* And simply work through our configuration. */
  for ( l_index = 0; l_index < m_config_count; l_index++ )
  {
    /* Format the string to be written. */
    snprintf( l_buffer, 127, "%s: %s\n",
              m_config_settings[l_index].name,
              m_config_settings[l_index].value );

    /* And attempt to do so. */
    if ( usbfs_puts( l_buffer, l_fileptr ) <= 0 )
    {
      /* The write failed... */
      usbfs_close( l_fileptr );
      return false;
    }
  }

  /* All is well; close the file and signal our success. */
  usbfs_close( l_fileptr );
  return true;
}


/*
 * check - looks to see if the configuration file has changed since the last
 *         time it was read. Will only do this at the frequency specified when
 *         the configuration was initially loaded.
 *         True is returned if the file has changed and been re-loaded; 
 *         false otherwise.
 */

bool config_check( void )
{
  uint32_t    l_timestamp;

  /* Have we reached the next check time? */
  if ( !time_reached( m_next_check ) )
  {
    /* Not reached the time, so nothing to be done. */
    return false;
  }

  /* This check is due; calculate the next one before anything else. */
  m_next_check = make_timeout_time_ms( m_check_ms );

  /* Now, fetch the timestamp on the configuration file. */
  l_timestamp = usbfs_timestamp( m_config_filename );
  if ( l_timestamp == m_config_timestamp )
  {
    /* File hasn't changed, so nothing to be done. */
    return false;
  }

  /* The file *has* changed, so we'll update the timestamp and fetch the data. */
  m_config_timestamp = l_timestamp;
  config_fetch();

  /* All done, now tell the caller that something changed. */
  return true;
}


/*
 * get - returns a pointer to the value defined for the named parameter; if 
 *       there is no setting for this parameter a NULL is returned.
 */

const char *config_get( const char *p_name )
{
  uint_fast8_t  l_index;
  const char   *l_valueptr = NULL;

  /* Fairly simple then, we just work through our settings. */
  for ( l_index = 0; l_index < m_config_count; l_index++ )
  {
    /* Need an exact string match on the name. */
    if ( strcmp( m_config_settings[l_index].name, p_name ) == 0 )
    {
      l_valueptr = m_config_settings[l_index].value;
      break;
    }
  }

  /* And return whatever we found (defaulting to NULL) */
  return l_valueptr;
}


/*
 * set - sets the configuration parameter to the provided value; if it already
 *       exists the value is overwritten. Memory may be allocated if required.
 */

void config_set( const char *p_name, const char *p_value )
{
  uint_fast8_t  l_index;
  config_t     *l_new_config;

  /* First off, scan through the existing settings to see if we already have it. */
  for ( l_index = 0; l_index < m_config_count; l_index++ )
  {
    /* Do we have a match? */
    if ( strcmp( m_config_settings[l_index].name, p_name ) == 0 )
    {
      /* Good; just copy in the new value then. */
      strncpy( m_config_settings[l_index].value, p_value, PCBP_CONFIG_VALUE_MAXLEN );
      m_config_settings[l_index].value[PCBP_CONFIG_VALUE_MAXLEN] = '\0';

      /* No more work needs doing. */
      return;
    }
  }

  /* We haven't found it, so we add it to the end; do we need more space? */
  if ( (m_config_count%5) == 0 )
  {
    /*
     * We (re)allocate memory in blocks of 5; this is an attempt to minimise
     * allocations without taking excessive, precious RAM.
     */
    l_new_config = realloc( m_config_settings, sizeof( config_t ) * ( m_config_count + 5 ) );
    if ( l_new_config == NULL )
    {
      /* The allocation failed, so leave everything alone. */
      return;
    }

    /* Otherwise, use this new memory (which may or may not have moved) */
    m_config_settings = l_new_config;
  }

  /* Lastly, add the new entry to the end of the list. */
  strncpy( m_config_settings[m_config_count].name, p_name, PCBP_CONFIG_NAME_MAXLEN );
  m_config_settings[m_config_count].name[PCBP_CONFIG_NAME_MAXLEN] = '\0';
  strncpy( m_config_settings[m_config_count].value, p_value, PCBP_CONFIG_VALUE_MAXLEN );
  m_config_settings[m_config_count].value[PCBP_CONFIG_VALUE_MAXLEN] = '\0';

  /* Increment the setting count, and we're done. */
  m_config_count++;
  return;
}

/* End of file opt/config.c */
