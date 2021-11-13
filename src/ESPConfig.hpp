#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>  // https://github.com/bblanchon/
#include <FS.h>

#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>

constexpr auto m_eepromSize{1024};
constexpr auto m_jsonDocSize{1024};

class ESPConfig {
  public:
    ESPConfig(fs::FS& fileSys, const char* configFileName,
              bool useEeprom = true);
    ESPConfig(fs::FS& fileSys, JsonObjectConst json);
    ~ESPConfig();
    void read();
    void read(const char* jsonStr, size_t jsonStrLen);
    void save() const;
    void remove(const char* key);
    void reset();
    template <typename T> bool is(const char* key) const;
    template <typename T> void value(const char* key, T value);
    template <typename T> T value(const char* key) const;
    const std::vector<const char*> keys() const;
    std::string toJSON(bool pretty = true) const;

   private:
    using ESPConfigP_t = ESPConfig*;
    using configValue_t = std::variant<bool, double, std::string, ESPConfigP_t,
                                       std::vector<bool>, std::vector<double>,
                                       std::vector<std::string>,
                                       std::vector<ESPConfigP_t>>;
    using configMap_t = std::map<std::string, configValue_t>;

    void readJson(JsonObjectConst json);

    configMap_t m_config;

    fs::FS& m_fileSys;
    const std::string m_configFileName;
    const bool m_useEeprom;
};

#include "ESPConfig_impl.hpp"
