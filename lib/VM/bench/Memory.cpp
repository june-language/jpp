#include "VM/Memory.hpp"
#include <nanobench.h>

u32 BenchPoolAllocation() {
  june::mem::PoolAllocator alloc;
  alloc.free(alloc.allocate(1024), 1024);
  return 0;
}

i32 main() {
  auto allocBench = ankerl::nanobench::Bench().minEpochIterations(109).run(
      "PoolAllocation", BenchPoolAllocation);
}
