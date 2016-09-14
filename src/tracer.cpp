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
#include <sys/types.h>
#include <sys/stat.h>

#include "tracer.h"

#define CPSR_T_MASK ( 1u << 5 )

long Tracer::trace( int request, void *addr /* = 0 */, void *data /* = 0 */ ) {
  long ret = ptrace( request, _process->pid(), (caddr_t)addr, data );
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
    perror("PTRACE_ATTACH");
    return false;
  }
}

void Tracer::detach() {
  trace( PTRACE_DETACH );
}

bool Tracer::read( size_t addr, unsigned char *buf, size_t blen ) {
  size_t i = 0;
  long *d, ret;

  d = (long*)buf;
  blen /= sizeof(long);

  for( i = 0; i < blen; ++i, addr += sizeof(long) ) {
    ret = trace( PTRACE_PEEKDATA, (void *)addr );
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
    ret = trace( PTRACE_POKETEXT, (void *)(addr + i), (void *)*(size_t *)&ptr[i] );
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
  if( trace( PTRACE_GETREGS, 0, &regs ) < 0 ){
    perror("PTRACE_GETREGS 1");
    return -1;
  }

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
  if( trace( PTRACE_SETREGS, 0, &regs ) < 0 ){
    perror("PTRACE_SETREGS");
    return -1;
  }

  if( trace( PTRACE_CONT ) < 0 ){
    perror("PTRACE_CONT");
    return -1;
  }

  waitpid( _process->pid(), NULL, WUNTRACED );

  // get registers again, R0 holds the return value
  if( trace( PTRACE_GETREGS, 0, &regs ) < 0 ){
    perror("PTRACE_GETREGS 2");
    return -1;
  }

  // restore original registers state
  if( trace( PTRACE_SETREGS, 0, &rbackup ) < 0 ){
    perror("PTRACE_SETREGS");
    return -1;
  }

  return regs.ARM_r0;
}

Tracer::Tracer( Process* process ) : _process(process) {
  // attach to process
  if( attach() == false ){
    perror("ptrace");
    FATAL( "Could not attach to process.\n" );
  }
}

const Symbols *Tracer::getSymbols() {
  if( _symbols.valid() == false ){
    _symbols._dlopen  = _process->findSymbol((uintptr_t)::dlopen);
    _symbols._dlsym   = _process->findSymbol((uintptr_t)::dlsym);
    _symbols._dlerror = _process->findSymbol((uintptr_t)::dlerror);
    _symbols._calloc  = _process->findSymbol((uintptr_t)::calloc);
    _symbols._free    = _process->findSymbol((uintptr_t)::free);

    if( _symbols.valid() == false ){
      FATAL( "Could not resolve process symbols.\n" );
    }
  }

  return &_symbols;
}

uintptr_t Tracer::writeString( const char *s ) {
  getSymbols();

  uintptr_t mem = call( _symbols._calloc, 2, strlen(s) + 1, 1 );

  write( mem, (unsigned char *)s, strlen(s) + 1 );

  return mem;
}

bool Tracer::dumpRegion( uintptr_t address, const char *output ) {
  // search address
  const MemoryMap *mem = _process->findRegion(address);
  if( !mem ){
    fprintf( stderr, "Could not find address 0x%x in any memory region.\n", address );
    return false;
  }
  printf( "Found 0x%08x in %s\n", address, mem->name().c_str() );

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
  return ok;
}

Tracer::~Tracer() {
  detach();
}
