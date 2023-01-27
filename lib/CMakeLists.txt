add_subdirectory(Common)

newJuneTarget(
  june

  BINARY
  main.cpp

  LINK_LIBS
  Common
)
