#include "memory_map.h"
#include <stdio.h>

MemoryMap::MemoryMap() :
  _begin(0),
  _end(0),
  _size(0),
  _offset(0),
  _inode(0) {

}

void MemoryMap::dump() const {
  printf( "%lx-%lx %s %08lx %s %d %s\n",
    _begin,
    _end,
    _permissions.c_str(),
    _offset,
    _device.c_str(),
    _inode,
    _name.c_str()
  );
}

MemoryMap MemoryMap::parse( const char *buffer ) {
  MemoryMap map;

  char perms[0xF] = {0},
       dev[0xFF] = {0},
       path[0xFF] = {0};

  sscanf( buffer, "%lx-%lx %s %lx %s %u %[^\n]s",
          &map._begin, &map._end,
          perms,
          &map._offset,
          dev,
          &map._inode,
          &path );

  map._permissions = perms;
  map._device      = dev;
  map._name        = path;
  map._size        = map._end - map._begin;

  return map;
}
