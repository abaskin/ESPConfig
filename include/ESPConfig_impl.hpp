#pragma once

#include <array>

#include "ESPConfig.hpp"

// ---- is ----

template <typename T>
inline bool ESPConfig::is(const char* key) const {
  return m_config.find(key) != m_config.end() &&
      #ifdef ESP32
        m_config.at(key).type() == typeid(T);
      #else
        std::holds_alternative<T>(m_config.at(key));
      #endif
}

template <>
inline bool ESPConfig::is<std::array<double, 2>>(const char* key) const {
  return m_config.find(key) != m_config.end() &&
      #ifdef ESP32
        m_config.at(key).type() == typeid(std::vector<double>) &&
      #else
        std::holds_alternative<std::vector<double>>(m_config.at(key)) &&
      #endif
         value<std::vector<double>>(key).size() == 2;
}

// ---- value setter ----

template <typename T>
inline ESPConfig& ESPConfig::value(const char* key, T value) {
  m_config[key] = (T)value;
  return *this;
}

template <>
inline ESPConfig& ESPConfig::value<const char*>(const char* key, const char* value) {
  m_config[key] = std::string{value};
  return *this;
}

template <>
inline ESPConfig& ESPConfig::value<std::array<double, 2>>(const char* key,
                                                          const std::array<double, 2> value) {
  m_config[key] = std::vector<double>{value[0], value[1]};
  return *this;
}

// ---- value getter ----

template <typename T>
inline T ESPConfig::value(const char* key) const {
#ifdef ESP32
  return (is<T>(key)) ? linb::any_cast<T>(m_config.at(key)) : (T){};
#else
  return (is<T>(key)) ? std::get<T>(m_config.at(key)) : (T){};
#endif
}

template <>
inline const char* ESPConfig::value(const char* key) const {
  return (is<std::string>(key))
  #ifdef ESP32
    ? linb::any_cast<std::string>(m_config.at(key)).c_str()
  #else
    ? std::get<std::string>(m_config.at(key)).c_str()
  #endif
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
