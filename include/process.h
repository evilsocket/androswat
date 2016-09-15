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
#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <unistd.h>
#include <vector>

#include "common.h"
#include "memory_map.h"

using std::vector;

#define PROCESS_FOREACH_MAP_CONST(INSTANCE) \
  for( vector<MemoryMap>::const_iterator i = (INSTANCE)->memory().begin(), e = (INSTANCE)->memory().end(); i != e; ++i )

#define PROCESS_FOREACH_MAP(INSTANCE) \
  for( vector<MemoryMap>::iterator i = (INSTANCE)->memory().begin(), e = (INSTANCE)->memory().end(); i != e; ++i )

class Process {
private:

  pid_t             _pid;
  string            _name;
  vector<MemoryMap> _memory;

  static string parseName(pid_t pid);
  static vector<MemoryMap> parseMaps(pid_t pid);

public:

  Process( pid_t pid );
  void dump() const;

  const MemoryMap *findRegion( uintptr_t address );
  uintptr_t findLibrary( const char *name );
  uintptr_t findSymbol( uintptr_t local );

  static Process *find( const char *name );

  inline const string& name() const {
    return _name;
  }
  
  inline pid_t pid() const {
    return _pid;
  }

  inline const vector<MemoryMap>& memory() const {
    return _memory;
  }
};

#endif
