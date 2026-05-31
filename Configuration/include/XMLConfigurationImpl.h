#pragma once

#include "IConfigurationImpl.h"
#include "pugixml.hpp"

class XMLConfigurationImpl : public IConfigurationImpl {
public:
    explicit XMLConfigurationImpl(const std::string& buffer);
    explicit XMLConfigurationImpl(std::shared_ptr<pugi::xml_document> doc, pugi::xml_node node);

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
    std::shared_ptr<pugi::xml_document> _doc;
    pugi::xml_node _node;
};