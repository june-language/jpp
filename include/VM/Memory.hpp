#ifndef vm_memory_hpp
#define vm_memory_hpp

#include <cstddef>
#include <list>
#include <map>
#include <vector>

namespace june {

typedef unsigned char u8;

struct __sys_align_t {
  char c;
  size_t sz;
};

static constexpr size_t kPoolSize = 4 * 1024;
static constexpr size_t kAlignment = sizeof(__sys_align_t) - sizeof(size_t);

struct MemoryPool {
  u8 *head;
  u8 *mem;
};

class MemoryManager {
  std::vector<MemoryPool> pools;
  std::map<size_t, std::list<u8 *>> free_chunks;

  void allocPool();

public:
  MemoryManager();
  ~MemoryManager();

  static MemoryManager &instance();

  void *alloc(size_t sz);
  void free(void *ptr, size_t sz);
};

namespace mem {

size_t mult8_roundup(size_t sz);

inline void *alloc(size_t sz) { return MemoryManager::instance().alloc(sz); }
inline void free(void *ptr, size_t sz) {
  return MemoryManager::instance().free(ptr, sz);
}

} // namespace mem
} // namespace june

#endif
