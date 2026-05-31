#pragma once

#include "IConfiguration.h"

class IConfigurationImpl {
public:
    virtual ~IConfigurationImpl() = default;

    virtual std::unique_ptr<IConfigurationImpl> getSubNodeInternal(const std::string& path) = 0;
    virtual std::vector<std::unique_ptr<IConfigurationImpl>> getSubNodesInternal(const std::string& path) = 0;
    virtual std::string getRawValue(const std::string& path) = 0;
    virtual std::string serialize(ConfigType targetType) = 0;
    virtual void setValueInternal(const std::string& path, const std::string& value) = 0;
    virtual std::unique_ptr<IConfigurationImpl> appendNodeInternal(const std::string& name, const std::string& value) = 0;
    virtual std::unique_ptr<IConfigurationImpl> createGroupInternal(const std::string& name) = 0;
    virtual std::vector<std::string> getChildrenNames() = 0;
    virtual bool saveToFileInternal(const std::string& path) = 0;
};