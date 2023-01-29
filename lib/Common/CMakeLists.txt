newJuneTarget(
  Common

  STATIC
  Err.cpp
  String.cpp
  Fs.cpp
  Env.cpp
  Debug.cpp
)

newJuneTest(
  CommonTests
  AGAINST Common

  SOURCES
    tests/ErrTest.cpp
    tests/StringTest.cpp
    # TODO: tests/FsTest.cpp
    # TODO: tests/EnvTest.cpp
    # TODO: tests/DebugTest.cpp
)
