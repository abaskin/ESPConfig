#pragma once

#include <array>

#include "ESPConfig.hpp"

// ---- is ----

template <typename T>
inline bool ESPConfig::is(const char* key) const {
  return m_config.find(key) != m_config.end() &&
         std::holds_alternative<T>(m_config.at(key));
}

template <>
inline bool ESPConfig::is<std::array<double, 2>>(const char* key) const {
  return m_config.find(key) != m_config.end() &&
         std::holds_alternative<std::vector<double>>(m_config.at(key)) &&
         value<std::vector<double>>(key).size() == 2;
}

// ---- value setter ----

template <typename T>
inline void ESPConfig::value(const char* key, T value) {
  m_config[key] = (T)value;
}

template <>
inline void ESPConfig::value<const char*>(const char* key, const char* value) {
  m_config[key] = std::string{value};
}

template <>
inline void ESPConfig::value<std::array<double, 2>>(const char* key,
                                                    const std::array<double, 2> value) {
  m_config[key] = std::vector<double>{value[0], value[1]};
}

// ---- value getter ----

template <typename T>
inline T ESPConfig::value(const char* key) const {
  return (is<T>(key)) ? std::get<T>(m_config.at(key)) : (T){};
}

template <>
inline const char* ESPConfig::value(const char* key) const {
  return (is<std::string>(key))
    ? std::get<std::string>(m_config.at(key)).c_str()
    : "";
}

template <>
inline std::array<double, 2> ESPConfig::value(const char* key) const {
  return (is<std::array<double, 2>>(key))
    ? std::array<double, 2>{
        value<std::vector<double>>(key)[0],
        value<std::vector<double>>(key)[1],
      }
    : std::array<double, 2>{};
}