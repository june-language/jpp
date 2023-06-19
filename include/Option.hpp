#ifndef june_option_hpp
#define june_option_hpp

#include "SFINAE.hpp"
#include <optional>
#include <variant>
#include <exception>
#include <ostream>
#include <string>

namespace june {
namespace functional {

template <NotVoid T>
class Option {
  bool _isSome;

  template <NotMonoState T>
  using Variant = std::variant<std::monostate, T>;

  template <MonoState T>
  using Variant = std::variant<std::monostate>;

  Variant<T> value;

public:
  constexpr Option() : _isSome(false), value(std::monostate{}) {}
  constexpr Option(T value_) : _isSome(true), value(value_) {}

  auto operator=(const Option &other) -> Option & {
    _isSome = other._isSome;
    value = other.value;
    return *this;
  }

  auto operator=(Option &&other) -> Option & {
    _isSome = other._isSome;
    value = other.value;
    return *this;
  }

  static auto Some() -> Option<std::monostate> { return Option<std::monostate>::Some(); }
  static auto Some(T value) -> Option<T> { return Option<T>::Some(value); }
  static auto None() -> Option<T> { return Option<T>::None(); }

  auto operator<<(std::ostream &os) -> std::ostream & {
    if (_isSome) {
      os << "Some(" << std::get<T>(value) << ")";
    } else {
      os << "None";
    }
  }

  auto operator==(const Option &other) -> bool {
    if (_isSome && other._isSome) {
      return value == other.value;
    } else if (!_isSome && !other._isSome) {
      return true;
    } else {
      return false;
    }
  }

  auto operator!=(const Option &other) -> bool {
    return !(*this == other);
  }

  inline auto isSome() const -> bool { return _isSome; }
  inline auto isNone() const -> bool { return !_isSome; }

  inline auto get() const -> T * {
    if (_isSome) {
      return &std::get<T>(value);
    } else {
      return nullptr;
    }
  }

  inline auto unwrap() const -> T {
    if (_isSome) {
      return std::get<T>(value);
    } else {
      throw std::runtime_error("Cannot unwrap None");
    }
  }

  inline auto unwrapOr(T other) const -> T {
    if (_isSome) {
      return std::get<T>(value);
    } else {
      return other;
    }
  }

  inline auto unwrapOrElse(std::function<T()> other) const -> T {
    if (_isSome) {
      return std::get<T>(value);
    } else {
      return other();
    }
  }

  inline auto map(std::function<T(T)> f) const -> Option<T> {
    if (_isSome) {
      return Option<T>::Some(f(std::get<T>(value)));
    } else {
      return Option<T>::None();
    }
  }

  inline auto mapOr(T other, std::function<T(T)> f) const -> T {
    if (_isSome) {
      return f(std::get<T>(value));
    } else {
      return other;
    }
  }

  inline auto mapOrElse(std::function<T()> other,
                        std::function<T(T)> f) const -> T {
    if (_isSome) {
      return f(std::get<T>(value));
    } else {
      return other();
    }
  }

  inline auto andThen(std::function<Option<T>(T)> f) const -> Option<T> {
    if (_isSome) {
      return f(std::get<T>(value));
    } else {
      return Option<T>::None();
    }
  }

  inline auto filter(std::function<bool(T)> f) const -> Option<T> {
    if (_isSome) {
      if (f(value)) {
        return Option<T>::Some(std::get<T>(value));
      } else {
        return Option<T>::None();
      }
    } else {
      return Option<T>::None();
    }
  }

  inline auto expect(const std::string &msg) const -> T {
    if (_isSome) {
      return std::get<T>(value);
    } else {
      throw std::runtime_error(msg);
    }
  }
};

}
}

#endif
