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
#ifndef __MEMORY_MAP_H__
#define __MEMORY_MAP_H__

#include <stdint.h>
#include <string>

using std::string;

class MemoryMap {
private:

  uintptr_t _begin;
  uintptr_t _end;
  size_t    _size;
  string    _permissions;
  uintptr_t _offset;
  string    _device;
  size_t    _inode;
  string    _name;

public:

  MemoryMap();

  void dump() const;

  static MemoryMap parse( const char *line );

  inline bool isExecutable() const {
      return _permissions.find("x") != string::npos;
  }

  inline bool contains( uintptr_t address ) const {
    return address >= _begin && address < _end;
  }

  inline uintptr_t begin() const {
    return _begin;
  }

  inline uintptr_t end() const {
    return _end;
  }

  inline size_t size() const {
    return _size;
  }

  inline string permissions() const {
    return _permissions;
  }

  inline uintptr_t offset() const {
    return _offset;
  }

  inline string device() const {
    return _device;
  }

  inline size_t inode() const {
    return _inode;
  }

  inline string name() const {
    return _name;
  }
};

#endif
