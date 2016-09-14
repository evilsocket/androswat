AndroSwat
===

A tool to inspect, dump, modify, search and inject libraries into Android processes, still **work in progress**!

## Usage

    Usage: /data/local/tmp/androswat <options> <action>

    OPTIONS:

      --pid    | -p PID  : Select process by pid.
      --name   | -n NAME : Select process by name.
      --size   | -s SIZE : Set size.
      --output | -o FILE : Set output file.

    ACTIONS:

      --help   | -H         : Show help menu.
      --show   | -S         : Show process informations.
      --search | -X HEX     : Search for the given pattern ( in hex ) in the process address space.
      --read   | -R ADDRESS : Read SIZE bytes from address and prints them, requires -s option.
      --dump   | -D ADDRESS : Dump memory region containing a specific address to a file, requires -o option.
      --inject | -I LIBRARY : Inject the shared LIBRARY into the process.

## License

Released under the BSD license.  
Copyright &copy; 2016, Simone Margaritelli <evilsocket@gmail.com>  
All rights reserved.
