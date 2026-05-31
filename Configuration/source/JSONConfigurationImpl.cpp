#include "JSONConfigurationImpl.h"
#include <fstream>
#include <iomanip>

JSONConfigurationImpl::JSONConfigurationImpl(const std::string& buffer) {
    _doc = std::make_shared<nlohmann::json>();
    try {
        if (buffer.empty() || buffer == "{}") {
            *_doc = nlohmann::json::object();
        }
        else {
            *_doc = nlohmann::json::parse(buffer);
        }
    }
    catch (...) {
        *_doc = nlohmann::json::object();
    }
    _node = _doc.get();
}

JSONConfigurationImpl::JSONConfigurationImpl(std::shared_ptr<nlohmann::json> doc, nlohmann::json* node)
    : _doc(doc), _node(node) {}

std::unique_ptr<IConfigurationImpl> JSONConfigurationImpl::getSubNodeInternal(const std::string& path) {
    if (path.empty() || path == ".") {
        return std::make_unique<JSONConfigurationImpl>(_doc, _node);
    }

    nlohmann::json* current = _node;
    auto tokens = ConfigurationFile::split(path);

    for (const auto& token : tokens) {
        if (current->is_object() && current->contains(token)) {
            current = &((*current)[token]);
        }
        else {
            return nullptr;
        }
    }
    return std::make_unique<JSONConfigurationImpl>(_doc, current);
}

std::vector<std::unique_ptr<IConfigurationImpl>> JSONConfigurationImpl::getSubNodesInternal(const std::string& path) {
    std::vector<std::unique_ptr<IConfigurationImpl>> results;
    nlohmann::json* target = _node;

    if (!path.empty() && path != ".") {
        auto tokens = ConfigurationFile::split(path);
        for (const auto& token : tokens) {
            if (target->is_object() && target->contains(token)) {
                target = &((*target)[token]);
            }
            else {
                return results;
            }
        }
    }

    if (target->is_array()) {
        for (auto& element : *target) {
            results.push_back(std::make_unique<JSONConfigurationImpl>(_doc, &element));
        }
    }
    else {
        results.push_back(std::make_unique<JSONConfigurationImpl>(_doc, target));
    }

    return results;
}

std::string JSONConfigurationImpl::getRawValue(const std::string& path) {
    nlohmann::json* t = _node;
    if (!path.empty() && path != ".") {
        auto tokens = ConfigurationFile::split(path);
        for (const auto& token : tokens) {
            if (t->is_object() && t->contains(token)) {
                t = &((*t)[token]);
            }
            else return "";
        }
    }

    if (t->is_string()) return t->get<std::string>();
    if (t->is_primitive()) return t->dump();
    return "";
}

void JSONConfigurationImpl::setValueInternal(const std::string& path, const std::string& value) {
    nlohmann::json* t = _node;
    auto tokens = ConfigurationFile::split(path);

    for (const auto& token : tokens) {
        if (!t->is_object()) {
            *t = nlohmann::json::object();
        }
        t = &((*t)[token]);
    }
    *t = value;
}

std::unique_ptr<IConfigurationImpl> JSONConfigurationImpl::appendNodeInternal(const std::string& name, const std::string& value) {
    if (!_node->is_object()) {
        *_node = nlohmann::json::object();
    }

    nlohmann::json newNodeValue = value.empty() ? nlohmann::json::object() : nlohmann::json(value);

    if (!_node->contains(name)) {
        (*_node)[name] = newNodeValue;
    }
    else {
        if (!(*_node)[name].is_array()) {
            nlohmann::json old = std::move((*_node)[name]);
            (*_node)[name] = nlohmann::json::array();
            (*_node)[name].push_back(std::move(old));
        }
        (*_node)[name].push_back(std::move(newNodeValue));
    }

    nlohmann::json* target = &((*_node)[name]);
    if (target->is_array()) target = &(target->back());

    return std::make_unique<JSONConfigurationImpl>(_doc, target);
}

std::unique_ptr<IConfigurationImpl> JSONConfigurationImpl::createGroupInternal(const std::string& name) {
    if (!_node->is_object()) *_node = nlohmann::json::object();

    nlohmann::json newNodeValue = nlohmann::json::object();
    nlohmann::json* targetNode = nullptr;

    if (!_node->contains(name)) {
        (*_node)[name] = newNodeValue;
        targetNode = &((*_node)[name]);
    }
    else {
        if (!(*_node)[name].is_array()) {
            nlohmann::json old = std::move((*_node)[name]);
            (*_node)[name] = nlohmann::json::array();
            (*_node)[name].push_back(std::move(old));
        }
        (*_node)[name].push_back(std::move(newNodeValue));
        targetNode = &((*_node)[name].back());
    }

    return std::make_unique<JSONConfigurationImpl>(_doc, targetNode);
}

std::vector<std::string> JSONConfigurationImpl::getChildrenNames() {
    std::vector<std::string> names;
    if (_node && _node->is_object()) {
        for (auto it = _node->begin(); it != _node->end(); ++it) {
            names.push_back(it.key());
        }
    }
    return names;
}

std::string JSONConfigurationImpl::serialize(ConfigType targetType) {
    return _node->dump(4);
}

bool JSONConfigurationImpl::saveToFileInternal(const std::string& path) {
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << std::setw(4) << *_doc;
    return true;
}