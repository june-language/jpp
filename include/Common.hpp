#ifndef common_hpp
#define common_hpp

#include "JuneConfig.hpp"
#include "Option.hpp"
#include "Result.hpp"
#include "SFINAE.hpp"
#include <assert.h>
#include <bit>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
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

// Basic typedefs for convenience
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long;
using i8 = char;
using i16 = short;
using i32 = int;
using i64 = long;
using f32 = float;
using f64 = double;

namespace june {

namespace byteorder {

enum class endian {
  little = 0,
  big = 1,
#ifdef JUNE_LE
  native = little,
#else
  native = big,
#endif
};

/// @brief Byte reader for reading bytes from a vector
struct ByteReader {
  u64 pos = 0;
  u64 len = 0;
  std::vector<u8> bytes;

  ByteReader(const std::vector<u8> &bytes) : bytes(bytes), len(bytes.size()) {}

  template <typename T> auto read() -> T {
    static_assert(std::is_integral_v<T>, "T must be an integral type");
    assert(pos + sizeof(T) <= len && "Invalid read");
    T val;
    std::memcpy(&val, bytes.data() + pos, sizeof(T));
    pos += sizeof(T);
    return val;
  }

  auto readString(u64 stringLength) -> const char * {
    assert(pos + stringLength <= len && "Invalid read");
    const char *val = (const char *)(bytes.data() + pos);
    pos += stringLength;
    return val;
  }
};

/// @brief Converts data to a u8 array
template <typename T> auto toBytes(T val) -> std::vector<u8> {
  static_assert(std::is_integral_v<T>, "T must be an integral type");
  std::vector<u8> bytes(sizeof(T));
  std::memcpy(bytes.data(), &val, sizeof(T));
  return bytes;
}

/// @brief Converts a string to a u8 array
static inline auto toBytes(const std::string &str) -> std::vector<u8> {
  std::vector<u8> bytes(str.size());
  std::memcpy(bytes.data(), str.data(), str.size());
  return bytes;
}

/// @brief Converts a u8 array to data
template <typename T> auto fromBytes(const std::vector<u8> &bytes, u64 start = 0, u64 size = 0) -> T {
  if (size == 0)
    size = sizeof(T);
  assert(start + size <= bytes.size() && "Invalid start/size");
  T val;
  std::memcpy(&val, bytes.data() + start, size);
  return val;
}

/// @brief Converts a u8 array to a string
static inline auto fromBytes(const std::vector<u8> &bytes, u64 start = 0, u64 size = 0) -> std::string {
  if (size == 0)
    size = bytes.size() - start;
  assert(start + size <= bytes.size() && "Invalid start/size");
  std::string str(size, '\0');
  std::memcpy(str.data(), bytes.data() + start, size);
  return str;
}

/// @brief Converts a 16-bit integer to/from native to big endian
static inline auto swapBigEndian16(u16 val) -> u16 {
  if constexpr (endian::native == endian::big)
    return val;
  return JUNE_BSWAP16(val);
}

/// @brief Converts a 32-bit integer to/from native to big endian
static inline auto swapBigEndian32(u32 val) -> u32 {
  if constexpr (endian::native == endian::big)
    return val;
  return JUNE_BSWAP32(val);
}

/// @brief Converts a 64-bit integer to/from native to big endian
static inline auto swapBigEndian64(u64 val) -> u64 {
  if constexpr (endian::native == endian::big)
    return val;
  return JUNE_BSWAP64(val);
}

/// @brief Converts a 64-bit signed integer to/from native to big endian
static inline auto swapBigEndian64Signed(i64 val) -> i64 {
  if constexpr (endian::native == endian::big)
    return val;
  return JUNE_BSWAP64(val);
}

/// @brief Converts a 32-bit float to/from native to big endian
static inline auto swapBigEndian32F(f32 val) -> f64 {
  if constexpr (endian::native == endian::big)
    return val;
  return JUNE_BSWAP32F(val);
}

/// @brief Converts a 64-bit float to/from native to big endian
static inline auto swapBigEndian64F(f64 val) -> f64 {
  if constexpr (endian::native == endian::big)
    return val;
  return JUNE_BSWAP64F(val);
}

} // namespace byteorder

namespace fs {

// Forward declares `june::fs::relativePath` and `june::fs::cwd` for the
// DebugLog macro

/// @brief Gets the current working directory
auto cwd() -> std::string;

/// @brief Gets the relative path to the provided dir (or cwd)
auto relativePath(const std::string &path, const std::string &dir = cwd())
    -> std::string;

} // namespace fs
} // namespace june

/// Logs the current line number and file name as well as
/// the function name and a given message to a stream
#define DebugLog                                                               \
  (JuneDebug ? std::cerr : NullStream::get())                                  \
      << "[" << june::fs::relativePath(__FILE__) << ":" << __LINE__ << "@"     \
      << __funcname__ << "] "

