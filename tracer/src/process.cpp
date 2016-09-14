#include "process.h"
#include <dirent.h>

inline bool is_numeric( const char *s ){
  for( const char *p = s; *p; p++ ){
    if( *p < '0' || *p > '9' ){
      return false;
    }
  }
  return true;
}

Process *Process::find( const char *name ) {
  DIR *dir = NULL;
  struct dirent *ent = NULL;

  dir = opendir("/proc/");
  if( !dir ){
    perror("opendir");
    FATAL( "Could not open /proc/.\n" );
  }

  while( (ent = readdir(dir)) != NULL ) {
    if( is_numeric( ent->d_name ) ){
      pid_t pid = strtoul( ent->d_name, NULL, 10 );
      string proc_name = Process::parseName(pid);
      if( proc_name == name ){
        return new Process(pid);
      }
    }
  }
  closedir(dir);

  FATAL( "Could not find process '%s'.\n", name );
  return NULL;
}

string Process::parseName(pid_t pid) {
  char procfile[0xFF] = {0},
       buffer[4096] = {0};
  FILE *fp = NULL;

  sprintf( procfile, "/proc/%u/cmdline", pid );
  fp = fopen( procfile, "rt" );
  if( fp == NULL ){
    FATAL( "Could not find pid %u.\n", pid );
  }

  size_t read = fread( buffer, 1, sizeof(buffer), fp );
  if( read == 0 && !feof(fp) ){
    fclose(fp);
    FATAL( "Error reading %s.\n", procfile );
  }

  fclose(fp);

  return buffer;
}

vector<MemoryMap> Process::parseMaps(pid_t pid) {
  vector<MemoryMap> memory;
  char procfile[0xFF] = {0},
       buffer[4096] = {0};
  FILE *fp = NULL;

  sprintf( procfile, "/proc/%u/maps", pid );
  fp = fopen( procfile, "rt" );
  if( fp == NULL ){
    FATAL( "Could not find %s.\n", procfile );
  }

  while( fgets( buffer, sizeof(buffer), fp ) ) {
    // trim line
    char *p = strrchr( buffer, '\n' );
    if( p ){
      *p = 0x00;
    }

    memory.push_back( MemoryMap::parse(buffer) );
  }
  fclose(fp);

  return memory;
}

Process::Process( pid_t pid ) : _pid(pid) {
  _name   = parseName(_pid);
  _memory = parseMaps(_pid);
}

void Process::dump() const {
  printf( "PROC ID   : %u\n", _pid );
  printf( "PROC NAME : %s\n", _name.c_str() );
  printf( "MEMORY    :\n\n" );
  PROCESS_FOREACH_MAP_CONST(this){
    i->dump();
  }
}

const MemoryMap *Process::findRegion( uintptr_t address ) {
  PROCESS_FOREACH_MAP_CONST(this){
    if( i->contains(address) ){
      return &(*i);
    }
  }
  return NULL;
}

uintptr_t Process::findLibrary( const char *name ) {
  PROCESS_FOREACH_MAP_CONST(this){
    if( i->isExecutable() && i->name().find(name) != string::npos ){
      // printf( "%s found in %s\n", name, i->name().c_str() );
      //i->dump();
      return i->begin();
    }
  }
  return 0;
}

uintptr_t Process::findSymbol( uintptr_t local ) {
  // we need an instance for the local process to get the library name
  // given the symbol address
  Process local_p(getpid());

  printf( "Searching symbol %p\n", local );

  const MemoryMap *local_mem = local_p.findRegion(local);
  if(!local_mem){
    fprintf( stderr, "Could not find symbol locally.\n" );
    return 0;
  }

  printf( "Function found in %s %p\n", local_mem->name().c_str(), local_mem->begin() );

  string library_name = local_mem->name();
  uintptr_t library_handle = findLibrary( library_name.c_str() );
  if(!library_handle){
    fprintf( stderr, "Could not find library %s.\n", library_name.c_str() );
    return 0;
  }

  printf( "Found library %p\n", library_handle );

  // Compute the delta of the local and the remote modules and apply it to
  // the local address of the symbol ... BOOM, remote symbol address!
  uintptr_t symbol = local + library_handle - local_mem->begin();

  printf( "Found symbol %p\n", symbol );

  return symbol;
}
