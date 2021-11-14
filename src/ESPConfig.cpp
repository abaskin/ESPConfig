#include "ESPConfig.hpp"

#include <ESP_EEPROM.h>

ESPConfig::ESPConfig(fs::FS& fileSys, const char* configFileName,
                     bool useEeprom)
    : m_fileSys(fileSys),
      m_configFileName(configFileName),
      m_useEeprom(useEeprom) {
}

ESPConfig::~ESPConfig(){
  for (auto key : keys()) {
    if (is<ESPConfigP_t>(key)) { remove(key); }
  }
}

ESPConfig::ESPConfig(fs::FS& fileSys, JsonObjectConst json)
    : m_fileSys(fileSys),
      m_configFileName(""),
      m_useEeprom(false) {
  readJson(json);
}

ESPConfig& ESPConfig::remove(const char* key) {
  if (is<ESPConfigP_t>(key)) {
    delete std::get<ESPConfigP_t>(m_config.at(key));
  }
  m_config.erase(key);
  return *this;
}

ESPConfig& ESPConfig::reset() {
  for (auto key : keys()) {
    remove(key);
  }
  return *this;
}

const std::vector<const char*> ESPConfig::keys() const {
  std::vector<const char*> key{};
  for (const auto& c : m_config) {
    key.push_back(c.first.c_str());
  }
  return key;
}

ESPConfig& ESPConfig::read() {
  std::unique_ptr<char[]> configBuff{ new char[m_eepromSize] };
  strcpy_P(configBuff.get(), PSTR("{}"));
  if (m_useEeprom) {    // read from EEPROM
    EEPROM.begin(m_eepromSize);
    EEPROM.get(0, configBuff);
    EEPROM.end();
  }
  read(configBuff.get(), m_eepromSize);
  return *this;
}

ESPConfig& ESPConfig::read(const char* jsonStr, size_t jsonStrLen) {
  DynamicJsonDocument json { m_jsonDocSize };
  auto error = deserializeJson(json, jsonStr, jsonStrLen);
  if (error || json[F("Saved")].as<bool>() == false) {
    //read configuration from FS json
    FSInfo fs_info;
    auto mounted = m_fileSys.info(fs_info);
    if (!mounted) { m_fileSys.begin(); }
    auto configFile = m_fileSys.open(m_configFileName.c_str(), "r");
    if (configFile) {
      DeserializationError error = deserializeJson(json, configFile);
      configFile.close();
      if (!mounted) { m_fileSys.end(); }
      if (error) {
        Serial.printf_P(
            PSTR("ESPConfig error: config file serializeJson() failed: %s\n"),
            error.f_str());
        return *this;
      }
    } else {
      Serial.printf_P(PSTR("ESPConfig warning: unable to open config file '%s' for read\n"),
                      m_configFileName.c_str());
      if (!mounted) { m_fileSys.end(); }
      return *this;
    }
  }

  readJson(json.as<JsonObject>());
  return *this;
}

void ESPConfig::readJson(JsonObjectConst json){
  for (auto kv : json) {
    if (kv.value().is<bool>()) {
      value(kv.key().c_str(), kv.value().as<bool>());
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
      value(kv.key().c_str(), new ESPConfig{m_fileSys, kv.value().as<JsonObjectConst>()});
      continue;
    }

    if (kv.value().is<JsonArrayConst>()) {
      auto arr { kv.value().as<JsonArrayConst>() };

      if (arr[0].is<bool>()) {
        value(kv.key().c_str(), std::vector<bool>{});
        value<std::vector<bool>>(kv.key().c_str()).reserve(arr.size());
        for (auto val : arr) {
          value<std::vector<bool>>(kv.key().c_str()).push_back(val.as<bool>());
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
            .push_back(new ESPConfig{m_fileSys, val.as<JsonObjectConst>()});
        }
        continue;
      }
    }
  }
}

std::string ESPConfig::toJSON(bool pretty) const {
  DynamicJsonDocument json { m_jsonDocSize };

  json[F("Saved")] = true;

  for (auto key : keys()) {
    // the order must match the configValue_t variant definition
    switch (m_config.at(key).index()) {
      case 0:  // bool
        json[key] = value<bool>(key);
        break;
      case 1:  // double
        json[key] = value<double>(key);
        break;
      case 2:  // std::string
        json[key] = value<const char*>(key);
        break;
      case 3:  // ESPConfig_t
        json[key] = value<ESPConfigP_t>(key)->toJSON().c_str();
        break;
      case 4: {  // std::vector<bool>
        auto arr{json.createNestedArray(key)};
        for (const bool val : value<std::vector<bool>>(key)) {
          arr.add(val);
        }
        break;
      }
      case 5: {  // std::vector<double>
        auto arr{json.createNestedArray(key)};
        for (const double val : value<std::vector<double>>(key)) {
          arr.add(val);
        }
        break;
      }
      case 6: {  // std::vector<std::string>
        auto arr{json.createNestedArray(key)};
        for (const std::string& val : value<std::vector<std::string>>(key)) {
          arr.add(val.c_str());
        }
        break;
      }
      case 7: {  // std::vector<ESPConfig_t>
        auto arr{json.createNestedArray(key)};
        for (const auto val : value<std::vector<ESPConfigP_t>>(key)) {
          arr.add(val->toJSON().c_str());
        }
        break;
      }
      default:
        break;
    }
  }

  auto jsonStrLen { measureJsonPretty(json) + 1 };
  std::unique_ptr<char[]> jsonStr { new char[jsonStrLen] };
  if (pretty) {
    serializeJsonPretty(json, jsonStr.get(), jsonStrLen);
  } else {
    serializeJson(json, jsonStr.get(), jsonStrLen);
  }
  return std::string{jsonStr.get()};
}

void ESPConfig::save() const {
  auto jsonStr{toJSON(!m_useEeprom)};
  if (m_useEeprom) {
    if (jsonStr.length() + 1 > m_eepromSize) {
      Serial.printf_P(
          PSTR("ESPConfig error: the config JSON size %d is greater than the "
               "available EEPROM size %d and the config JSON was not saved.\n"
               "Please increase the available EEPROM size\n"),
          jsonStr.length() + 1, m_eepromSize);
      return *this;
    }

    // write to EEPROM
    char configBuff[m_eepromSize];
    memcpy(configBuff, jsonStr.c_str(), jsonStr.length() + 1);
    EEPROM.begin(m_eepromSize);
    EEPROM.put(0, configBuff);
    EEPROM.commit();
    EEPROM.end();
    return *this;
  }

  // write configuration json to FS
  FSInfo fs_info;
  auto mounted = m_fileSys.info(fs_info);
  if (!mounted) { m_fileSys.begin(); }
  auto configFile = m_fileSys.open(m_configFileName.c_str(), "w");
  if (configFile) {
    auto written = configFile.print(jsonStr.c_str());
    configFile.close();
    if (written != jsonStr.length()) {
      Serial.printf_P(PSTR("ESPConfig error: file system write failed, %d written not %d\n"),
                      written, jsonStr.length());
    }
  } else {
    Serial.printf_P(PSTR("ESPConfig error: unable to open config file '%s' for write\n"),
                    m_configFileName.c_str());
  }
  if (!mounted) { m_fileSys.end(); }
  return *this;
}
