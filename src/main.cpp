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
#include <algorithm>

#include "tracer.h"

typedef enum {
  ACTION_HELP = 0,
  ACTION_SHOW,
  ACTION_SEARCH,
  ACTION_READ,
  ACTION_DUMP,
  ACTION_INJECT
}
action_t;

static struct option options[] = {
  { "pid",    required_argument, 0, 'p' },
  { "name",   required_argument, 0, 'n' },
  { "output", required_argument, 0, 'o' },
  { "size",   required_argument, 0, 's' },
  { "filter", required_argument, 0, 'f' },

  { "help",   no_argument,       0, 'H' },
  { "show",   no_argument,       0, 'S' },
  { "search", required_argument, 0, 'X' },
  { "read",   required_argument, 0, 'R' },
  { "dump",   required_argument, 0, 'D' },
  { "inject", required_argument, 0, 'I' },
  {0,0,0,0}
};

static pid_t          __pid     = -1;
static string         __name    = "";
static action_t       __action  = ACTION_HELP;
static Process       *__process = NULL;
static string         __output  = "";
static uintptr_t      __address = -1;
static size_t         __size    = -1;
static string         __library = "";
static string         __hex_pattern = "";
static unsigned char *__pattern = NULL;
static string         __filter  = "";

void help( const char *name );
void app_init( const char *name );

unsigned char *parsehex( char *hex );
void dumphex( unsigned char *buffer, size_t base, size_t size, const char *padding = "", size_t step = 16 );

void action_show( const char *name );
void action_search( const char *name );
void action_read( const char *name );
void action_dump( const char *name );
void action_inject( const char *name );

