#pragma once

#include <string>
#include <vector>
#include <memory>

enum class ConfigType { 
    XML, 
    JSON, 
    YAML 
};

class IConfigurationImpl;

class IConfiguration {
public:
    ~IConfiguration();

    std::shared_ptr<IConfiguration> getSubNode(const std::string& path);
    std::vector<std::shared_ptr<IConfiguration>> getSubNodes(const std::string& path);

    template<typename T>
    bool getWithDefaultValue(const std::string& path, T& outValue, const T& defaultValue);

    template<typename T>
    void setValue(const std::string& path, const T& value);
    void setValue(const std::string& path, const char* value) { this->setValue(path, std::string(value)); }

    template<typename T>
    std::shared_ptr<IConfiguration> appendNode(const std::string& name, const T& value);
    std::shared_ptr<IConfiguration> appendNode(const std::string& name, const char* value) { return this->appendNode(name, std::string(value)); }
    std::shared_ptr<IConfiguration> createGroup(const std::string& name);

private:
    explicit IConfiguration(std::unique_ptr<IConfigurationImpl> impl);
    friend class ConfigurationFile;

    std::string to_string_helper(const std::string& value) { return value; }
    std::string to_string_helper(const char* value) { return std::string(value); }
    std::string to_string_helper(bool value) { return value ? "1" : "0"; }

    template<typename T>
    std::string to_string_helper(const T& value);

    std::unique_ptr<IConfigurationImpl> pimpl;
};

class ConfigurationFile {
public:
    static std::shared_ptr<IConfiguration> loadFromFile(const std::string& path, ConfigType type);
    static std::shared_ptr<IConfiguration> loadFromBuffer(const std::string& buffer, ConfigType type);
    static std::shared_ptr<IConfiguration> createFile(ConfigType type, const std::string& rootName = "root");
    static bool convertAndSave(const std::shared_ptr<IConfiguration>& source, ConfigType targetType, const std::string& path);
    static bool saveToFile(const std::shared_ptr<IConfiguration>& root, const std::string& path);

    friend class XMLConfigurationImpl;
    friend class YAMLConfigurationImpl;
    friend class JSONConfigurationImpl;

private:
    static void recursiveCopy(const std::shared_ptr<IConfiguration>& src, std::shared_ptr<IConfiguration>& dst, int depth = 0);
    static std::vector<std::string> split(const std::string& path, char delimiter = '.');

    ConfigurationFile() = default;
};