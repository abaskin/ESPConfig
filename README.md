# ESP Configuration Library

This is a library for managing configuration settings with support for saving to
EEPROM or the file system (LittleFS, SPIFFS, SD). Configuration information is
stored is stored as JSON.

## Using the library

Once an ESPConfig object is created the existing configuration can then be read
from the EEPROM, file system, or a JSON string. Values can then be retrieved
from or written to the ESPConfig object and the `save` method called to persist
the values to the EEPROM or file system. The configuration files are processed
in reverse of the order specified, followed by the EEPROM if present. In the case
where you would like to handle reading and writing the data yourself, call `read`
with the data in a JSON string and call `toJSON` to get the data to store as
required. This facilitates using with another library that also writes to the
EEPROM. The flags passed to ArduinoJson in the library.json file allow for
comments to be used configuration files stored in the filesystem when used with
PlatformIO.

## Supported Value Types

C++ Type | JSON Type
-------- | ---------
`bool` | boolean
`uint32_t` | number
`double` | number
`std::string` | string
`ESPConfigP_t` | object
`std::vector<bool>` | array of boolean
`std::vector<int32_t>` | array of number
`std::vector<double>` | array of number
`std::vector<std::string>` | array of string
`std::vector<ESPConfigP_t>` | array of object

## Declaring the ESPConfig object

Calling the constructor without any arguments creates an object that only reads
from and writes to the EEPROM.

```c++
ESPConfig objectName();
```

- **objectName** - the name of the object

```c++
ESPConfig objectName(const char* configFileName,
                     fs::FS* fileSys,
                     const mountCallBack_t mountCB = [](){},
                     const mountCallBack_t unmountCB = [](){},
                     const bool useEeprom = true);
```

- **objectName** - the name of the object
- **configFileName** - the filename of the configuration file in the file system
- **fileSys** - the file system for the configFileName (LittleFS, SPIFFS, SD)
- **mountCB** - a callback to mount the filesystem if required
- **unmountCB** - a callback to unmount the filesystem if required
- **useEeprom** - use the EEPROM to store the configuration data

```c++
ESPConfig objectName(const std::vector<const char*> configFileList,
                     fs::FS* fileSys,
                     const mountCallBack_t mountCB = [](){},
                     const mountCallBack_t unmountCB = [](){},
                     const bool useEeprom = true);
```

- **objectName** - the name of the object
- **configFileList** - a list filenames of configuration files in the file system
- **fileSys** - the file system for the configFileName (LittleFS, SPIFFS, SD)
- **mountCB** - a callback to mount the filesystem if required
- **unmountCB** - a callback to unmount the filesystem if required
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
std::vector<double> value<std::vector<int32_t>>(key)
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
void value(const char* key, std::vector<int32_t> value)
void value(const char* key, std::vector<double> value)
void value(const char* key, std::vector<std::string> value)
void value(const char* key, std::vector<ESPConfigP_t> value)
```

- **key** - the value's key
- **value** - the value to be saved

Set the value for a given key.
Note: Empty vectors may be used to create a value but they will not be read back
from persistent storage. This is because there is no way to determine the type of
an empty array in JSON.

```c++
std::vector<std::string> keys()
```

Retrieve all the keys for the ESPConfig object.

```c++
ESPConfig& read();
ESPConfig& read(const char* jsonStr);
ESPConfig& read(const char* jsonStr, size_t jsonStrLen)
```

- **jsonStr** - the JSON string to process
- **jsonStrLen** -  the maximum number of bytes to read from jsonStr

Read configuration from the EEPROM, file system, or the passed JSON string.
Note: Empty arrays in the JSON will be ignored because there is no way to determine
the type of an empty array.

```c++
void save();
```

Save the configuration to either the EEPROM or the file system. The first
configuration file is used if the object was created with useEeprom false and
a configuration file was specified.

```c++
std::string toJSON(ESPConfig::saveFormat format = ESPConfig::saveFormat::minified)
```

- **format** - the format to return
  - ESPConfig::saveFormat::minified - minified JSON document, i.e. a document without spaces or line break between values.
  - ESPConfig::saveFormat::pretty - prettified JSON document, i.e. a document with spaces and line-breaks between values
  - ESPConfig::saveFormat::msgPack - a MessagePack document

Return the configuration data as JSON or MessagePack.

```c++
ESPConfig& remove(const char* key)
```

- **key** - the value's key

Remove a value based in its key.

```c++
ESPConfig& reset()
```

Remove all values.

## Compile Time Settings

The following value can be set at compile time with preprocessor macro identifiers.

Macro Identifier | Setting | Default
---------------- | ------- | -------
ESPCONFIG_EEPROMSIZE | The size of the EEPROM area used to save the configuration | 1024
ESPCONFIG_JSONDOCSIZE | The size of the JsonDocument used by the configuration| 1024
ESPCONFIG_SAVEDKEY | The key in the saved JSON to mark it as being saved by ESPConfig | ESPConfigSaved