int main( int argc, char **argv )
{
  int c, option_index = 0;
  while (1) {
    c = getopt_long( argc, argv, "o:p:n:s:f:HSX:D:R:I:", options, &option_index );
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

      case 'f':
        __filter = optarg;
      break;

      case 'S':
        __action = ACTION_SHOW;
      break;

      case 'X':
        __action  = ACTION_SEARCH;
        __hex_pattern = optarg;
        __pattern = parsehex( optarg );
        if( !__pattern ){
          help( argv[0] );
        }
      break;

      case 'R':
        __action = ACTION_READ;
        __address = strtoul( optarg, NULL, 16 );
      break;

      case 'D':
        __action  = ACTION_DUMP;
        __address = strtoul( optarg, NULL, 16 );
      break;

      case 'I':
        __action  = ACTION_INJECT;
        __library = optarg;
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

  switch(__action) {
    case ACTION_SHOW:   action_show( argv[0] ); break;
    case ACTION_READ:   action_read( argv[0] ); break;
    case ACTION_SEARCH: action_search( argv[0] ); break;
    case ACTION_DUMP:   action_dump( argv[0] ); break;
    case ACTION_INJECT: action_inject( argv[0] ); break;
  }

  delete __process;
  if( __pattern != NULL ){
    delete[] __pattern;
  }

  return 0;
}

void help( const char *name ){
  printf( "Usage: %s <options> <action>\n\n", name );
  printf( "OPTIONS:\n\n" );
  printf( "  --pid    | -p PID  : Select process by pid.\n" );
  printf( "  --name   | -n NAME : Select process by name.\n" );
  printf( "  --size   | -s SIZE : Set size.\n" );
  printf( "  --output | -o FILE : Set output file.\n" );
  printf( "  --filter | -f EXPR : Specify a filter for the memory region name.\n" );

  printf( "\nACTIONS:\n\n" );
  printf( "  --help   | -H         : Show help menu.\n" );
  printf( "  --show   | -S         : Show process informations.\n" );
  printf( "  --search | -X HEX     : Search for the given pattern ( in hex ) in the process address space, might be used with --filter option.\n" );
  printf( "  --read   | -R ADDRESS : Read SIZE bytes from address and prints them, requires -s option.\n" );
  printf( "  --dump   | -D ADDRESS : Dump memory region containing a specific address to a file, requires -o option.\n" );
  printf( "  --inject | -I LIBRARY : Inject the shared LIBRARY into the process.\n" );
  exit(0);
}

void app_init( const char *name ) {
  printf( "AndroSwat v1.0\n" );

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

  printf( "Process: %s ( pid=%d )\n\n", __process->name().c_str(), __process->pid() );
}

unsigned char *parsehex( char *hex ) {
  size_t len = strlen(hex);

  if( len % 2 != 0 ){
    fprintf( stderr, "ERROR: Invalid hexadecimal pattern.\n\n" );
    return NULL;
  }

  unsigned char *dst = new unsigned char[ len / 2 ];
  int i = 0;

  for( char *p = hex; p < hex + len; p += 2 ) {
    unsigned int byte = 0;
    if( sscanf( p, "%2x", &byte ) != 1 ) {
      fprintf( stderr, "ERROR: Invalid hexadecimal pattern.\n\n" );
      delete[] dst;
      return NULL;
    }

    dst[i++] = byte & 0xff;
  }
  return dst;
}

void dumphex( unsigned char *buffer, size_t base, size_t size, const char *padding, size_t step ) {
  unsigned char *p = &buffer[0], *end = p + size;

  while( p < end ) {
    unsigned int left = end - p;
    step = std::min( step, left );
    printf( "%s%08X | ", padding, base );
    for( int i = 0; i < step; ++i ){
      printf( "%02x ", p[i] );
    }
    printf( "| ");
    for( int i = 0; i < step; ++i ){
      printf( "%c", isprint(p[i]) ? p[i] : '.' );
    }

    printf( "\n" );
    p += step;
    base += step;
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

  // align size
  __size = ( __size % sizeof(long) ? __size + (sizeof(long) - __size % sizeof(long)) : __size );

  printf( "Reading %lu bytes from %p ( %s ) ...\n\n", __size, __address, mem->name().c_str() );

  unsigned char *buffer = new unsigned char[ __size ];
  if( tracer.read( __address, buffer, __size ) ){
    dumphex( buffer, __address, __size );
  }
  else {
    perror("ptrace");
    fprintf( stderr, "Could not read from process.\n" );
  }

  delete[] buffer;
}

void action_search( const char *name ) {
  size_t pattern_size = __hex_pattern.size() / 2;

  printf( "Searching for pattern :\n\n" );
  dumphex( __pattern, 0, pattern_size, "  " );
  printf("\n");

  Tracer tracer( __process );

  PROCESS_FOREACH_MAP_CONST( __process ){
    if( __filter.size() != 0 && i->name().find(__filter) == string::npos ){
      continue;
    }
    // printf( "  Searching in %p-%p ( %s ) ...\n", i->begin(), i->end(), i->name().c_str() );
    unsigned char *buffer = new unsigned char[ i->size() ];

    if( tracer.read( i->begin(), buffer, i->size() ) ){
      size_t end_offset = i->size() - pattern_size;
      for( size_t off = 0; off <= end_offset; ++off ){
        if( buffer[off] == __pattern[0] && memcmp( &buffer[off], __pattern, pattern_size ) == 0 ){
          printf( "Match @ offset %lu of %p-%p ( %s ):\n\n", off, i->begin(), i->end(), i->name().c_str() );
          dumphex( &buffer[off], i->begin() + off, std::min( 64, (int)(end_offset - off) ), "  " );
          printf("\n");
        }
      }
    }
    else {
      printf( "  Could not read %p-%p ( %s ).\n", i->begin(), i->end(), i->name().c_str() );
    }

    delete[] buffer;
  }
}

void action_dump( const char *name ) {
  if( __output == "" ){
    fprintf( stderr, "ERROR: --dump action require --output option to be set.\n\n" );
    help( name );
  }

  Tracer tracer( __process );
  tracer.dumpRegion( __address, __output.c_str() );
}

void action_inject( const char *name ) {
  Tracer tracer( __process );

  const Symbols *syms = tracer.getSymbols();

  uintptr_t pstr = tracer.writeString( __library.c_str() );

  printf( "Library name string allocated @ %p\n", pstr );

  uintptr_t ret = tracer.call( syms->_dlopen, 2, pstr, 0 );

  printf( "dlopen returned 0x%x\n", ret );

  tracer.call( syms->_free, 1, pstr );
}
