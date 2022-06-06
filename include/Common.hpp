#ifndef common_env_hpp
#define common_env_hpp

#include "JuneConfig.hpp"
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

// `NullStream` is a stream that doesn't print anything.
class NullStream : public std::ostream {
  NullStream() : std::ostream(nullptr) {}
  ~NullStream() override = default;

public:
  static std::ostream &get() {
    static NullStream instance;
    return instance;
  }
};

/// Logs the current line number and file name, and allows for a message to be
/// logged with additional information:
///
///     DebugLog << "Hello, world!" << std::endl; // Hello, world!
///     DebugLog << "Hello, world" << ": " << 42 << std::endl; // Hello, world:
///     42 DebugLog << std::endl; // This prints only the information needed.
#define DebugLog                                                               \
  (JuneDebug ? std::cerr : NullStream::get())                                  \
      << "[" << june::fs::relativePath(__FILE__) << ":" << __LINE__ + 1        \
      << "] "

/// @brief Common namespace for June.
namespace june {
namespace args {

struct ArgDesc {
  std::string queryName;
  std::string description;
  std::string longName;
  std::string shortName;
  bool hasValue;
  bool shouldNewline; // used internally for help printing
};

struct Arg {
  std::string queryName;
  std::string value;
  bool valueExists;

  Arg(std::string queryName, std::string value)
      : queryName(queryName), value(value), valueExists(!value.empty()) {}
};

extern std::vector<ArgDesc> descriptors;

std::vector<std::string> ArgsGetCodeArgs();
void ArgsAddArgument(std::string queryName, std::string shortName,
                     std::string longName, std::string description,
                     bool hasValue = false, bool shouldNewline = false);
void ArgsParseArguments(int argc, char **argv);
bool ArgsArgumentExists(std::string queryName);
bool ArgsPositionalExists(int position);
Arg ArgsGetArgument(std::string queryName);
Arg ArgsGetPositional(int position);
void ArgsModifyArgument(std::string queryName, std::string newValue);
std::vector<Arg> ArgsGetPositionals();

bool ArgsAnyArgumentExists();

void ArgsPrintHelp(const std::string &argv0);
} // namespace args

namespace err {
/// @brief An error kind
enum ErrKind {
  /// @brief No error.
  ErrNone,
  /// @brief IO error.
  ErrFileIo,
  /// @brief Exec error.
  ErrExecFail,
  /// @brief Invalid command line argument.
  ErrArgsFail,
  /// @brief June code raised an uncaught error.
  ErrRaised,
};

/// @brief Describes an error kind.
const char *errKindAsString(ErrKind kind);

/// @brief An error with a description and kind.
struct Error {
  ErrKind kind;
  std::string desc;
  bool fatal;

  /// @brief Constructs an error with a description and kind.
  Error(ErrKind kind, std::string desc) : kind(kind), desc(desc) {}

  Error(const Error &other) : kind(other.kind), desc(other.desc) {}

  /// @brief Prints the error.
  void print(std::ostream &out, bool printKind = true) const {
    out << "error: " << desc;

    if (kind != ErrNone && printKind) {
      out << " (" << errKindAsString(kind) << ")";
    }

    out << std::endl;

    if (fatal) {
      out << "fatal error: exiting" << std::endl;
      exit(1);
    }
  }

  /// @brief The error as a string.
  std::string toString() const {
    std::stringstream ss;
    print(ss);
    return ss.str();
  }

  /// @brief Acts upon an error, exiting with the error's code
  inline void act() {
    if (fatal)
      std::exit(kind);
  }

  /// @brief Determines if an error is fatal.
  inline bool isFatal() const { return fatal; }

  /// @brief Sets the error's fatal flag.
  inline void setFatal(const bool &fatal) { this->fatal = fatal; }
};

/// @brief A simple void type for returning nothing in `Result<O, E>` functions.
struct VoidType {};

template <typename O, typename E> struct Result {
private:
  bool isError;
  union {
    O ok;
    E err;
  };

public:
  Result(const O ok) : isError(false), ok(ok) {}
  Result(const E err) : isError(true), err(err) {}

  Result(const Result &other) : isError(other.isError) {
    if (isError)
      err = other.err;
    else
      ok = other.ok;
  }

  ~Result() {
    if (isError)
      err.~E();
    else
      ok.~O();
  }

  static Result<VoidType, E> Ok() { return Result<VoidType, E>(VoidType{}); }

  static Result<O, E> Ok(O ok) { return Result<O, E>(ok); }

  static Result<O, E> Err(E err) { return Result<O, E>(err); }

  std::ostream &operator<<(std::ostream &os) {
    if (isError)
      return os << err;
    else
      return os << ok;
  }

  inline bool isOk() const { return !isError; }
  inline bool isErr() const { return isError; }

  inline const O *getOk() const { return isOk() ? &ok : nullptr; }
  inline const E *getErr() const { return isErr() ? &err : nullptr; }

  inline const O &unwrap() const {
    if (isError)
      throw std::runtime_error("Result unwrapped an Err");
    return ok;
  }

  inline const E &unwrapErr() const {
    if (!isError)
      throw std::runtime_error("Result unwrapped an Ok");
    return err;
  }
};

template <typename O, typename E>
std::ostream &operator<<(std::ostream &os, const Result<O, E> &err);

using Errors = err::Result<VoidType, err::Error>;
} // namespace err

namespace string {
/// @brief Checks if a string ends with a certain substring.
bool endsWith(const std::string &str, const std::string &suffix);

/// @brief Splits a string into a vector of strings.
std::vector<std::string> split(const std::string &str,
                               const std::string &delim);

/// @brief Replaces all occurrences of a substring in a string.
std::string replaceAll(const std::string &str, const std::string &from,
                       const std::string &to);

/// @brief Joins a vector of strings into a single string.
std::string join(const std::vector<std::string> &strs,
                 const std::string &delim);

/// @brief Converts a string to lowercase.
std::string toLower(const std::string &str);

/// @brief Converts a string to uppercase.
std::string toUpper(const std::string &str);

/// @brief Duplicates a string into a new C-style string.
const char *duplicateAsCString(const std::string &str);
} // namespace string

namespace fs {
using namespace june::err;

const int kMaxPathChars = 1024;

/// @brief Reads a file into a string.
Result<std::string, Error> readFile(const std::string &path);

/// @brief Checks if a file exists.
Result<bool, Error> exists(const std::string &path);

/// @brief Converts a path into an absolute path.
std::string absPath(const std::string &path, std::string *parentDir = nullptr,
                    const bool &dirAddDoubleDot = false);

/// @brief Determines if a path is absolute.
bool isAbsolute(const std::string &path);

/// @brief Checks if a path is of a directory.
bool isDir(const std::string &path);

/// @brief Gets the directory path of a path.
std::string dirName(const std::string &path);

/// @brief Gets the current working directory.
Result<std::string, Error> cwd();

/// @brief Gets the home directory of the user invoking the process.
Result<std::string, Error> home();

/// @brief Gets the relative path relative to the provided directory, or the
///        current working directory.
std::string relativePath(const std::string &path,
                         const std::string &refDir = "");

/// @brief Searches a directory using the provided matcher.
std::vector<std::string>
search(const std::string &dir,
       const std::function<bool(const std::string &)> &matcher);
} // namespace fs

namespace env {
/// @brief Gets the value of an environment variable.
std::string get(const std::string &key);

/// @brief Gets the path to the current process's executable.
std::string getProcPath();
} // namespace env
} // namespace june

#endif
