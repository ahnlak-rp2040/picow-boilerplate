/*
 * opt/config.h - part of the PicoW C/C++ Boilerplate Project
 *
 * Header for an optional configuration file handler; this is provided as part
 * of the boilerplate, but if you don't require it (or have built your own)
 * you can safely delete this file (and config.c) and remove it from your
 * CMakeLists.txt file.
 * 
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * This file is released under the BSD 3-Clause License; see LICENSE for details.
 */

#pragma once


/* Constants. */

#define PCBP_CONFIG_NAME_MAXLEN     31
#define PCBP_CONFIG_VALUE_MAXLEN    63
#define PCBP_CONFIG_MAX_ENTRIES     50
#define PCBP_CONFIG_FILENAME_MAXLEN 31


/* Structures */

typedef struct
{
  char  name[PCBP_CONFIG_NAME_MAXLEN+1];
  char  value[PCBP_CONFIG_VALUE_MAXLEN+1];
} config_t;


/* Function prototypes. */

#ifdef __cplusplus
extern "C" {
#endif

void        config_load( const char *, const config_t *, uint16_t );
bool        config_save( void );
bool        config_check( void );

const char *config_get( const char * );
void        config_set( const char *, const char * );

#ifdef __cplusplus
}
#endif


/* End of file opt/config.h */
