#include "VM/Memory.hpp"
#include "JuneConfig.hpp"
#include "c/Memory.h"
#include <mutex>

static std::mutex MemLock;

namespace june {
namespace mem {
size_t mult8_roundup(size_t sz) { return (sz > 512) ? sz : (sz + 7) & ~7; }
} // namespace mem

#if JuneMemDebug == true
static size_t totalAlloc = 0;
static size_t totalAllocNoPool = 0;
static size_t totalAllocRequested = 0;
static size_t totalManuallyAlloc = 0;
#endif

void MemoryManager::allocPool() {
  u8 *alloc = new u8[kPoolSize];
#if JuneMemDebug
  totalAlloc += kPoolSize;
#endif
  this->pools.push_back({alloc, alloc});
}

MemoryManager::MemoryManager() { allocPool(); }
MemoryManager::~MemoryManager() {
  for (auto &c : free_chunks) {
    if (c.first > kPoolSize) {
      for (auto &blk : c.second) {
        delete[] blk;
      }
    }
    c.second.clear();
  }
  free_chunks.clear();

  for (auto &p : pools)
    delete[] p.mem;

#if JuneMemDebug == true
  fprintf(stdout,
          "Total allocated: %zu bytes, without mempool: %zu, requests: %zu, "
          "manually allocated: %zu bytes\n",
          totalAlloc, totalAllocNoPool, totalAllocRequested,
          totalManuallyAlloc);
#endif
}

MemoryManager &MemoryManager::instance() {
  static MemoryManager mem;
  return mem;
}

void *MemoryManager::alloc(size_t sz) {
  if (sz == 0)
    return nullptr;
  std::lock_guard<std::mutex> lock(MemLock);

#if JuneMemDebug == true
  totalAllocNoPool += sz;
  ++totalAllocRequested;
#endif

  sz = mem::mult8_roundup(sz);

  if (sz > kPoolSize) {
#if JuneMemDebug == true
    fprintf(stdout, "Allocating manually ... %zu bytes\n", sz);
    totalManuallyAlloc += sz;
#endif
    return new u8[sz];
  }

  if (free_chunks[sz].size() == 0) {
    for (auto &p : pools) {
      size_t free_space = kPoolSize - (p.head - p.mem);
      if (free_space >= sz) {
        u8 *loc = p.head;
        p.head += sz;
#if JuneMemDebug == true
        fprintf(stdout, "Allocating from pool ... %zu bytes\n", sz);
#endif
        return loc;
      }
    }
    allocPool();
    auto &p = pools.back();
    u8 *loc = p.head;
    p.head += sz;
#if JuneMemDebug == true
    fprintf(stdout, "Allocating from NEW pool ... %zu bytes\n", sz);
#endif
    return loc;
  }

  u8 *loc = free_chunks[sz].front();
  free_chunks[sz].pop_front();
#if JuneMemDebug == true
  fprintf(stdout, "Using previously allocated ... %zu bytes\n", sz);
#endif
  return loc;
}

void MemoryManager::free(void *ptr, size_t sz) {
  if (ptr == nullptr || sz == 0)
    return;
  std::lock_guard<std::mutex> lock(MemLock);

  if (sz > kPoolSize) {
#if JuneMemDebug == true
    fprintf(stdout, "Deleting manually ... %zu bytes\n", sz);
#endif
    delete[](u8 *) ptr;
    return;
  }
#if JuneMemDebug == true
  fprintf(stdout, "Giving back to pool ... %zu bytes\n", sz);
#endif
  free_chunks[sz].push_front((u8 *)ptr);
}

} // namespace june

// C API

size_t JuneMemMult8Roundup(size_t sz) { return june::mem::mult8_roundup(sz); }

void *JuneMemAlloc(size_t sz) { return june::mem::alloc(sz); }

void JuneMemFree(void *ptr, size_t sz) { june::mem::free(ptr, sz); }
