#include <stdio.h>
#include <getopt.h>

#include "tracer.h"

typedef enum {
  ACTION_HELP = 0,
  ACTION_SHOW = 1,
  ACTION_DUMP = 2
}
action_t;

static struct option options[] = {
  { "pid",    required_argument, 0, 'p' },
  { "name",   required_argument, 0, 'n' },
  { "output", required_argument, 0, 'o' },

  { "help", no_argument,       0, 'h' },
  { "show", no_argument,       0, 's' },
  { "dump", required_argument, 0, 'd' },
  {0,0,0,0}
};

static pid_t     __pid     = -1;
static string    __name    = "";
static action_t  __action  = ACTION_HELP;
static Process  *__process = NULL;
static string    __output  = "";
static uintptr_t __address = -1;

void help( const char *name ){
  printf( "Usage: %s <options> <action>\n\n", name );
  printf( "OPTIONS:\n\n" );
  printf( "  --pid    | -p PID  : Select process by pid.\n" );
  printf( "  --name   | -n NAME : Select process by name.\n" );
  printf( "  --output | -o FILE : Set output file.\n" );

  printf( "\nACTIONS:\n\n" );
  printf( "  --help | -h         : Show help menu.\n" );
  printf( "  --show | -s         : Show process informations.\n" );
  printf( "  --dump | -d ADDRESS : Dump memory region containing a specific address, requires -o option.\n" );
  exit(0);
}

void check_and_init( const char *name ) {
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

int main( int argc, char **argv )
{
  int c, option_index = 0;
  while (1) {
    c = getopt_long( argc, argv, "o:p:n:hsd:", options, &option_index );
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
        __action = ACTION_SHOW;
      break;

      case 'd':
        __action  = ACTION_DUMP;
        __address = strtol( optarg, NULL, 16 );
      break;

      case 'h':
        help( argv[0] );
      break;

      default:
        help( argv[0] );
    }
  }

  if( __action == ACTION_HELP ){
    help( argv[0] );
  }

  check_and_init( argv[0] );

  if( __action == ACTION_SHOW ){
    __process->dump();
  }
  else if( __action == ACTION_DUMP ){
    if( __output == "" ){
      fprintf( stderr, "ERROR: --dump action require --output option to be set.\n\n" );
      help( argv[0] );
    }

    Tracer tracer( __process );

    //tracer.dumpRegion( __address, __output.c_str() );
    uintptr_t pexit = __process->findSymbol( (uintptr_t)::exit );
    tracer.call( pexit, 1, 1 );
  }

  delete __process;

  return 0;
}
