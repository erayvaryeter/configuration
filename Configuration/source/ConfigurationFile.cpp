#include "IConfigurationImpl.h"
#include "XMLConfigurationImpl.h"
#include "JSONConfigurationImpl.h"
#include "YAMLConfigurationImpl.h"

#include <sstream>
#include <fstream>
#include <iostream>

IConfiguration::IConfiguration(std::unique_ptr<IConfigurationImpl> impl, std::string name) 
    : pimpl(std::move(impl)), m_nodeName(std::move(name)) {}
    
IConfiguration::~IConfiguration() = default;

std::shared_ptr<IConfiguration> IConfiguration::getSubNode(const std::string & path) {
    auto childImpl = pimpl->getSubNodeInternal(path);
    return childImpl ? std::shared_ptr<IConfiguration>(new IConfiguration(std::move(childImpl))) : nullptr;
}

std::vector<std::shared_ptr<IConfiguration>> IConfiguration::getSubNodes(const std::string & path) {
    auto children = pimpl->getSubNodesInternal(path);
    std::vector<std::shared_ptr<IConfiguration>> result;
    for (auto& child : children) {
        result.push_back(std::shared_ptr<IConfiguration>(new IConfiguration(std::move(child))));
    }
    return result;
}

template<typename T>
bool internalConvert(const std::string & src, T & dst) {
    if (src.empty()) return false;
    std::stringstream ss(src);
    ss.imbue(std::locale::classic());
    return (ss >> dst) ? true : false;
}

template<>
bool internalConvert<std::string>(const std::string& src, std::string& dst) {
    dst = src;
    return true;
}

template<>
bool internalConvert<bool>(const std::string& src, bool& dst) {
    if (src == "1" || src == "true") {
        dst = true;
        return true;
    }
    if (src == "0" || src == "false") {
        dst = false;
        return true;
    }
    return false;
}

template<typename T>
bool IConfiguration::getWithDefaultValue(const std::string & path, T & outValue, const T & defaultValue) {
    std::string raw = pimpl->getRawValue(path);
    if (raw.empty() || !internalConvert<T>(raw, outValue)) {
        outValue = defaultValue;
        return false;
    }
    return true;
}

template<typename T>
void IConfiguration::setValue(const std::string& path, const T& value) {
    pimpl->setValueInternal(path, to_string_helper(value));
}

template<typename T>
std::shared_ptr<IConfiguration> IConfiguration::appendNode(const std::string& name, const T& value) {
    auto childImpl = pimpl->appendNodeInternal(name, to_string_helper(value));
    return std::shared_ptr<IConfiguration>(new IConfiguration(std::move(childImpl)));
}

std::shared_ptr<IConfiguration> IConfiguration::appendSubNode(const std::string& name, std::shared_ptr<IConfiguration> subNode) {
    if (!subNode) return nullptr;

    auto dstChild = this->createGroup(name);
    if (dstChild) {
        ConfigurationFile::recursiveCopy(subNode, dstChild);
    }
    return dstChild;
}

std::shared_ptr<IConfiguration> IConfiguration::createGroup(const std::string& name) {
    auto childImpl = pimpl->createGroupInternal(name);
    return std::shared_ptr<IConfiguration>(new IConfiguration(std::move(childImpl)));
}

template<typename T>
std::string IConfiguration::to_string_helper(const T& value) {
    std::stringstream ss;
    ss.imbue(std::locale::classic());
    ss << value;
    return ss.str();
}

std::shared_ptr<IConfiguration> ConfigurationFile::loadFromFile(const std::string & path, ConfigType type) {
    std::ifstream file(path);
    if (!file.is_open()) return nullptr;

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return loadFromBuffer(buffer.str(), type);
}

std::shared_ptr<IConfiguration> ConfigurationFile::loadFromBuffer(const std::string& buffer, ConfigType type) {
    if (buffer.empty()) return nullptr;

    std::unique_ptr<IConfigurationImpl> concreteImpl = nullptr;

    if (type == ConfigType::XML) {
        concreteImpl = std::make_unique<XMLConfigurationImpl>(buffer);
    }
    else if (type == ConfigType::JSON) {
        concreteImpl = std::make_unique<JSONConfigurationImpl>(buffer);
    }
    else if (type == ConfigType::YAML) {
        concreteImpl = std::make_unique<YAMLConfigurationImpl>(buffer);
    }

    if (!concreteImpl) return nullptr;

    return std::shared_ptr<IConfiguration>(new IConfiguration(std::move(concreteImpl), ""));
}

std::shared_ptr<IConfiguration> ConfigurationFile::createFile(ConfigType type, const std::string& rootName) {
    std::string initialContent;

    if (type == ConfigType::XML) {
        initialContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<" + rootName + "></" + rootName + ">";
    }
    else if (type == ConfigType::JSON) {
        initialContent = "{\"" + rootName + "\": {}}";
    }
    else if (type == ConfigType::YAML) {
        initialContent = rootName + ":\n  {}";
    }

    return loadFromBuffer(initialContent, type);
}

