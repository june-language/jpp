#ifndef june_sfinae_hpp
#define june_sfinae_hpp

#include <functional>
#include <type_traits>

template <typename T>
concept Equatable = requires(T a, T b) {
  { a == b }
  ->std::convertible_to<bool>;
};

template <typename T>
concept Hashable = requires(T a) {
  { std::hash<T>()(a) }
  ->std::convertible_to<std::size_t>;
};

template <typename T>
concept Void = requires() {
  { std::is_void<T>::value }
  ->std::convertible_to<bool>;
};

template <typename T>
concept NotVoid = requires() {
  { std::is_void<T>::value }
  ->std::convertible_to<bool>;
  { !std::is_void<T>::value }
};

#endif
