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
