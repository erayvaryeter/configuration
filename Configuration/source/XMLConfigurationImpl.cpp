#include <sstream>
#include "XMLConfigurationImpl.h"

XMLConfigurationImpl::XMLConfigurationImpl(const std::string& buffer) {
    _doc = std::make_shared<pugi::xml_document>();
    pugi::xml_parse_result result = _doc->load_string(buffer.c_str());
    _node = *_doc;
}

XMLConfigurationImpl::XMLConfigurationImpl(std::shared_ptr<pugi::xml_document> doc, pugi::xml_node node) : _doc(doc), _node(node) {}

std::unique_ptr<IConfigurationImpl> XMLConfigurationImpl::getSubNodeInternal(const std::string& path) {
    auto tokens = ConfigurationFile::split(path);
    pugi::xml_node current = _node;
    for (const auto& token : tokens) {
        current = current.child(token.c_str());
        if (!current) return nullptr;
    }
    return std::make_unique<XMLConfigurationImpl>(_doc, current);
}

std::vector<std::unique_ptr<IConfigurationImpl>> XMLConfigurationImpl::getSubNodesInternal(const std::string& path) {
    std::vector<std::unique_ptr<IConfigurationImpl>> results;
    auto tokens = ConfigurationFile::split(path);

    pugi::xml_node parent = _node;
    std::string lastToken = path;

    if (!tokens.empty()) {
        lastToken = tokens.back();
        tokens.pop_back();
        for (const auto& token : tokens) {
            parent = parent.child(token.c_str());
            if (!parent) return results;
        }
    }

    for (pugi::xml_node child : parent.children(lastToken.c_str())) {
        results.push_back(std::make_unique<XMLConfigurationImpl>(_doc, child));
    }
    return results;
}

std::string XMLConfigurationImpl::getRawValue(const std::string& path) {
    if (path.empty() || path == ".") {
        return _node.child_value();
    }

    auto tokens = ConfigurationFile::split(path);
    pugi::xml_node current = _node;
    for (const auto& token : tokens) {
        current = current.child(token.c_str());
        if (!current) return "";
    }
    return current.child_value();
}

std::string XMLConfigurationImpl::serialize(ConfigType targetType) {
    std::stringstream ss;
    _doc->save(ss, "\t");
    return ss.str();
}

void XMLConfigurationImpl::setValueInternal(const std::string& path, const std::string& value) {
    pugi::xml_node targetNode;

    if (path.empty() || path == ".") {
        targetNode = _node;
    }
    else {
        auto tokens = ConfigurationFile::split(path);
        targetNode = _node;

        for (const auto& token : tokens) {
            targetNode = targetNode.child(token.c_str());
            if (!targetNode) return;
        }
    }

    if (targetNode.first_child().type() == pugi::node_pcdata) {
        targetNode.first_child().set_value(value.c_str());
    }
    else {
        targetNode.append_child(pugi::node_pcdata).set_value(value.c_str());
    }
}

std::unique_ptr<IConfigurationImpl> XMLConfigurationImpl::appendNodeInternal(const std::string& name, const std::string& value) {
    pugi::xml_node newNode = _node.append_child(name.c_str());
    if (!value.empty()) {
        newNode.append_child(pugi::node_pcdata).set_value(value.c_str());
    }
    return std::make_unique<XMLConfigurationImpl>(_doc, newNode);
}

std::unique_ptr<IConfigurationImpl> XMLConfigurationImpl::createGroupInternal(const std::string& name) {
    pugi::xml_node newNode = _node.append_child(name.c_str());
    return std::make_unique<XMLConfigurationImpl>(_doc, newNode);
}

std::vector<std::string> XMLConfigurationImpl::getChildrenNames() {
    std::vector<std::string> names;
    for (pugi::xml_node child : _node.children()) {
        if (child.type() == pugi::node_element) {
            std::string n = child.name();
            if (std::find(names.begin(), names.end(), n) == names.end()) {
                names.push_back(n);
            }
        }
    }
    return names;
}

bool XMLConfigurationImpl::saveToFileInternal(const std::string& path) {
    if (!_doc) return false;
    return _doc->save_file(path.c_str(), "\t");
}