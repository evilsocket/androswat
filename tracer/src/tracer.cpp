#include <sys/types.h>
#include <sys/stat.h>

#include "tracer.h"

long Tracer::trace( int request, void *addr /* = 0 */, size_t data /* = 0 */ ) {
    long ret = ptrace( request, _process->pid(), (caddr_t)addr, (void *)data );
    if( ret == -1 && (errno == EBUSY || errno == EFAULT || errno == ESRCH) ){
        // perror("ptrace");
        return -1;
    }
    return ret;
}

bool Tracer::attach() {
  if( trace( PTRACE_ATTACH ) != -1 ){
    int status;
    waitpid( _process->pid(), &status, 0 );
    return true;
  }
  else {
    return false;
  }
}

void Tracer::detach() {
  trace( PTRACE_DETACH );
}

bool Tracer::read( size_t addr, unsigned char *buf, size_t blen ){
  size_t i = 0;
  long *d, *s, ret;

  d = (long*)buf;
  s = (long*)addr;
  blen /= sizeof(long);

  for( i = 0; i < blen; ++i ) {
    ret = trace( PTRACE_PEEKDATA, s + i );
    if(errno) {
      return false;
    }
    d[i] = ret;
  }

  return true;
}

Tracer::Tracer( Process* process ) : _process(process) {

}

bool Tracer::dumpRegion( uintptr_t address, const char *output ) {
  // search address
  const MemoryMap *mem = _process->findRegion(address);
  if( !mem ){
    fprintf( stderr, "Could not find address 0x%x in any memory region.\n", address );
    return false;
  }
  printf( "Found 0x%08x in %s\n", address, mem->name().c_str() );

  // attach to process
  if( attach() == false ){
    perror("ptrace");
    fprintf( stderr, "Could not attach to process.\n" );
    return false;
  }

  bool ok = false;
  size_t toread = mem->size() - ( address - mem->begin() );
  unsigned char *buffer = new unsigned char[ toread ];
  // read memory region into buffer
  if( read( address, buffer, toread ) ){
    printf( "Dumping %ld bytes to '%s' ...\n", toread, output );
    FILE *fp = fopen( output, "w+b" );
    if( fp ){
      fwrite( buffer, 1, toread, fp );
      fclose(fp);
      // we're running as root, we need to chmod the file in order to pull it.
      chmod( output, 0755 );
      ok = true;
    }
    else {
      perror("fopen");
      fprintf( stderr, "Failed to create dump file.\n" );
    }
  }
  else {
    perror("ptrace");
    fprintf( stderr, "Could not read from process.\n" );
  }

  delete[] buffer;
  detach();
  return ok;
}

Tracer::~Tracer() {
  detach();
}
