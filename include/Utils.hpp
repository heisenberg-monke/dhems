#pragma once

#include <nlohmann/json.hpp>

template <typename T>
concept Enum = std::is_enum_v<T>;

template <Enum E>
inline std::string toString(E e) {
    return nlohmann::json(e).get<std::string>();
}

template <Enum E>
inline E fromString(const std::string &s) {
    return nlohmann::json(s).get<E>();
}