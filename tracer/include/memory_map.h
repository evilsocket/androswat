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
