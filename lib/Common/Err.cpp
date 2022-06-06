#include "Common.hpp"

const char *june::err::errKindAsString(ErrKind kind) {
  switch (kind) {
  case ErrNone:
    return "No error";
  case ErrFileIo:
    return "File I/O error";
  case ErrExecFail:
    return "Execution error";
  case ErrArgsFail:
    return "Invalid command line arguments";
  case ErrRaised:
    return "June code raised an uncaught error";
  default:
    return "Unknown error";
  }
}
