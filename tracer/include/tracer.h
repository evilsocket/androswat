#ifndef __TRACER_H__
#define __TRACER_H__

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "process.h"

class Tracer {
private:

  Process *_process;

  long trace( int request, void *addr = 0, void *data = 0 );
  bool attach();
  void detach();

public:

  Tracer( Process* process );
  virtual ~Tracer();

  bool dumpRegion( uintptr_t address, const char *output );

  bool read( size_t addr, unsigned char *buf, size_t blen );
  bool write( size_t addr, unsigned char *buf, size_t blen);
  uintptr_t call( uintptr_t function, int nargs, ... );

};

#endif
