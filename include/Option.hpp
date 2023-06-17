#ifndef option_hpp
#define option_hpp

#include "SFINAE.hpp"
#include <optional>
#include <variant>

namespace june {
namespace functional {

template <NotVoid T>
class Option<T> {
  bool isSome;
  std::variant<T> value;

public:
  constexpr Option() : isSome(false), value() {}
  constexpr Option(T value) : isSome(true), value(value) {}

  auto operator=(const Option &other) -> Option & {
    isSome = other.isSome;
    value = other.value;
    return *this;
  }

  auto operator=(Option &&other) -> Option & {
    isSome = other.isSome;
    value = other.value;
    return *this;
  }

  static auto Some() -> Option<std::nullopt_t> { return Option<std::nullopt_t>::Some(); }
  static auto Some(T value) -> Option<T> { return Option<T>::Some(value); }
  static auto None() -> Option<T> { return Option<T>::None(); }

  auto operator<<(std::ostream &os) -> std::ostream & {
    if (isSome) {
      os << "Some(" << value << ")";
    } else {
      os << "None";
    }
  }

  auto operator==(const Option &other) -> bool {
    if (isSome && other.isSome) {
      return value == other.value;
    } else if (!isSome && !other.isSome) {
      return true;
    } else {
      return false;
    }
  }

  auto operator!=(const Option &other) -> bool {
    return !(*this == other);
  }

  inline auto isSome() const -> bool { return isSome; }
  inline auto isNone() const -> bool { return !isSome; }

  inline auto unwrap() const -> T {
    if (isSome) {
      return value;
    } else {
      throw std::runtime_error("Cannot unwrap None");
    }
  }

  inline auto unwrapOr(T other) const -> T {
    if (isSome) {
      return value;
    } else {
      return other;
    }
  }

  inline auto unwrapOrElse(std::function<T()> other) const -> T {
    if (isSome) {
      return value;
    } else {
      return other();
    }
  }

  inline auto map(std::function<T(T)> f) const -> Option<T> {
    if (isSome) {
      return Option<T>::Some(f(value));
    } else {
      return Option<T>::None();
    }
  }

  inline auto mapOr(T other, std::function<T(T)> f) const -> T {
    if (isSome) {
      return f(value);
    } else {
      return other;
    }
  }

  inline auto mapOrElse(std::function<T()> other,
                        std::function<T(T)> f) const -> T {
    if (isSome) {
      return f(value);
    } else {
      return other();
    }
  }

  inline auto andThen(std::function<Option<T>(T)> f) const -> Option<T> {
    if (isSome) {
      return f(value);
    } else {
      return Option<T>::None();
    }
  }

  inline auto filter(std::function<bool(T)> f) const -> Option<T> {
    if (isSome) {
      if (f(value)) {
        return Option<T>::Some(value);
      } else {
        return Option<T>::None();
      }
    } else {
      return Option<T>::None();
    }
  }

  inline auto expect(const std::string &msg) const -> T {
    if (isSome) {
      return value;
    } else {
      throw std::runtime_error(msg);
    }
  }
};

}
}

#endif
