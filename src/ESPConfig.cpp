#include "ESPConfig.hpp"

#include <algorithm>

ESPConfig::ESPConfig()
    : m_fileSys{nullptr},
      m_configFileList{{}},
      m_useEeprom{true},
      m_mountCB{[](fileSystem_t fileSys) {}},
      m_unmountCB{[](fileSystem_t fileSys) {}} {
  read();
}

ESPConfig::ESPConfig(const char* configFileName, fileSystem_t fileSys,
                     const mountCallBack_t mountCB,
                     const mountCallBack_t unmountCB,
                     const bool useEeprom)
    : m_fileSys{fileSys},
      m_configFileList{{configFileName}},
      m_useEeprom{useEeprom},
      m_mountCB{mountCB},
      m_unmountCB{unmountCB} {
  read();
}

ESPConfig::ESPConfig(const std::vector<const char*> configFileList,
                     fileSystem_t fileSys, const mountCallBack_t mountCB,
                     const mountCallBack_t unmountCB, const bool useEeprom)
    : m_fileSys{fileSys},
      m_configFileList{configFileList},
      m_useEeprom{useEeprom},
      m_mountCB{mountCB},
      m_unmountCB{unmountCB} {
  read();
}

ESPConfig::ESPConfig(JsonObjectConst json)
    : m_fileSys{nullptr},
      m_configFileList{{}},
      m_useEeprom{false} {
  readJson(json);
}

ESPConfig::~ESPConfig() {
  for (auto key : keys()) {
    if (is<ESPConfigP_t>(key)) {
      remove(key);
    }
  }
}

ESPConfig& ESPConfig::remove(const char* key) {
  if (is<ESPConfigP_t>(key)) {
#if __has_include(<variant>)
    delete std::get<ESPConfigP_t>(m_config.at(key));
#else
    delete linb::any_cast<ESPConfigP_t>(m_config.at(key));
#endif
  }
  m_config.erase(key);
  return *this;
}

ESPConfig& ESPConfig::reset() {
  std::for_each(keys().begin(), keys().end(),
    [this](const char* key) { remove(key); });
  return *this;
}

const std::vector<const char*> ESPConfig::keys() const {
  std::vector<const char*> key{};
  std::for_each(m_config.begin(), m_config.end(),
      [&key](const std::pair<std::string, configValue_t>& c){
        key.push_back(c.first.c_str());
      });
  return key;
}

ESPConfig& ESPConfig::read() {
  read("");

  DynamicJsonDocument json { m_jsonDocSize };
  EEPROM.begin(m_eepromSize);
  EepromStream eepromStream(0, m_eepromSize);
  auto error { deserializeMsgPack(json, eepromStream) };
  eepromStream.flush();
  EEPROM.end();

  if (!error && json[F("Saved")].as<bool>()) {
    readJson(json.as<JsonObject>());
  }

  return *this;
}

ESPConfig& ESPConfig::read(const char* jsonStr) {
  return read(jsonStr, strlen(jsonStr));
}

ESPConfig& ESPConfig::read(const char* jsonStr, size_t jsonStrLen) {
  DynamicJsonDocument json { m_jsonDocSize };
  //read configuration from FS json
  if (m_fileSys) {
    m_mountCB(m_fileSys);
    std::for_each(m_configFileList.rbegin(), m_configFileList.rend(),
      [&json, this](const char* fileName) {
        auto configFile = m_fileSys->open(fileName, "r");
        if (configFile) {
          DeserializationError error = deserializeJson(json, configFile);
          configFile.close();
          if (error) {
            Serial.printf_P(
              PSTR("ESPConfig read error: config file serializeJson() failed: %s\n"),
              error.c_str());
            return;
          }
        } else {
          Serial.printf_P(
            PSTR("ESPConfig read warning: unable to open config file '%s' for read\n"),
            fileName);
          return;
        }
        readJson(json.as<JsonObject>());
      });
    m_unmountCB(m_fileSys);
  }

  if (jsonStrLen != 0) {
    auto error { deserializeJson(json, jsonStr, jsonStrLen) };
    if (!error) {
      readJson(json.as<JsonObject>());
    }
  }

  return *this;
}

void ESPConfig::readJson(JsonObjectConst json){
  for (auto kv : json) {
    if (kv.value().is<bool>()) {
      value(kv.key().c_str(), kv.value().as<bool>());
      continue;
    }

    if (kv.value().is<int32_t>()) {
      value(kv.key().c_str(), kv.value().as<int32_t>());
      continue;
    }

    if (kv.value().is<double>()) {
      value(kv.key().c_str(), kv.value().as<double>());
      continue;
    }

    if (kv.value().is<const char*>()) {
      value(kv.key().c_str(), kv.value().as<const char*>());
      continue;
    }

    if (kv.value().is<JsonObjectConst>()) {
      value(kv.key().c_str(), new ESPConfig{kv.value().as<JsonObjectConst>()});
      continue;
    }

    if (kv.value().is<JsonArrayConst>() &&
        kv.value().as<JsonArrayConst>().size() != 0) {
      auto arr { kv.value().as<JsonArrayConst>() };

      if (arr[0].is<bool>()) {
        value(kv.key().c_str(), std::vector<bool>{});
        value<std::vector<bool>>(kv.key().c_str()).reserve(arr.size());
        for (auto val : arr) {
          value<std::vector<bool>>(kv.key().c_str()).push_back(val.as<bool>());
        }
        continue;
      }

      if (arr[0].is<int32_t>()) {
        value(kv.key().c_str(), std::vector<int32_t>{});
        value<std::vector<int32_t>>(kv.key().c_str()).reserve(arr.size());
        for (auto val : arr) {
          value<std::vector<int32_t>>(kv.key().c_str()).push_back(val.as<int32_t>());
        }
        continue;
      }

      if (arr[0].is<double>()) {
        value(kv.key().c_str(), std::vector<double>{});
        value<std::vector<double>>(kv.key().c_str()).reserve(arr.size());
        for (auto val : arr) {
          value<std::vector<double>>(kv.key().c_str()).push_back(val.as<double>());
        }
        continue;
      }

      if (arr[0].is<const char*>()) {
        value(kv.key().c_str(), std::vector<std::string>{});
        value<std::vector<std::string>>(kv.key().c_str()).reserve(arr.size());
        for (auto val : arr) {
          value<std::vector<std::string>>(kv.key().c_str()).push_back(val.as<const char*>());
        }
        continue;
      }

      if (arr[0].is<JsonObjectConst>()) {
        value(kv.key().c_str(), std::vector<ESPConfigP_t>{});
        value<std::vector<ESPConfigP_t>>(kv.key().c_str()).reserve(arr.size());
        for (auto val : arr) {
          value<std::vector<ESPConfigP_t>>(kv.key().c_str())
            .push_back(new ESPConfig{val.as<JsonObjectConst>()});
        }
        continue;
      }
    }
  }
}

