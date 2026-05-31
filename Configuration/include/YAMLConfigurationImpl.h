#pragma once

#include "IConfigurationImpl.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

struct YamlNode {
    enum class Type { Map, Sequence, Scalar, Null };

    Type type = Type::Null;
    std::string scalar;
    std::vector<std::pair<std::string, std::shared_ptr<YamlNode>>> mapItems;
    std::vector<std::shared_ptr<YamlNode>> seqItems;

    static std::shared_ptr<YamlNode> makeMap();
    static std::shared_ptr<YamlNode> makeSequence();
    static std::shared_ptr<YamlNode> makeScalar(const std::string& value);
    static std::shared_ptr<YamlNode> makeNull();

    bool hasKey(const std::string& key) const;
    std::shared_ptr<YamlNode> get(const std::string& key) const;
    void set(const std::string& key, std::shared_ptr<YamlNode> node);
    std::vector<std::string> keys() const;
};

class YAMLConfigurationImpl : public IConfigurationImpl {
public:
    explicit YAMLConfigurationImpl(const std::string& buffer);
    explicit YAMLConfigurationImpl(std::shared_ptr<YamlNode> doc, std::shared_ptr<YamlNode> node);

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
    std::shared_ptr<YamlNode> _doc;
    std::shared_ptr<YamlNode> _node;

    std::shared_ptr<YamlNode> navigateTo(const std::string& path) const;
    std::shared_ptr<YamlNode> navigateToParent(const std::string& path, std::string& lastToken) const;

    static std::shared_ptr<YamlNode> parse(const std::string& buffer);
    static void writeNode(std::ostream& out, const std::shared_ptr<YamlNode>& node, int indent, bool inSeq);
};