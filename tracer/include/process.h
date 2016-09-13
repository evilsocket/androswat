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

  inline pid_t pid() const {
    return _pid;
  }

  inline const vector<MemoryMap>& memory() const {
    return _memory;
  }
};

#endif
