#pragma once

#include "IConfigurationImpl.h"
#include "json.hpp"

class JSONConfigurationImpl : public IConfigurationImpl {
public:
    explicit JSONConfigurationImpl(const std::string& buffer);
    explicit JSONConfigurationImpl(std::shared_ptr<nlohmann::json> doc, nlohmann::json* node);

    std::unique_ptr<IConfigurationImpl> getSubNodeInternal(const std::string& path) override;
    std::vector<std::unique_ptr<IConfigurationImpl>> getSubNodesInternal(const std::string& path) override;
    std::string getRawValue(const std::string& path) override;
    std::string serialize(ConfigType targetType) override;
    void setValueInternal(const std::string& path, const std::string& value) override;
    std::unique_ptr<IConfigurationImpl> appendNodeInternal(const std::string& name, const std::string& value) override;
    std::unique_ptr<IConfigurationImpl> createGroupInternal(const std::string& name) override;
    std::vector<std::string> getChildrenNames() override;
    bool saveToFileInternal(const std::string& path) override;

private:
    std::shared_ptr<nlohmann::json> _doc;
    nlohmann::json* _node;
};