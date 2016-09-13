#include <sys/types.h>
#include <sys/stat.h>

#include "tracer.h"

#define CPSR_T_MASK ( 1u << 5 )

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

bool Tracer::read( size_t addr, unsigned char *buf, size_t blen ) {
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

bool Tracer::write( size_t addr, unsigned char *buf, size_t blen) {
  size_t i = 0;
  long ret;

  // make sure the buffer is word aligned
  char *ptr = (char *)calloc(blen + blen % sizeof(size_t), 1);
  memcpy(ptr, buf, blen);

  for( i = 0; i < blen; i += sizeof(size_t) ){
    ret = trace( PTRACE_POKETEXT, (void *)(addr + i), *(size_t *)&ptr[i] );
    if( ret == -1 ) {
      ::free(ptr);
      return false;
    }
  }

  ::free(ptr);

  return true;
}

uintptr_t Tracer::call( uintptr_t function, int nargs, ... ) {
  int i = 0;
  struct pt_regs regs = {{0}}, rbackup = {{0}};

  // get registers and backup them
  trace( PTRACE_GETREGS, 0, (size_t)&regs );
  memcpy( &rbackup, &regs, sizeof(struct pt_regs) );

  va_list vl;
  va_start(vl,nargs);

  for( i = 0; i < nargs; ++i ){
    uintptr_t arg = va_arg( vl, uintptr_t );

    // fill R0-R3 with the first 4 arguments
    if( i < 4 ){
      regs.uregs[i] = arg;
    }
    // push remaining params onto stack
    else {
      regs.ARM_sp -= sizeof(uintptr_t) ;
      write( (size_t)regs.ARM_sp, (uint8_t *)&arg, sizeof(uintptr_t) );
    }
  }

  va_end(vl);

  regs.ARM_lr = 0;
  regs.ARM_pc = function;
  // setup the current processor status register
  if ( regs.ARM_pc & 1 ){
    /* thumb */
    regs.ARM_pc   &= (~1u);
    regs.ARM_cpsr |= CPSR_T_MASK;
  }
  else{
    /* arm */
    regs.ARM_cpsr &= ~CPSR_T_MASK;
  }

  // do the call
  trace( PTRACE_SETREGS, 0, (size_t)&regs );
  trace( PTRACE_CONT );
  waitpid( _process->pid(), NULL, WUNTRACED );

  // get registers again, R0 holds the return value
  trace( PTRACE_GETREGS, 0, (size_t)&regs );

  // restore original registers state
  trace( PTRACE_SETREGS, 0, (size_t)&rbackup );

  return regs.ARM_r0;
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
