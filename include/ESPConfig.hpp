#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>  // https://github.com/bblanchon/
#include <FS.h>

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// ESP32 does not support std::varient at this time
#if __has_include(<variant>)
# include <variant>
#else
# include <typeindex>
# include <typeinfo> 
# include "any.hpp"
#endif

#ifndef ESPCONFIG_EEPROMSIZE
# define ESPCONFIG_EEPROMSIZE 1024u
#endif

#ifndef ESPCONFIG_JSONDOCSIZE
# define ESPCONFIG_JSONDOCSIZE 1024u
#endif

constexpr auto m_eepromSize{ESPCONFIG_EEPROMSIZE};
constexpr auto m_jsonDocSize{ESPCONFIG_JSONDOCSIZE};

class ESPConfig {
  public:
    using ESPConfigP_t = ESPConfig*;
    using mountCallBack_t = std::function<void()>;
    using fileSystem_t = fs::FS;
    using fileSystemP_t = fileSystem_t*;
    enum class saveFormat: uint8_t {
      minified,
      pretty,
      msgPack
    };

    ESPConfig();

    ESPConfig(const char* configFileName, 
              fileSystemP_t fileSys,
              mountCallBack_t mountCB = [](){},
              mountCallBack_t unmountCB = [](){},
              const bool useEeprom = true);

    ESPConfig(const std::vector<const char*> configFileList,
              fileSystemP_t fileSys,
              mountCallBack_t mountCB = [](){},
              mountCallBack_t unmountCB = [](){},
              const bool useEeprom = true);

    ESPConfig(JsonObjectConst json);
    
    ~ESPConfig();
    ESPConfig& read();
    ESPConfig& read(const char* jsonStr);
    ESPConfig& read(const char* jsonStr, size_t jsonStrLen);
    ESPConfig& remove(const char* key);
    ESPConfig& reset();
    void save() const;
    template <typename T> bool is(const char* key) const;
    template <typename T> ESPConfig& value(const char* key, T value);
    template <typename T> T value(const char* key) const;
    const std::vector<const char*> keys() const;
    std::string toJSON(saveFormat format = saveFormat::minified) const;

   private:
  #if __has_include(<variant>)
    using configValue_t = std::variant<bool,
                                       int32_t,
                                       double,
                                       std::string,
                                       ESPConfigP_t,
                                       std::vector<bool>,
                                       std::vector<int32_t>,
                                       std::vector<double>,
                                       std::vector<std::string>,
                                       std::vector<ESPConfigP_t>>;
  #else
   const std::array<std::type_index, 10> anyIndex{{
       std::type_index(typeid(bool)),
       std::type_index(typeid(int32_t)),
       std::type_index(typeid(double)),
       std::type_index(typeid(std::string)),
       std::type_index(typeid(ESPConfigP_t)),
       std::type_index(typeid(std::vector<bool>)),
       std::type_index(typeid(std::vector<int32_t>)),
       std::type_index(typeid(std::vector<double>)),
       std::type_index(typeid(std::vector<std::string>)),
       std::type_index(typeid(std::vector<ESPConfigP_t>)),
   }};
   using configValue_t = linb::any;
  #endif

    void readJson(JsonObjectConst json);
    DynamicJsonDocument toJSONObj() const;

    std::unordered_map<std::string, configValue_t> m_config;

    std::unique_ptr<fileSystem_t> m_fileSys;
    const std::vector<const char*> m_configFileList;
    const bool m_useEeprom;
    const mountCallBack_t m_mountCB;
    const mountCallBack_t m_unmountCB;
};

#include "ESPConfig_impl.hpp"
