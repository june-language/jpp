#ifndef vm_memory_hpp
#define vm_memory_hpp

#include "../Common.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <list>
#include <map>

namespace june {

typedef unsigned char u8;

namespace mem {

struct __sys_align_t {
  char c;
  size_t sz;
};

static auto mult8RoundUp(const std::size_t size) -> const std::size_t {
  return (size + 7) & ~7;
}

// MARK: Generic Allocator

class Allocator {
protected:
  size_t _totalAlloc;
  size_t _totalAllocNoPool;
  size_t _totalAllocRequested;
  size_t _totalManuallyAlloc;

  std::mutex _memLock;

public:
  Allocator() {}
  ~Allocator() {
#if JuneMemDebug
    DebugLog << "Total memory allocated: " << _totalAlloc << " bytes"
             << std::endl;
    DebugLog << "Total memory allocated (no pool): " << _totalAllocNoPool
             << " bytes" << std::endl;
    DebugLog << "Total memory requested: " << _totalAllocRequested << " bytes"
             << std::endl;
    DebugLog << "Total memory manually allocated: " << _totalManuallyAlloc
             << " bytes" << std::endl;
#endif
  }

  virtual auto allocate(const std::size_t size, const std::size_t alignment = 0)
      -> void * = 0;
  virtual auto free(void *ptr, std::size_t size) -> void = 0;
};

// MARK: PoolAllocator

struct MemoryPool {
  u8 *head;
  u8 *mem;
};

class PoolAllocator : public Allocator {
  std::vector<MemoryPool> _pools;
  std::map<size_t, std::list<u8 *>> _freeChunks;

  auto allocPool() -> void;

public:
  static constexpr size_t kPoolSize = 4 * 1024;
  static constexpr size_t kAlignment = sizeof(__sys_align_t) - sizeof(size_t);

  PoolAllocator() : Allocator() { allocPool(); }
  ~PoolAllocator() {
    for (auto &c : _freeChunks) {
      if (c.first > kPoolSize) {
        for (auto &blk : c.second) {
          delete[] blk;
        }
      }
      c.second.clear();
    }
    _freeChunks.clear();

    for (auto &p : _pools)
      delete[] p.mem;
  }

  auto allocate(const std::size_t size, const std::size_t alignment = 0)
      -> void * override;
  auto free(void *ptr, std::size_t size) -> void override;
};

// MARK: Memory management simplified interface

static Allocator *allocator = new PoolAllocator();

static auto alloc(const std::size_t size, const std::size_t alignment = 0)
    -> void * {
  return allocator->allocate(size, alignment);
}

static auto free(void *ptr, std::size_t size) -> void {
  allocator->free(ptr, size);
}

static auto zero(void *ptr, std::size_t size) -> void {
  std::memset(ptr, 0, size);
}

} // namespace mem

} // namespace june

#endif
