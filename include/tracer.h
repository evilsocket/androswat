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
#ifndef __TRACER_H__
#define __TRACER_H__

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "process.h"

typedef struct _Symbols {
  uintptr_t _dlopen;
  uintptr_t _dlsym;
  uintptr_t _dlerror;
  uintptr_t _calloc;
  uintptr_t _free;

  _Symbols() : _dlopen(0), _dlsym(0), _dlerror(0), _calloc(0), _free(0) {

  }

  inline bool valid() const {
    return ( _dlopen && _dlsym && _dlerror && _calloc && _free );
  }
}
Symbols;

class Tracer {
private:

  Process *_process;
  Symbols  _symbols;

  long trace( int request, void *addr = 0, void *data = 0 );
  bool attach();
  void detach();

public:

  Tracer( Process* process );
  virtual ~Tracer();

  bool dumpRegion( uintptr_t address, const char *output );

  const Symbols *getSymbols();

  bool read( size_t addr, unsigned char *buf, size_t blen );
  bool write( size_t addr, unsigned char *buf, size_t blen);
  uintptr_t writeString( const char *s );
  uintptr_t call( uintptr_t function, int nargs, ... );

};

#endif
