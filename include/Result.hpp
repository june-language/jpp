#ifndef june_result_hpp
#define june_result_hpp

#include "SFINAE.hpp"
#include <optional>
#include <variant>

namespace june {
namespace err {

using namespace june::functional;

template <typename O, typename E>
struct Result {
private:
  bool isError;
  Option<O> ok;
  Option<E> err;

public:
  constexpr Result(const O ok) requires NotVoid<O> : isError(false), ok(ok), err(std::nullopt_t) {}

  template <typename = NotVoid<E>>
  constexpr Result(const E err) : isError(true), ok(std::nullopt_t), err(err) {}

  Result(const Result &other) : isError(other.isError) {
    if (isError) {
      err = other.err;
    } else {
      ok = other.ok;
    }
  }

  template <typename = isVoid<O>> static auto Ok() -> Result<O, E> {
    return Result<O, E>(inner_void_t{});
  }

  template <typename = isVoid<E>> static auto Err() -> Result<O, E> {
    return Result<O, E>(inner_void_t{});
  }

  template <typename = isNotVoid<O>> static auto Ok(O ok) -> Result<O, E> {
    return Result<O, E>(ok);
  }

  template <typename = isNotVoid<E>> static auto Err(E err) -> Result<O, E> {
    return Result<O, E>(err);
  }

  auto operator<<(std::ostream &os) -> std::ostream & {
    if (isError)
      return os << err;
    return os << ok;
  }

  inline auto isOk() const -> bool { return !isError; }
  inline auto isErr() const -> bool { return isError; }

  template <typename = isNotVoid<O>> inline auto unwrap() const -> O {
    if (isError)
      throw std::runtime_error("Called unwrap() on an error Result");
    return ok;
  }

  template <typename = isNotVoid<O>> inline auto unwrapOr(O other) const -> O {
    if (isError)
      return other;
    return ok;
  }

  template <typename = isNotVoid<O>>
  inline auto unwrapOrElse(std::function<O(E)> f) const -> O {
    if (isError)
      return f(err);
    return ok;
  }

  template <typename = isNotVoid<O>>
  inline auto expect(std::string msg) const -> O {
    if (isError)
      throw std::runtime_error(msg);
    return ok;
  }

  template <typename = isNotVoid<O>>
  inline auto mapErr(std::function<E(E)> f) const -> Result<O, E> {
    if (isError)
      return Result<O, E>(f(err));
    return Result<O, E>(ok);
  }

  template <typename = isNotVoid<O>> inline auto getOk() const -> O * {
    if (isError)
      return nullptr;
    return &ok;
  }

  template <typename = isNotVoid<E>> inline auto getErr() const -> E * {
    if (isError)
      return &err;
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
}

using Errors = err::Result<void, std::string>;

} // namespace err
} // namespace june

#endif
