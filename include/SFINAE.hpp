#ifndef june_sfinae_hpp
#define june_sfinae_hpp

#include <functional>
#include <type_traits>
#include <variant>

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
concept Nullopt = requires() {
  { std::is_same<T, std::monostate>::value } -> std::convertible_to<bool>;
};

template <typename T>
concept NotNullopt = requires() {
  { !std::is_same<T, std::monostate>::value } -> std::convertible_to<bool>;
};

template <typename T>
concept Void = requires() {
  { std::is_void<T>::value }
  ->std::convertible_to<bool>;
  { std::is_same<std::is_void<T>, std::true_type>::value }
  ->std::convertible_to<bool>;
};

template <typename T>
concept NotVoid = requires() {
  { !std::is_void<T>::value }
  ->std::convertible_to<bool>;
  { std::is_same<std::is_void<T>, std::false_type>::value }
  ->std::convertible_to<bool>;
};

#endif