/// @brief Common namespace for June
namespace june {

namespace structures {

/// @brief A "bitflags"-like type
template <typename T> class BitFlags {
  u64 flags;

public:
  BitFlags() : flags(0) {}
  BitFlags(u64 flags) : flags(flags) {}
  BitFlags(const BitFlags &other) : flags(other.flags) {}

  auto operator=(const BitFlags &other) -> BitFlags & {
    flags = other.flags;
    return *this;
  }

  auto operator==(const BitFlags &other) const -> bool {
    return flags == other.flags;
  }

  auto operator!=(const BitFlags &other) const -> bool {
    return flags != other.flags;
  }

  auto operator[](T flag) const -> bool {
    return flags & (1 << static_cast<int>(flag));
  }

  // Allows for `flags[Flag::Foo] = true;`
  auto operator[](T flag) -> bool & {
    return *reinterpret_cast<bool *>(reinterpret_cast<u64>(&flags) +
                                     static_cast<int>(flag));
  }

  auto operator~() const -> BitFlags { return BitFlags(~flags); }

  auto set(T flag, bool value = true) -> void {
    if (value)
      flags |= (1 << static_cast<int>(flag));
    else
      flags &= ~(1 << static_cast<int>(flag));
  }

  auto get(T flag) const -> bool {
    return flags & (1 << static_cast<int>(flag));
  }

  auto clear() -> void { flags = 0; }

  auto operator|(T flag) const -> BitFlags { return BitFlags(flags | flag); }

  auto operator&(T flag) const -> BitFlags { return BitFlags(flags & flag); }

  auto operator|=(T flag) -> BitFlags & {
    flags |= flag;
    return *this;
  }

  auto operator&=(T flag) -> BitFlags & {
    flags &= flag;
    return *this;
  }
};

/// @brief An alertantive to std::views::values
template <typename K, typename V>
auto values(const std::unordered_map<K, V> &map) -> std::vector<V> {
  std::vector<V> values;
  values.reserve(map.size());
  for (const auto &[_, value] : map)
    values.push_back(value);
  return values;
}

/// @brief An alternative to std::views::keys
template <typename K, typename V>
auto keys(const std::unordered_map<K, V> &map) -> std::vector<K> {
  std::vector<K> keys;
  keys.reserve(map.size());
  for (const auto &[key, _] : map)
    keys.push_back(key);
  return keys;
}

} // namespace structures

namespace dbg {

/// @brief Demangles a C++ symbol
auto demangle(const u8 *symbol) -> std::string;

/// @brief Demangles a C++ symbol
static auto demangle(const std::string &symbol) -> std::string {
  return demangle(symbol.c_str());
}

/// @brief Gets the name of a type
template <typename T> auto typeName() -> std::string {
  return demangle(typeid(T).name());
}

} // namespace dbg

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
  Raised,
  /// @brief An error occurred while unwrapping on None or Err
  Unwrap,
};

/// @brief Describes an error kidn
auto errKindAsString(ErrorKind kind) -> const i8 *;

/// @brief An error with a description and kind
struct Error {
  ErrorKind kind;
  std::string desc;
  bool fatal;

  /// @brief Constructs an error with a description and kind
  Error(ErrorKind kind, std::string desc, bool fatal = false)
      : kind(kind), desc(std::move(desc)), fatal(fatal) {}

  /// @brief Empty error
  Error() : kind(ErrorKind::None), desc(""), fatal(false) {}

  Error(const Error &other) = default;

  /// @brief Prints the error
  auto print(std::ostream &out, bool printKind = true) const -> void {
    out << "err: " << desc;

    if (kind != ErrorKind::None && printKind) {
      out << " (" << errKindAsString(kind) << ")";
    }

    if (fatal) {
      out << " - fatal error: exiting";
      exit(1);
    }
  }

  /// @brief The error as a string
  auto str() const -> std::string {
    std::stringstream ss;
    print(ss);
    return ss.str();
  }

  /// @brief Sets an error as fatal
  auto setFatal(bool isFatal) -> void { fatal = isFatal; }

  auto operator==(const Error &other) const -> bool {
    return kind == other.kind && desc == other.desc && fatal == other.fatal;
  }

  auto operator!=(const Error &other) const -> bool {
    return !(*this == other);
  }

  friend auto operator<<(std::ostream &out, const Error &err) -> std::ostream & {
    err.print(out);
    return out;
  }
};

using namespace functional;

} // namespace err

namespace string {

/// @brief Checks if a string ends with a certain substring
auto endsWith(const std::string &str, const std::string &suffix) -> bool;

/// @brief Checks if a string starts with a certain substring
auto startsWith(const std::string &str, const std::string &prefix) -> bool;

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

/// @brief Trims whitespace from the beginning and end of a string
auto trim(const std::string &str) -> std::string;

/// @brief Converts a string to C-style string by copying it
auto duplicateAsCString(const std::string &str) -> const i8 *;

} // namespace string

namespace fs {

using namespace june::err;

const int kMaxPathChars = 1024;

/// @brief Reads a file into a string
auto readFile(const std::string &path) -> Result<std::string, Error>;

/// @brief Reads a file into an array of bytes
auto readFileBytes(const std::string &path) -> Result<std::vector<u8>, Error>;

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

/// @brief Gets the home directory of the user invoking the program
auto home() -> std::string;

/// @brief Searches a directory using the provided matcher
auto search(const std::string &dir,
            const std::function<bool(const std::string &)> &matcher)
    -> std::vector<std::string>;

/// @brief Writes a vector of bytes to a file
auto writeBytes(const std::string &path, const std::vector<u8> &bytes, bool createFile = true)
    -> Result<void, Error>;

} // namespace fs

namespace env {

/// @brief Gets the value of an environment variable
auto get(const std::string &key) -> std::string;

/// @brief Gets the path to the current process's executable
auto getExecutablePath() -> std::string;

} // namespace env

} // namespace june

#endif
