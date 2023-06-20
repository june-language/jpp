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

template <typename T>
requires NotVoid<T> && NotNullopt<T>
class Option {
  std::optional<T> _value;
public:
  constexpr Option() : _value(std::nullopt) {}
  constexpr Option(T value) : _value(value) {}

  Option(T value, bool isSome) : _value(value) {}
  Option(const Option &other) : _value(other._value) {}

  auto operator=(const Option &other) -> Option & {
    _value = other._value;
    return *this;
  }

  auto operator=(Option &&other) -> Option & {
    _value = other._value;
    return *this;
  }

  static auto Some() -> Option<T> { return Option<T>(std::nullopt); }
  static auto Some(T value) -> Option<T> { return Option<T>::Some(value); }
  static auto None() -> Option<T> { return Option<T>(std::nullopt); }

  auto operator<<(std::ostream &os) -> std::ostream & {
    if (_value.has_value()) {
      os << "Some(" << _value.value() << ")";
    } else {
      os << "None";
    }
  }

  auto operator==(const Option &other) -> bool {
    return _value == other._value;
  }

  auto operator!=(const Option &other) -> bool {
    return !(*this == other);
  }

  inline auto isSome() const -> bool { return _value.has_value(); }
  inline auto isNone() const -> bool { return !_value.has_value(); }

  inline auto get() const -> T * {
    if (_value.has_value()) {
      return &_value.value();
    } else {
      return nullptr;
    }
  }

  inline auto unwrap() const -> T {
    if (_value.has_value()) {
      return _value.value();
    } else {
      throw std::runtime_error("Cannot unwrap None");
    }
  }

  inline auto unwrapOr(T other) const -> T {
    if (_value.has_value()) {
      return _value.value();
    } else {
      return other;
    }
  }

  inline auto unwrapOrElse(std::function<T()> other) const -> T {
    if (_value.has_value()) {
      return _value.value();
    } else {
      return other();
    }
  }

  inline auto map(std::function<T(T)> f) const -> Option<T> {
    if (_value.has_value()) {
      return Option<T>::Some(f(_value.value()));
    } else {
      return Option<T>::None();
    }
  }

  inline auto mapOr(T other, std::function<T(T)> f) const -> T {
    if (_value.has_value()) {
      return f(_value.value());
    } else {
      return other;
    }
  }

  inline auto mapOrElse(std::function<T()> other,
                        std::function<T(T)> f) const -> T {
    if (_value.has_value()) {
      return f(_value.value());
    } else {
      return other();
    }
  }

  inline auto andThen(std::function<Option<T>(T)> f) const -> Option<T> {
    if (_value.has_value()) {
      return f(_value.value());
    } else {
      return Option<T>::None();
    }
  }

  inline auto filter(std::function<bool(T)> f) const -> Option<T> {
    if (_value.has_value()) {
      if (f(_value)) {
        return Option<T>::Some(_value.value());
      } else {
        return Option<T>::None();
      }
    } else {
      return Option<T>::None();
    }
  }

  inline auto expect(const std::string &msg) const -> T {
    if (_value.has_value()) {
      return _value.value();
    } else {
      throw std::runtime_error(msg);
    }
  }
};

}
}

#endif
