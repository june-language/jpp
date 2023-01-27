#include "Common.hpp"

namespace june {

namespace err {

const char *errKindAsString(ErrorKind kind) {
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
  }
}

template<typename O, typename E>
std::ostream &operator<<(std::ostream &out, const Result<O, E> &result) {
  if (result.isOk()) {
    out << "Ok(" << result.unwrap() << ")";
  } else {
    out << "Err(" << result.unwrapErr() << ")";
  }

  return out;
}

} // namespace err

} // namespace june