std::string ESPConfig::toJSON(ESPConfig::saveFormat format) const {
  std::string output;

  switch (format) {
    case saveFormat::minified:
      serializeJson(toJSONObj(), output);
      break;
    case saveFormat::pretty:
      serializeJsonPretty(toJSONObj(), output);
      break;
    case saveFormat::msgPack:
      serializeMsgPack(toJSONObj(), output);
      break;
    default:
      break;
  }

  return output;
}

DynamicJsonDocument ESPConfig::toJSONObj() const {
  DynamicJsonDocument json{m_jsonDocSize};

  json[F("Saved")] = true;

  for (auto key : keys()) {
  #if __has_include(<variant>)
    auto index { m_config.at(key).index() };
  #else
    auto index{std::distance(
      anyIndex.begin(),
      std::find(anyIndex.begin(), anyIndex.end(),
                std::type_index(m_config.at(key).type()))
    )};
  #endif
    // the order must match the configValue_t variant definition
    switch (index) {
      case 0:  // bool
        json[key] = value<bool>(key);
        break;
      case 1:  // int32_t
        json[key] = value<int32_t>(key);
        break;
      case 2:  // double
        json[key] = value<double>(key);
        break;
      case 3:  // std::string
        json[key] = value<const char*>(key);
        break;
      case 4:  // ESPConfig_t
        json[key] = value<ESPConfigP_t>(key)->toJSON().c_str();
        break;
      default: {
        auto arr{json.createNestedArray(key)};
        switch (index) {
          case 5:   // std::vector<bool>
            std::for_each(value<std::vector<bool>>(key).begin(),
                          value<std::vector<bool>>(key).end(),
                          [&arr](const bool val){ arr.add(val); });
            break;
          case 6:   // std::vector<int32_t>
            std::for_each(value<std::vector<int32_t>>(key).begin(),
                          value<std::vector<int32_t>>(key).end(),
                          [&arr](const int32_t val){ arr.add(val); });
            break;
          case 7:   // std::vector<double>
            std::for_each(value<std::vector<double>>(key).begin(),
                          value<std::vector<double>>(key).end(),
                          [&arr](const double val){ arr.add(val); });
            break;
          case 8:   // std::vector<std::string>
            std::for_each(value<std::vector<std::string>>(key).begin(),
                          value<std::vector<std::string>>(key).end(),
                          [&arr](const std::string& val){ arr.add(val.c_str()); });
            break;
          case 9:   // std::vector<ESPConfig_t>
            std::for_each(value<std::vector<ESPConfigP_t>>(key).begin(),
                          value<std::vector<ESPConfigP_t>>(key).end(),
                          [&arr](const ESPConfigP_t val){ arr.add(val->toJSON().c_str()); });
            break;
          default:
            break;
        }
        break;
      }
    }
  }

  return json;
}

void ESPConfig::save() const {
  if (m_useEeprom) {
    auto json{toJSONObj()};
    auto toWrite{measureMsgPack(json)};
    if (toWrite > m_eepromSize) {
      Serial.printf_P(
          PSTR("ESPConfig save error: the config data size %d is greater than "
               "the available EEPROM size %d and the config data was not saved.\n"
               "Please increase the available EEPROM size using the "
               "ESPCONFIG_EEPROMSIZE macro identifier.\n"),
          toWrite, m_eepromSize);
      return;
    }

    EEPROM.begin(m_eepromSize);
    EepromStream eepromStream(0, toWrite);
    serializeMsgPack(json, eepromStream);
    eepromStream.flush();
    EEPROM.end();

    return;
  }

  if (m_fileSys) {
    // write configuration json to FS
    if (m_configFileList.empty()) {
      Serial.printf_P(PSTR("ESPConfig save error: no config file provided\n"));
      return;
    }

    m_mountCB(m_fileSys);
    auto configFile = m_fileSys->open(m_configFileList.at(0), "w");
    if (configFile) {
      auto json{ toJSONObj() };
      auto toWrite { measureJsonPretty(json) };
      auto written { serializeJsonPretty(json, configFile) };
      configFile.close();
      if (written != toWrite) {
        Serial.printf_P(PSTR("ESPConfig save error: file system write failed, %d "
                             "bytes written not %d\n"),
                        written, toWrite);
      }
    } else {
      Serial.printf_P(
          PSTR("ESPConfig save error: unable to open config file '%s' for write\n"),
          m_configFileList[0]);
    }
    m_unmountCB(m_fileSys);
  }

  return;
}
