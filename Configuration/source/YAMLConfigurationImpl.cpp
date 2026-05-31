#include "YAMLConfigurationImpl.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

std::shared_ptr<YamlNode> YamlNode::makeMap() {
    auto n = std::make_shared<YamlNode>();
    n->type = Type::Map;
    return n;
}

std::shared_ptr<YamlNode> YamlNode::makeSequence() {
    auto n = std::make_shared<YamlNode>();
    n->type = Type::Sequence;
    return n;
}

std::shared_ptr<YamlNode> YamlNode::makeScalar(const std::string& value) {
    auto n = std::make_shared<YamlNode>();
    n->type = Type::Scalar;
    n->scalar = value;
    return n;
}

std::shared_ptr<YamlNode> YamlNode::makeNull() {
    return std::make_shared<YamlNode>();
}

bool YamlNode::hasKey(const std::string& key) const {
    for (const auto& p : mapItems)
        if (p.first == key) return true;
    return false;
}

std::shared_ptr<YamlNode> YamlNode::get(const std::string& key) const {
    for (const auto& p : mapItems)
        if (p.first == key) return p.second;
    return nullptr;
}

void YamlNode::set(const std::string& key, std::shared_ptr<YamlNode> node) {
    for (auto& p : mapItems) {
        if (p.first == key) {
            p.second = node;
            return;
        }
    }
    mapItems.push_back({ key, node });
}

std::vector<std::string> YamlNode::keys() const {
    std::vector<std::string> result;
    for (const auto& p : mapItems)
        result.push_back(p.first);
    return result;
}

static int getIndent(const std::string& line) {
    int count = 0;
    for (char c : line) {
        if (c == ' ') ++count;
        else break;
    }
    return count;
}

static std::string trimLeft(const std::string& s) {
    size_t i = 0;
    while (i < s.size() && s[i] == ' ') ++i;
    return s.substr(i);
}

static std::string trimRight(const std::string& s) {
    size_t i = s.size();
    while (i > 0 && (s[i - 1] == ' ' || s[i - 1] == '\r' || s[i - 1] == '\n')) --i;
    return s.substr(0, i);
}

static std::string trim(const std::string& s) {
    return trimRight(trimLeft(s));
}

static std::string unquote(const std::string& s) {
    if (s.size() >= 2) {
        if ((s.front() == '"' && s.back() == '"') ||
            (s.front() == '\'' && s.back() == '\'')) {
            return s.substr(1, s.size() - 2);
        }
    }
    return s;
}

struct RawLine {
    int indent;
    std::string content;
};

static std::vector<RawLine> toRawLines(const std::string& buffer) {
    std::vector<RawLine> lines;
    std::istringstream ss(buffer);
    std::string line;
    while (std::getline(ss, line)) {
        std::string trimmed = trimRight(line);
        if (trimmed.empty()) continue;
        std::string content = trimLeft(trimmed);
        if (content.empty() || content[0] == '#') continue;
        lines.push_back({ getIndent(trimmed), content });
    }
    return lines;
}

static std::shared_ptr<YamlNode> parseBlock(const std::vector<RawLine>& lines, size_t& i, int baseIndent);

static std::shared_ptr<YamlNode> parseSequenceBlock(const std::vector<RawLine>& lines, size_t& i, int seqIndent) {
    auto seq = YamlNode::makeSequence();
    while (i < lines.size() && lines[i].indent == seqIndent && lines[i].content[0] == '-') {
        std::string rest = trim(lines[i].content.substr(1));
        ++i;

        if (rest.empty()) {
            auto child = parseBlock(lines, i, seqIndent + 2);
            seq->seqItems.push_back(child);
        }
        else {
            size_t colon = rest.find(':');
            if (colon != std::string::npos && (colon + 1 >= rest.size() || rest[colon + 1] == ' ' || rest[colon + 1] == '\0')) {
                auto mapNode = YamlNode::makeMap();
                std::string key = trim(rest.substr(0, colon));
                std::string val = (colon + 1 < rest.size()) ? trim(rest.substr(colon + 1)) : "";

                if (!val.empty() && val != "{}") {
                    mapNode->set(key, YamlNode::makeScalar(unquote(val)));
                }
                else {
                    auto child = parseBlock(lines, i, seqIndent + 2);
                    mapNode->set(key, child);
                }

                while (i < lines.size() && lines[i].indent > seqIndent && lines[i].content[0] != '-') {
                    std::string innerContent = lines[i].content;
                    size_t c2 = innerContent.find(':');
                    if (c2 != std::string::npos && (c2 + 1 >= innerContent.size() || innerContent[c2 + 1] == ' ')) {
                        std::string k2 = trim(innerContent.substr(0, c2));
                        std::string v2 = (c2 + 1 < innerContent.size()) ? trim(innerContent.substr(c2 + 1)) : "";
                        ++i;
                        if (v2.empty() || v2 == "{}") {
                            auto child2 = parseBlock(lines, i, lines[i > 0 ? i : 0].indent);
                            mapNode->set(k2, child2);
                        }
                        else {
                            mapNode->set(k2, YamlNode::makeScalar(unquote(v2)));
                        }
                    }
                    else {
                        break;
                    }
                }

                seq->seqItems.push_back(mapNode);
            }
            else {
                seq->seqItems.push_back(YamlNode::makeScalar(unquote(rest)));
            }
        }
    }
    return seq;
}

