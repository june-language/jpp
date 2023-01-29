add_subdirectory(Common)
add_subdirectory(VM)

newJuneTarget(
  june

  BINARY
  main.cpp

  LINK_LIBS
  Common
  VM
)
