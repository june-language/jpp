#ifndef vm_memory_hpp
#define vm_memory_hpp

#include "../Common.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <list>
#include <map>
#include <mutex>

namespace june {

namespace mem {

struct __sys_align_t {
  u8 c;
  u64 sz;
};

static auto mult8RoundUp(const u64 size) -> const u64 {
  return (size + 7) & ~7;
}

// Generic Allocator

class Allocator {
protected:
  u64 _totalAlloc;
  u64 _totalAllocNoPool;
  u64 _totalAllocRequested;
  u64 _totalManuallyAlloc;

  std::mutex _memLock;

public:
  Allocator() = default;
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

  virtual auto allocate(const u64 size, const u64 alignment = 0)
      -> void * = 0;
  virtual auto free(void *ptr, u64 size) -> void = 0;
};

// PoolAllocator

struct MemoryPool {
  u8 *head;
  u8 *mem;
};

class PoolAllocator : public Allocator {
  std::vector<MemoryPool> _pools;
  std::map<u64, std::list<u8 *>> _freeChunks;

  auto allocPool() -> void;

public:
  static constexpr u64 kPoolSize = 4 * 1024;
  static constexpr u64 kAlignment = sizeof(__sys_align_t) - sizeof(u64);

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

  auto allocate(const u64 size, const u64 alignment = 0)
      -> void * override;
  auto free(void *ptr, u64 size) -> void override;
};

// Memory management simplified interface

static Allocator *allocator = new PoolAllocator();

static auto alloc(const u64 size, const u64 alignment = 0)
    -> void * {
  return allocator->allocate(size, alignment);
}

static auto free(void *ptr, u64 size) -> void {
  allocator->free(ptr, size);
}

static auto zero(void *ptr, u64 size) -> void {
  std::memset(ptr, 0, size);
}

} // namespace mem

} // namespace june

#endif
