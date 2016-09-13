#ifndef __TRACER_H__
#define __TRACER_H__

#include <sys/ptrace.h>
#include <sys/wait.h>

#include "process.h"

class Tracer {
private:

  Process *_process;

  long trace( int request, void *addr = 0, size_t data = 0 );
  bool attach();
  void detach();
  bool read( size_t addr, unsigned char *buf, size_t blen );

public:

  Tracer( Process* process );
  virtual ~Tracer();

  bool dumpRegion( uintptr_t address, const char *output );
};

#endif
