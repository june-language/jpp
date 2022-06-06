add_subdirectory(Common)
add_subdirectory(VM)
add_subdirectory(Core)

newJuneTarget(
  june

  BINARY
  main.cpp

  LINK_LIBS JuneCommon JuneVM
)