static std::shared_ptr<YamlNode> parseBlock(const std::vector<RawLine>& lines, size_t& i, int baseIndent) {
    if (i >= lines.size()) return YamlNode::makeNull();

    if (lines[i].content[0] == '-') {
        return parseSequenceBlock(lines, i, lines[i].indent);
    }

    auto map = YamlNode::makeMap();

    while (i < lines.size() && lines[i].indent >= baseIndent) {
        if (lines[i].indent > baseIndent) {
            ++i;
            continue;
        }

        const std::string& content = lines[i].content;

        if (content[0] == '-') {
            return parseSequenceBlock(lines, i, lines[i].indent);
        }

        size_t colon = content.find(':');
        if (colon == std::string::npos) {
            ++i;
            continue;
        }

        std::string key = trim(content.substr(0, colon));
        std::string val = (colon + 1 < content.size()) ? trim(content.substr(colon + 1)) : "";
        ++i;

        if (val.empty() || val == "{}") {
            if (i < lines.size() && lines[i].indent > baseIndent) {
                int childIndent = lines[i].indent;
                if (lines[i].content[0] == '-') {
                    auto seq = parseSequenceBlock(lines, i, childIndent);
                    map->set(key, seq);
                }
                else {
                    auto child = parseBlock(lines, i, childIndent);
                    map->set(key, child);
                }
            }
            else {
                map->set(key, YamlNode::makeMap());
            }
        }
        else {
            map->set(key, YamlNode::makeScalar(unquote(val)));
        }
    }

    return map;
}

std::shared_ptr<YamlNode> YAMLConfigurationImpl::parse(const std::string& buffer) {
    auto lines = toRawLines(buffer);
    if (lines.empty()) return YamlNode::makeMap();
    size_t i = 0;
    return parseBlock(lines, i, 0);
}

void YAMLConfigurationImpl::writeNode(std::ostream& out, const std::shared_ptr<YamlNode>& node, int indent, bool inSeq) {
    if (!node) return;

    std::string pad(indent, ' ');

    switch (node->type) {
    case YamlNode::Type::Scalar:
        out << node->scalar << "\n";
        break;

    case YamlNode::Type::Map:
        for (size_t idx = 0; idx < node->mapItems.size(); ++idx) {
            const auto& kv = node->mapItems[idx];
            const std::string& k = kv.first;
            const auto& v = kv.second;

            if (inSeq && idx == 0) {
                out << pad.substr(0, pad.size() >= 2 ? pad.size() - 2 : 0) << "- ";
            }
            else {
                out << pad;
            }

            if (!v || v->type == YamlNode::Type::Null) {
                out << k << ": {}\n";
            }
            else if (v->type == YamlNode::Type::Scalar) {
                out << k << ": " << v->scalar << "\n";
            }
            else if (v->type == YamlNode::Type::Map) {
                out << k << ":\n";
                writeNode(out, v, indent + 2, false);
            }
            else if (v->type == YamlNode::Type::Sequence) {
                out << k << ":\n";
                for (const auto& item : v->seqItems) {
                    if (item->type == YamlNode::Type::Scalar) {
                        out << std::string(indent + 2, ' ') << "- ";
                        writeNode(out, item, indent + 2, false);
                    }
                    else {
                        writeNode(out, item, indent + 4, true);
                    }
                }
            }
        }
        break;

    case YamlNode::Type::Sequence:
        for (const auto& item : node->seqItems) {
            if (item->type == YamlNode::Type::Scalar) {
                out << pad << "- ";
                writeNode(out, item, indent, false);
            }
            else {
                writeNode(out, item, indent + 2, true);
            }
        }
        break;

    case YamlNode::Type::Null:
        out << pad << "{}\n";
        break;
    }
}

std::shared_ptr<YamlNode> YAMLConfigurationImpl::navigateTo(const std::string& path) const {
    if (path.empty() || path == ".") return _node;
    auto tokens = ConfigurationFile::split(path);
    auto current = _node;
    for (const auto& token : tokens) {
        if (!current || current->type != YamlNode::Type::Map) return nullptr;
        current = current->get(token);
    }
    return current;
}

std::shared_ptr<YamlNode> YAMLConfigurationImpl::navigateToParent(const std::string& path, std::string& lastToken) const {
    auto tokens = ConfigurationFile::split(path);
    if (tokens.empty()) { lastToken = ""; return _node; }
    lastToken = tokens.back();
    tokens.pop_back();
    auto current = _node;
    for (const auto& token : tokens) {
        if (!current || current->type != YamlNode::Type::Map) return nullptr;
        auto next = current->get(token);
        if (!next) {
            auto newMap = YamlNode::makeMap();
            current->set(token, newMap);
            current = newMap;
        }
        else {
            current = next;
        }
    }
    return current;
}

