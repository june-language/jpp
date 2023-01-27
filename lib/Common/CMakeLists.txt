newJuneTarget(
  Common

  STATIC
  Err.cpp
  String.cpp
  Fs.cpp
  Env.cpp
)

newJuneTest(
  CommonTests
  AGAINST Common

  SOURCES
    tests/ErrTest.cpp
)
