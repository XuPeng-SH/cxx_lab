#pragma once
#include <tuple>
#include <type_traits>
#include <iostream>
#include <utility>

template <typename...> using void_t = void;

template <typename Pred>
using enable_spec_if = void_t<std::enable_if<Pred::value, void>>;

template <typename>
struct always_false : std::false_type {};