bool ConfigurationFile::convertAndSave(const std::shared_ptr<IConfiguration>& source, ConfigType targetType, const std::string& path) {
    if (!source) return false;
    auto names = source->pimpl->getChildrenNames();
    std::string rootName = names.empty() ? "root" : names[0];
    auto newCfg = createFile(targetType, rootName);
    if (!newCfg) return false;
    auto srcRoot = source->getSubNode(rootName);
    auto dstRoot = newCfg->getSubNode(rootName);
    if (srcRoot && dstRoot) {
        recursiveCopy(srcRoot, dstRoot);
    }
    return saveToFile(newCfg, path);
}

bool ConfigurationFile::saveToFile(const std::shared_ptr<IConfiguration>& root, const std::string& path) {
    if (!root || !root->pimpl) return false;
    return root->pimpl->saveToFileInternal(path);
}

void ConfigurationFile::recursiveCopy(const std::shared_ptr<IConfiguration>& src, std::shared_ptr<IConfiguration>& dst, int depth) {
    if (!src || !dst) return;
    if (depth > 32) return;

    auto childrenNames = src->pimpl->getChildrenNames();

    if (childrenNames.empty()) {
        std::string val = src->pimpl->getRawValue("");
        if (!val.empty()) dst->setValue("", val);
    }
    else {
        for (const auto& name : childrenNames) {
            auto srcChildren = src->getSubNodes(name);
            for (const auto& srcChild : srcChildren) {
                auto dstChild = dst->createGroup(name);
                if (dstChild) {
                    recursiveCopy(srcChild, dstChild, depth + 1);
                }
            }
        }
    }
}

std::vector<std::string> ConfigurationFile::split(const std::string& path, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(path);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}

template bool IConfiguration::getWithDefaultValue<int>(const std::string&, int&, const int&);
template bool IConfiguration::getWithDefaultValue<uint8_t>(const std::string&, uint8_t&, const uint8_t&);
template bool IConfiguration::getWithDefaultValue<uint16_t>(const std::string&, uint16_t&, const uint16_t&);
template bool IConfiguration::getWithDefaultValue<uint32_t>(const std::string&, uint32_t&, const uint32_t&);
template bool IConfiguration::getWithDefaultValue<uint64_t>(const std::string&, uint64_t&, const uint64_t&);
template bool IConfiguration::getWithDefaultValue<int8_t>(const std::string&, int8_t&, const int8_t&);
template bool IConfiguration::getWithDefaultValue<int16_t>(const std::string&, int16_t&, const int16_t&);
template bool IConfiguration::getWithDefaultValue<int32_t>(const std::string&, int32_t&, const int32_t&);
template bool IConfiguration::getWithDefaultValue<int64_t>(const std::string&, int64_t&, const int64_t&);
template bool IConfiguration::getWithDefaultValue<float>(const std::string&, float&, const float&);
template bool IConfiguration::getWithDefaultValue<double>(const std::string&, double&, const double&);
template bool IConfiguration::getWithDefaultValue<long>(const std::string&, long&, const long&);
template bool IConfiguration::getWithDefaultValue<unsigned long>(const std::string&, unsigned long&, const unsigned long&);
template bool IConfiguration::getWithDefaultValue<long long>(const std::string&, long long&, const long long&);
template bool IConfiguration::getWithDefaultValue<std::string>(const std::string&, std::string&, const std::string&);
template bool IConfiguration::getWithDefaultValue<bool>(const std::string&, bool&, const bool&);

template void IConfiguration::setValue<int>(const std::string&, const int&);
template void IConfiguration::setValue<uint8_t>(const std::string&, const uint8_t&);
template void IConfiguration::setValue<uint16_t>(const std::string&, const uint16_t&);
template void IConfiguration::setValue<uint32_t>(const std::string&, const uint32_t&);
template void IConfiguration::setValue<uint64_t>(const std::string&, const uint64_t&);
template void IConfiguration::setValue<int8_t>(const std::string&, const int8_t&);
template void IConfiguration::setValue<int16_t>(const std::string&, const int16_t&);
template void IConfiguration::setValue<int32_t>(const std::string&, const int32_t&);
template void IConfiguration::setValue<int64_t>(const std::string&, const int64_t&);
template void IConfiguration::setValue<float>(const std::string&, const float&);
template void IConfiguration::setValue<double>(const std::string&, const double&);
template void IConfiguration::setValue<long>(const std::string&, const long&);
template void IConfiguration::setValue<unsigned long>(const std::string&, const unsigned long&);
template void IConfiguration::setValue<long long>(const std::string&, const long long&);
template void IConfiguration::setValue<std::string>(const std::string&, const std::string&);
template void IConfiguration::setValue<bool>(const std::string&, const bool&);

template std::shared_ptr<IConfiguration> IConfiguration::appendNode<int>(const std::string&, const int&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<uint8_t>(const std::string&, const uint8_t&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<uint16_t>(const std::string&, const uint16_t&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<uint32_t>(const std::string&, const uint32_t&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<uint64_t>(const std::string&, const uint64_t&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<int8_t>(const std::string&, const int8_t&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<int16_t>(const std::string&, const int16_t&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<int32_t>(const std::string&, const int32_t&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<int64_t>(const std::string&, const int64_t&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<float>(const std::string&, const float&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<double>(const std::string&, const double&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<long>(const std::string&, const long&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<unsigned long>(const std::string&, const unsigned long&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<long long>(const std::string&, const long long&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<std::string>(const std::string&, const std::string&);
template std::shared_ptr<IConfiguration> IConfiguration::appendNode<bool>(const std::string&, const bool&);