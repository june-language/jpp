#include "Common.hpp"

namespace june {

namespace err {

auto errKindAsString(ErrorKind kind) -> const char * {
  switch (kind) {
  case ErrorKind::None:
    return "none";
  case ErrorKind::FileIo:
    return "file i/o";
  case ErrorKind::Exec:
    return "exec error";
  case ErrorKind::Args:
    return "args error";
  case ErrorKind::Raised:
    return "raised";
  case ErrorKind::Unwrap:
    return "unwrap error";
  }
}

} // namespace err

} // namespace june
