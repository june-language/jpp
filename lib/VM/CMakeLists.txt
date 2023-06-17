newJuneTarget(
  VM
  
  STATIC
  
  Memory.cpp
  
  OpCodes.cpp
  Dylib.cpp

  Stack.cpp

  SrcFile.cpp
  
  # Splits Vars.cpp into multiple files
  # to reduce compile time
  Frame.cpp
  VarStack.cpp
  Vars.cpp

  FailStack.cpp

  Vars/Any.cpp
  Vars/Base.cpp
  Vars/Bool.cpp
  Vars/Float.cpp
  Vars/Func.cpp
  Vars/Int.cpp
  Vars/Nil.cpp
  Vars/Src.cpp
  Vars/String.cpp
  Vars/Vec.cpp

  LINK_LIBS
  Common
)

newJuneTest(
  bench_vm
  AGAINST VM

  BENCH

  SOURCES
    bench/Memory.cpp
)
