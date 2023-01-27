#ifndef common_hpp
#define common_hpp

#include "JuneConfig.hpp"
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "_defines.h"

/// `NullStream` is a dummy stream that does nothing.
class NullStream : public std::ostream {
  NullStream() : std::ostream(nullptr) {}
  ~NullStream() override = default;

public:
  static auto get() -> std::ostream & {
    static NullStream instance;
    return instance;
  }
};

/// Logs the current line number and file name as well as
/// the function name and a given message to a stream
#define DebugLog                                                               \
  (JuneDebug ? std::cerr : NullStream::get())                                  \
      << "[" << june::fs::relativePath(__FILE__) << ":" << __LINE__ << "@"     \
      << __funcname__ << "] "

/// @brief Common namespace for June
namespace june {

namespace sfinae {

/// @brief Checks if a type is equatable to itself (has == and != operators)
template <typename T> struct isEquatable {
  template <typename U> static auto test(U *) -> decltype(std::declval<U>() ==
                                                         std::declval<U>(),
                                                         std::declval<U>() !=
                                                             std::declval<U>(),
                                                         std::true_type());

  template <typename> static auto test(...) -> std::false_type;

  static constexpr bool value = decltype(test<T>(nullptr))::value;
};

}

namespace err {
/// @brief Error codes
enum class ErrorKind {
  /// @brief No error
  None,
  /// @brief An error occurred while doing file I/O
  FileIo,
  /// @brief An error occurred during execution
  Exec,
  /// @brief An error occurred while parsing arguments
  Args,
  /// @brief An error was raised by the user
  Raised
};

/// @brief Describes an error kidn
auto errKindAsString(ErrorKind kind) -> const char *;

/// @brief An error with a description and kind
struct Error {
  ErrorKind kind;
  std::string desc;
  bool fatal;

  /// @brief Constructs an error with a description and kind
  Error(ErrorKind kind, std::string desc, bool fatal = false)
      : kind(kind), desc(desc), fatal(fatal) {}

  Error(const Error &other)
      : kind(other.kind), desc(other.desc), fatal(other.fatal) {}

  /// @brief Prints the error
  auto print(std::ostream &out, bool printKind = true) const -> void {
    out << "err: " << desc;

    if (kind != ErrorKind::None && printKind) {
      out << " (" << errKindAsString(kind) << ")";
    }

    out << std::endl;

    if (fatal) {
      out << "fatal error: exiting" << std::endl;
      exit(1);
    }
  }

  /// @brief The error as a string
  auto toString() const -> std::string {
    std::stringstream ss;
    print(ss);
    return ss.str();
  }

  /// @brief Sets an error as fatal
  auto setFatal(bool isFatal) -> void { fatal = isFatal; }

  auto operator==(const Error &other) const -> bool {
    return kind == other.kind && desc == other.desc && fatal == other.fatal;
  }
};

template <typename O, typename E,
          typename = std::enable_if_t<sfinae::isEquatable<O>::value &&
                                      sfinae::isEquatable<E>::value>>
struct Result {
private:
  bool isError;
  union {
    O ok;
    E err;
  };

public:
  // allows for implicit conversion from O to Result<O, E>
  constexpr Result(const O ok) : isError(false), ok(ok) {}
  // allows for implicit conversion from E to Result<O, E>
  constexpr Result(const E err) : isError(true), err(err) {}

  Result(const Result &other) : isError(other.isError) {
    if (isError) {
      err = other.err;
    } else {
      ok = other.ok;
    }
  }

  ~Result() {
    if (isError) {
      err.~E();
    } else {
      ok.~O();
    }
  }

  static auto Ok() -> Result<void, E> { return Result<void, E>({}); }
  static auto Ok(O ok) -> Result<O, E> { return Result<O, E>(ok); }
  static auto Err(E err) -> Result<O, E> { return Result<O, E>(err); }

  auto operator<<(std::ostream &os) -> std::ostream & {
    if (isError)
      return os << err;
    else
      return os << ok;
  }

  inline auto isOk() -> bool const { return !isError; }
  inline auto isErr() -> bool const { return isError; }

  inline auto getOk() -> O *const { return isError ? nullptr : &ok; }
  inline auto getErr() -> E *const { return isError ? &err : nullptr; }

  inline auto unwrap() -> O const {
    if (isError)
      throw std::runtime_error("called `Result::unwrap()` on an `Err` value");
    return ok;
  }

  inline auto unwrapErr() -> E const {
    if (!isError)
      throw std::runtime_error("called `Result::unwrapErr()` on an `Ok` value");
    return err;
  }

  auto unwrapOr(O other) -> O {
    if (isError)
      return other;
    return ok;
  }

  auto unwrapOrElse(std::function<O(E &)> f) -> O {
    if (isError)
      return f(err);
    return ok;
  }

  auto operator==(const Result &other) const -> bool {
    if (isError != other.isError)
      return false;

    if (isError)
      return err == other.err;
    else
      return ok == other.ok;
  }
};

template <typename O, typename E>
auto operator<<(std::ostream &os, const Result<O, E> &result) -> std::ostream &;

using Errors = err::Result<void, err::Error>;
} // namespace err

namespace string {

/// @brief Checks if a string ends with a certain substring
auto endsWith(const std::string &str, const std::string &suffix) -> bool;

/// @brief Splits a string into a vector of strings
auto split(const std::string &str, const std::string &delim)
    -> std::vector<std::string>;

/// @brief Replaces all occurrences of a substring in a string
auto replaceAll(const std::string &str, const std::string &from,
                const std::string &to) -> std::string;

/// @brief Joins a vector of strings into a single string
auto join(const std::vector<std::string> &strs, const std::string &delim)
    -> std::string;

/// @brief Converts a string to lowercase
auto toLower(const std::string &str) -> std::string;

/// @brief Converts a string to uppercase
auto toUpper(const std::string &str) -> std::string;

/// @brief Converts a string to titlecase
auto toTitle(const std::string &str) -> std::string;

/// @brief Converts a string to C-style string by copying it
auto duplicateAsCString(const std::string &str) -> const char *;

} // namespace string

namespace fs {

using namespace june::err;

const int kMaxPathChars = 1024;

/// @brief Reads a file into a string
auto readFile(const std::string &path) -> Result<std::string, Error>;

/// @brief Checks if a file exists
auto exists(const std::string &path) -> bool;

/// @brief Converts a path into an absolute path
auto absolutePath(const std::string &path, std::string *parent = nullptr)
    -> std::string;

/// @brief Determines if a path is absolute
auto isAbsolute(const std::string &path) -> bool;

/// @brief Determines if a path is a directory
auto isDir(const std::string &path) -> bool;

/// @brief Determines if a path is a file
auto isFile(const std::string &path) -> bool;

/// @brief Gets the file name of path
auto filename(const std::string &path) -> std::string;

/// @brief Gets the current working directory
auto cwd() -> std::string;

/// @brief Gets the home directory of the user invoking the program
auto home() -> std::string;

/// @brief Gets the relative path to the provided dir (or cwd)
auto relativePath(const std::string &path, const std::string &dir = cwd())
    -> std::string;

/// @brief Searches a directory using the provided matcher
auto search(const std::string &dir,
            const std::function<bool(const std::string &)> &matcher)
    -> std::vector<std::string>;
} // namespace fs

namespace env {

/// @brief Gets the value of an environment variable
auto get(const std::string &key) -> std::string;

/// @brief Gets the path to the current process's executable
auto getExecutablePath() -> std::string;

} // namespace env

} // namespace june

#endif