YAMLConfigurationImpl::YAMLConfigurationImpl(const std::string& buffer) {
    auto parsed = parse(buffer);
    _doc = parsed;
    _node = parsed;
}

YAMLConfigurationImpl::YAMLConfigurationImpl(std::shared_ptr<YamlNode> doc, std::shared_ptr<YamlNode> node)
    : _doc(doc), _node(node)
{
}

std::unique_ptr<IConfigurationImpl> YAMLConfigurationImpl::getSubNodeInternal(const std::string& path) {
    if (path.empty() || path == ".") {
        return std::make_unique<YAMLConfigurationImpl>(_doc, _node);
    }
    auto target = navigateTo(path);
    if (!target) return nullptr;
    return std::make_unique<YAMLConfigurationImpl>(_doc, target);
}

std::vector<std::unique_ptr<IConfigurationImpl>> YAMLConfigurationImpl::getSubNodesInternal(const std::string& path) {
    std::vector<std::unique_ptr<IConfigurationImpl>> results;
    if (path.empty() || path == ".") return results;

    auto tokens = ConfigurationFile::split(path);
    std::string lastToken = tokens.back();
    tokens.pop_back();

    auto parent = _node;
    for (const auto& t : tokens) {
        if (!parent || parent->type != YamlNode::Type::Map) return results;
        parent = parent->get(t);
    }
    if (!parent || parent->type != YamlNode::Type::Map) return results;

    auto target = parent->get(lastToken);
    if (!target) return results;

    if (target->type == YamlNode::Type::Sequence) {
        for (const auto& item : target->seqItems) {
            results.push_back(std::make_unique<YAMLConfigurationImpl>(_doc, item));
        }
    }
    else {
        results.push_back(std::make_unique<YAMLConfigurationImpl>(_doc, target));
    }

    return results;
}

std::string YAMLConfigurationImpl::getRawValue(const std::string& path) {
    auto target = navigateTo(path);
    if (!target || target->type != YamlNode::Type::Scalar) return "";
    return target->scalar;
}

void YAMLConfigurationImpl::setValueInternal(const std::string& path, const std::string& value) {
    if (path.empty() || path == ".") {
        _node->type = YamlNode::Type::Scalar;
        _node->scalar = value;
        _node->mapItems.clear();
        _node->seqItems.clear();
        return;
    }

    std::string lastToken;
    auto parent = navigateToParent(path, lastToken);
    if (!parent) return;

    auto existing = parent->get(lastToken);
    if (existing) {
        existing->type = YamlNode::Type::Scalar;
        existing->scalar = value;
        existing->mapItems.clear();
        existing->seqItems.clear();
    }
    else {
        parent->set(lastToken, YamlNode::makeScalar(value));
    }
}

std::unique_ptr<IConfigurationImpl> YAMLConfigurationImpl::appendNodeInternal(const std::string& name, const std::string& value) {
    if (!_node || _node->type != YamlNode::Type::Map) return nullptr;

    auto newNode = value.empty() ? YamlNode::makeMap() : YamlNode::makeScalar(value);
    auto existing = _node->get(name);

    if (!existing) {
        _node->set(name, newNode);
        return std::make_unique<YAMLConfigurationImpl>(_doc, newNode);
    }

    if (existing->type != YamlNode::Type::Sequence) {
        auto seq = YamlNode::makeSequence();
        seq->seqItems.push_back(existing);
        seq->seqItems.push_back(newNode);
        _node->set(name, seq);
        return std::make_unique<YAMLConfigurationImpl>(_doc, newNode);
    }

    existing->seqItems.push_back(newNode);
    return std::make_unique<YAMLConfigurationImpl>(_doc, newNode);
}

std::unique_ptr<IConfigurationImpl> YAMLConfigurationImpl::createGroupInternal(const std::string& name) {
    return appendNodeInternal(name, "");
}

std::vector<std::string> YAMLConfigurationImpl::getChildrenNames() {
    std::vector<std::string> names;
    if (!_node || _node->type != YamlNode::Type::Map) return names;
    for (const auto& p : _node->mapItems) {
        if (std::find(names.begin(), names.end(), p.first) == names.end()) {
            names.push_back(p.first);
        }
    }
    return names;
}

std::string YAMLConfigurationImpl::serialize(ConfigType) {
    std::ostringstream out;
    writeNode(out, _node, 0, false);
    return out.str();
}

bool YAMLConfigurationImpl::saveToFileInternal(const std::string& path) {
    std::ofstream fout(path);
    if (!fout.is_open()) return false;
    writeNode(fout, _doc, 0, false);
    return true;
}