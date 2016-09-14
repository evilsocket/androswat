/*
 * Copyright (c) 2016, Simone Margaritelli <evilsocket at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of ARM Inject nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>

#include "tracer.h"

typedef enum {
  ACTION_HELP = 0,
  ACTION_SHOW,
  ACTION_READ,
  ACTION_DUMP
}
action_t;

static struct option options[] = {
  { "pid",    required_argument, 0, 'p' },
  { "name",   required_argument, 0, 'n' },
  { "output", required_argument, 0, 'o' },
  { "size",   required_argument, 0, 's' },

  { "help", no_argument,       0, 'H' },
  { "show", no_argument,       0, 'S' },
  { "read", required_argument, 0, 'R' },
  { "dump", required_argument, 0, 'D' },
  {0,0,0,0}
};

static pid_t     __pid     = -1;
static string    __name    = "";
static action_t  __action  = ACTION_HELP;
static Process  *__process = NULL;
static string    __output  = "";
static uintptr_t __address = -1;
static size_t    __size    = -1;

void help( const char *name );
void app_init( const char *name );
void action_show( const char *name );
void action_read( const char *name );
void action_dump( const char *name );

int main( int argc, char **argv )
{
  int c, option_index = 0;
  while (1) {
    c = getopt_long( argc, argv, "o:p:n:s:HSD:R:", options, &option_index );
    if( c == -1 ){
      break;
    }

    switch(c) {
      case 'p':
        __pid = strtol( optarg, NULL, 10 );
      break;

      case 'n':
        __name = optarg;
      break;

      case 'o':
        __output = optarg;
      break;

      case 's':
        __size = strtoul( optarg, NULL, 10 );
      break;

      case 'S':
        __action = ACTION_SHOW;
      break;

      case 'R':
        __action = ACTION_READ;
        __address = strtoul( optarg, NULL, 16 );
      break;

      case 'D':
        __action  = ACTION_DUMP;
        __address = strtoul( optarg, NULL, 16 );
      break;

      case 'H':
        help( argv[0] );
      break;

      default:
        help( argv[0] );
    }
  }

  if( __action == ACTION_HELP ){
    help( argv[0] );
  }

  app_init( argv[0] );

  if( __action == ACTION_SHOW ){
    action_show( argv[0] );
  }
  else if( __action == ACTION_READ ){
    action_read( argv[0] );
  }
  else if( __action == ACTION_DUMP ){
    action_dump( argv[0] );
  }

  delete __process;

  return 0;
}

void help( const char *name ){
  printf( "Usage: %s <options> <action>\n\n", name );
  printf( "OPTIONS:\n\n" );
  printf( "  --pid    | -p PID  : Select process by pid.\n" );
  printf( "  --name   | -n NAME : Select process by name.\n" );
  printf( "  --size   | -s SIZE : Set size.\n" );
  printf( "  --output | -o FILE : Set output file.\n" );

  printf( "\nACTIONS:\n\n" );
  printf( "  --help | -H         : Show help menu.\n" );
  printf( "  --show | -S         : Show process informations.\n" );
  printf( "  --dump | -D ADDRESS : Dump memory region containing a specific address to a file, requires -o option.\n" );
  printf( "  --read | -R ADDRESS : Read SIZE bytes from address and prints them, requires -s option.\n" );
  exit(0);
}

void app_init( const char *name ) {
  if( getuid() != 0 ){
    fprintf( stderr, "ERROR: This program must be runned as root.\n\n" );
    help( name );
  }
  else if( __pid != -1 && __name != "" ){
    fprintf( stderr, "ERROR: --pid and --name options are mutually exclusive.\n\n" );
    help( name );
  }
  else if( __pid != -1 ){
    __process = new Process( __pid );
  }
  else if( __name != "" ){
    __process = Process::find( __name.c_str() );
  }
  else {
    fprintf( stderr, "ERROR: One of --pid or --name options are required.\n\n" );
    help( name );
  }
}

void action_show( const char *name ) {
  __process->dump();
}

void action_read( const char *name ) {
  if( __size == -1 ){
    fprintf( stderr, "ERROR: --read action require --size option to be set.\n\n" );
    help( name );
  }

  const MemoryMap *mem = __process->findRegion(__address);
  if( mem == NULL ){
    FATAL( "Could not find address %p in the process space.\n", __address );
  }

  Tracer tracer( __process );

  printf( "Reading %lu bytes from %p ( %s ) ...\n\n", __size, __address, mem->name().c_str() );

  unsigned char *buffer = new unsigned char[ __size ];
  if( read( __address, buffer, __size ) ){
    unsigned char *p = &buffer[0], *end = p + __size;
    unsigned int step = 16;

    while( p < end ) {
      printf( "%08x  |  ", __address );
      for( int i = 0; i < step; ++i ){
        printf( "%02x ", p[i] );
      }
      printf( " |  ");
      for( int i = 0; i < step; ++i ){
        printf( "%c", isprint(p[i]) ? p[i] : '.' );
      }

      printf( "\n" );
      p += step;
      __address += step;
    }

  }
  else {
    perror("ptrace");
    fprintf( stderr, "Could not read from process.\n" );
  }

  delete[] buffer;
}

void action_dump( const char *name ) {
  if( __output == "" ){
    fprintf( stderr, "ERROR: --dump action require --output option to be set.\n\n" );
    help( name );
  }

  Tracer tracer( __process );
  tracer.dumpRegion( __address, __output.c_str() );
}
