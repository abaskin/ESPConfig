# ESP Configuration Library

This is a library for managing configuration settings with support for saving to 
EEPROM or the file system (LittleFS, SPIFFS, SD). The ESP_EEPROM library 
(jwrw/ESP_EEPROM) is used to access the EEPROM.

## Using the library

When an ESPConfig object is created the existing configuration is read from
the EEPROM or the file system. Values can then be retrived from or written to the
ESPConfig object and the `save` method called to persist the values. If the data 
in the EEPROM is not valid the configuration file is used. This is useful on 
the case of a new device where the EEPROM has not been written yet. The 
configuration file contains JSON. 

## Supported Value Types

C++ Type | JSON Type
-------- | ---------
`bool` | boolean
`double` | number
`std::string` | string
`ESPConfigP_t` | object
`std::vector<bool>` | array of boolean
`std::vector<double>` | array of number
`std::vector<std::string>` | array of string
`std::vector<ESPConfigP_t>` | array of object

## Declaring the ESPConfig object

```c++
ESPConfig objectName(fs::FS& fileSys, 
                     const char* configFileName,
                     bool useEeprom = true);
```

- **objectName** - the name of the object
- **fileSys** - the file system for the configFileName (LittleFS, SPIFFS, SD)
- **configFileName** - the filename of the configuration file in the file system
- **useEeprom** - use the EEPROM to store the configuration data

## Object Methods

```c++
bool is<T>(const char* key)

bool is<bool>(const char* key)
bool is<double>(const char* key)
bool is<std::string>(const char* key)
bool is<ESPConfigP_t>(const char* key)
bool is<std::vector<bool>>(const char* key)
bool is<std::vector<double>>(const char* key)
bool is<std::vector<std::sting>>(const char* key)
bool is<std::vector<ESPConfigP_t>>(const char* key)
bool is<std::array<double, 2>>(const char* key)
```

- **key** - the value's key

Tests if the ESPConfig object is currently holding a value of type T.

```c++
T value<T>(key)

bool value<bool>(key)
double value<double>(key)
const char* value<const char*>(key)
ESPConfigP_t value<ESPConfigP_t>(key)
std::array<double, 2> value<std::array<double, 2>>(key)
std::vector<bool> value<std::vector<bool>>(key)
std::vector<double> value<std::vector<double>>(key)
std::vector<std::string> value<std::vector<std::string>>(key)
std::vector<ESPConfigP_t> value<std::vector<ESPConfigP_t>>(key)
```

- **key** - the value's key

Retrive the value for a given key.

```c++
void value(const char* key, T value)

void value(const char* key, bool value)
void value(const char* key, double value)
void value(const char* key, const char* value)
void value(const char* key, ESPConfigP_t value)
void value(const char* key, std::array<double, 2> value)
void value(const char* key, std::vector<bool> value)
void value(const char* key, std::vector<double> value)
void value(const char* key, std::vector<std::string> value)
void value(const char* key, std::vector<ESPConfigP_t> value)
```

- **key** - the value's key

Set the value for a given key.

```c++
std::vector<const char*> keys()
```

Retreive all the keys for a ESPConfig object.

```c++
void save();
```

Save the configuration to either EEPROM or the file system.

```c++
std::string toJSON(bool pretty = true)
```

- **pretty** - create a prettified JSON document, i.e. a document with spaces 
and line-breaks between values

Return the configuration data as JSON.
