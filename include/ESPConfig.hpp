#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>  // https://github.com/bblanchon/
#include <FS.h>

#include <functional>
#include <map>
#include <string>
#include <vector>

#ifdef ESP32
#include "any.hpp"
#else // ESP8266
#include <variant>
#endif

constexpr auto m_eepromSize{1024};
constexpr auto m_jsonDocSize{1024};

class ESPConfig {
  public:
    ESPConfig(fs::FS& fileSys, const char* configFileName,
              bool useEeprom = true);
    ESPConfig(fs::FS& fileSys, JsonObjectConst json);
    ~ESPConfig();
    ESPConfig& read();
    ESPConfig& read(const char* jsonStr, size_t jsonStrLen);
    ESPConfig& remove(const char* key);
    ESPConfig& reset();
    void save() const;
    template <typename T> bool is(const char* key) const;
    template <typename T> ESPConfig& value(const char* key, T value);
    template <typename T> T value(const char* key) const;
    const std::vector<const char*> keys() const;
    std::string toJSON(bool pretty = true) const;

   private:
    using ESPConfigP_t = ESPConfig*;
  #ifdef ESP32
    using configValue_t = linb::any;
  #else
    using configValue_t = std::variant<bool, double, std::string, ESPConfigP_t,
                                       std::vector<bool>, std::vector<double>,
                                       std::vector<std::string>,
                                       std::vector<ESPConfigP_t>>;
  #endif
    using configMap_t = std::map<std::string, configValue_t>;

    void readJson(JsonObjectConst json);

    configMap_t m_config;

    fs::FS& m_fileSys;
    const std::string m_configFileName;
    const bool m_useEeprom;
};

#include "ESPConfig_impl.hpp"
