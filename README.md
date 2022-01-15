# ESP Configuration Library

This is a library for managing configuration settings with support for saving to
EEPROM or the file system (LittleFS, SPIFFS, SD) as JSON. On the ESP8266 the
ESP_EEPROM library (jwrw/ESP_EEPROM) is used to access the EEPROM. On the ESP32
the Preferences library is used.

## Using the library

Once an ESPConfig object is created the existing configuration can then be read
from the EEPROM, file system, or a JSON string. Values can then be retrieved
from or written to the ESPConfig object and the `save` method called to persist
the values to the EEPROM or file system. The configuration files are processed
in the order specified, followed by the EEPROM if present. In the case where you
would like to handle reading and writing the data yourself, call `read` with the
data in a JSON string and call `toJSON` to get the data to store as required. This
facilitates using with another library that also writes to the EEPROM. The flags
passed to ArduinoJson in the library.json file allow for comments to be used
configuration files stored in the filesystem if used with PlatformIO.

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

```c++
ESPConfig objectName(fs::FS& fileSys,
                     std::vector<const char*> configFileList,
                     bool useEeprom = true);
```

- **objectName** - the name of the object
- **fileSys** - the file system for the configFileName (LittleFS, SPIFFS, SD)
- **configFileList** - a list filenames of configuration files in the file system
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

Retrieve the value for a given key.

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
- **value** - the value to be saved

Set the value for a given key.

```c++
std::vector<const char*> keys()
```

Retrieve all the keys for the ESPConfig object.

```c++
ESPConfig& read();
ESPConfig& read(const char* jsonStr, size_t jsonStrLen)
```

- **jsonStr** - the JSON string to process
- **jsonStrLen** -  the maximum number of bytes to read from jsonStr

Read configuration from the EEPROM, file system, or the passed JSON string.

```c++
void save();
```

Save the configuration to either the EEPROM or the file system.

```c++
std::string toJSON(bool pretty = true)
```

- **pretty** - create a prettified JSON document, i.e. a document with spaces
and line-breaks between values

Return the configuration data as JSON.

```c++
ESPConfig& remove(const char* key)
```

- **key** - the value's key

Remove a value based in its key

```c++
ESPConfig& reset()
```

Remove all values
