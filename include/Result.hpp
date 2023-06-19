#ifndef june_result_hpp
#define june_result_hpp

#include "Option.hpp"
#include "SFINAE.hpp"
#include <exception>
#include <optional>
#include <ostream>
#include <string>
#include <variant>

namespace june {
namespace err {

// Due to not being able to use `void` as a template parameter (see std::variant),
// we use std::monostate as a placeholder for void. This type alias `null` is used
// to make the code more readable and to make it easier to change the type if
// std::monostate is no longer needed.
using null = std::monostate;

using namespace june::functional;

template <NotVoid O, NotVoid E>
struct Result {
private:
  bool isError;
  Option<O> ok;
  Option<E> err;

public:
  constexpr Result(const O ok) : isError(false), ok(ok), err() {}

  constexpr Result(const E err) : isError(true), ok(), err(err) {}

  Result(const Result &other) : isError(other.isError) {
    if (isError) {
      err = other.err;
    } else {
      ok = other.ok;
    }
  }
 
  static auto Ok() -> Result<O, E> {
    return Result<O, E>(null{});
  }

  static auto Err() -> Result<O, E> {
    return Result<O, E>(null{});
  }

  static auto Ok(O ok) -> Result<O, E> requires NotMonoState<O> {
    return Result<O, E>(ok);
  }

  static auto Err(E err) -> Result<O, E> requires NotMonoState<E> {
    return Result<O, E>(err);
  }

  auto operator<<(std::ostream &os) -> std::ostream & {
    if (isError)
      return os << err;
    return os << ok;
  }

  inline auto isOk() const -> bool { return !isError; }
  inline auto isErr() const -> bool { return isError; }

  inline auto unwrap() const -> O {
    if (isError)
      throw std::runtime_error("Called unwrap() on an error Result");
    return ok.unwrap();
  }

  inline auto unwrapErr() const -> E {
    if (!isError)
      throw std::runtime_error("Called unwrapErr() on an ok Result");
    return err.unwrap();
  }

  inline auto unwrapOr(O other) const -> O requires NotMonoState<O> {
    if (isError)
      return other;
    return ok.unwrap();
  }

  inline auto unwrapOrElse(std::function<O(E)> f) const -> O requires (NotMonoState<O> && NotMonoState<E>) {
    if (isError)
      return f(err.unwrap());
    return ok.unwrap();
  }

  inline auto expect(std::string msg) const -> O requires NotMonoState<O> {
    if (isError)
      throw std::runtime_error(msg);
    return ok.unwrap();
  }

  inline auto mapErr(std::function<E(E)> f) const -> Result<O, E> requires NotMonoState<E> {
    if (isError)
      return Result<O, E>(f(err));
    return Result<O, E>(ok);
  }

  inline auto getOk() const -> O * requires NotMonoState<O> {
    if (isError)
      return nullptr;
    return ok.get();
  }

  inline auto getErr() const -> E * requires NotMonoState<E> {
    if (isError)
      return err.get();
    return nullptr;
  }

  auto operator==(const Result &other) const -> bool {
    if (isError != other.isError)
      return false;
    if (isError)
      return err == other.err;
    return ok == other.ok;
  }

  auto operator!=(const Result &other) const -> bool {
    return !(*this == other);
  }
};

using Errors = err::Result<null, std::string>;

} // namespace err
} // namespace june

#endif
