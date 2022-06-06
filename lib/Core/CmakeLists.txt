newJuneTarget(
  Core


  SHARED
  LIBRARY_INSTALL_DIR "June" # {prefix}/lib/june/...
  Core.cpp

  LINK_LIBS JuneVM JuneCommon
)
