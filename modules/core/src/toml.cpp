#include "rmvl/core/toml.hpp"

#include <cstdio>
#include <memory>

#define TOML_HEADER_ONLY 1
#define TOML_EXCEPTIONS 0
#define TOML_ENABLE_FORMATTERS 0
#define TOML_ENABLE_UNRELEASED_FEATURES 0
#define TOML_LARGE_FILES 1
#include <toml++/toml.hpp>

namespace rm::toml {

struct Node::Impl {
    ::toml::table root;
    explicit Impl(::toml::table table) : root(std::move(table)) {}
};

NodeType Node::type() const noexcept {
    if (!valid())
        return NodeType::Undefined;
    switch (static_cast<const ::toml::node *>(_node)->type()) {
    case ::toml::node_type::table: return NodeType::Table;
    case ::toml::node_type::array: return NodeType::Array;
    case ::toml::node_type::string: return NodeType::String;
    case ::toml::node_type::integer: return NodeType::Integer;
    case ::toml::node_type::floating_point: return NodeType::Floating;
    case ::toml::node_type::boolean: return NodeType::Boolean;
    case ::toml::node_type::date: return NodeType::Date;
    case ::toml::node_type::time: return NodeType::Time;
    case ::toml::node_type::date_time: return NodeType::DateTime;
    default: return NodeType::Undefined;
    }
}

std::size_t Node::size() const noexcept {
    if (isTable())
        return static_cast<const ::toml::node *>(_node)->as_table()->size();
    if (isArray())
        return static_cast<const ::toml::node *>(_node)->as_array()->size();
    return 0;
}

bool Node::contains(std::string_view key) const noexcept { return (*this)[key].valid(); }

Node Node::operator[](std::string_view key) const noexcept {
    if (!isTable())
        return {};
    const auto *child = static_cast<const ::toml::node *>(_node)->as_table()->get(key);
    return child ? Node(_impl, child) : Node{};
}

Node Node::operator[](std::size_t index) const noexcept {
    if (!isArray() || index >= size())
        return {};
    return Node(_impl, static_cast<const ::toml::node *>(_node)->as_array()->get(index));
}

std::vector<std::string_view> Node::keys() const {
    std::vector<std::string_view> result;
    if (!isTable())
        return result;
    result.reserve(size());
    for (const auto &[key, value] : *static_cast<const ::toml::node *>(_node)->as_table())
        result.push_back(key.str());
    return result;
}

bool Node::readValue(std::string &value) const {
    if (type() != NodeType::String)
        return false;
    value = static_cast<const ::toml::node *>(_node)->as_string()->get();
    return true;
}

bool Node::readValue(bool &value) const noexcept {
    if (type() != NodeType::Boolean)
        return false;
    value = static_cast<const ::toml::node *>(_node)->as_boolean()->get();
    return true;
}

bool Node::readValue(int64_t &value) const noexcept {
    if (type() != NodeType::Integer)
        return false;
    value = static_cast<const ::toml::node *>(_node)->as_integer()->get();
    return true;
}

bool Node::readValue(double &value) const noexcept {
    if (type() != NodeType::Floating)
        return false;
    value = static_cast<const ::toml::node *>(_node)->as_floating_point()->get();
    return true;
}

namespace {
struct FileCloser {
    void operator()(std::FILE *file) const noexcept { std::fclose(file); }
};
Date convert(const ::toml::date &value) { return {value.year, value.month, value.day}; }
Time convert(const ::toml::time &value) { return {value.hour, value.minute, value.second, value.nanosecond}; }
} // namespace

bool Node::readValue(Date &value) const noexcept {
    if (type() != NodeType::Date)
        return false;
    value = convert(static_cast<const ::toml::node *>(_node)->as_date()->get());
    return true;
}

bool Node::readValue(Time &value) const noexcept {
    if (type() != NodeType::Time)
        return false;
    value = convert(static_cast<const ::toml::node *>(_node)->as_time()->get());
    return true;
}

bool Node::readValue(DateTime &value) const noexcept {
    if (type() != NodeType::DateTime)
        return false;
    const auto &raw = static_cast<const ::toml::node *>(_node)->as_date_time()->get();
    value = {convert(raw.date), convert(raw.time), std::nullopt};
    if (raw.offset)
        value.offsetMinutes = raw.offset->minutes;
    return true;
}

Node loads(std::string_view source, Error *error) {
    if (error)
        *error = {};
    auto result = ::toml::parse(source);
    if (!result) {
        if (error) {
            const auto &reason = result.error();
            *error = {ErrorCode::Parse, std::string(reason.description()),
                      reason.source().begin.line, reason.source().begin.column};
        }
        return {};
    }
    auto impl = std::make_shared<Node::Impl>(std::move(result).table());
    const auto *root = &impl->root;
    return Node(std::move(impl), root);
}

Node load(std::string_view path, Error *error) {
    if (error)
        *error = {};
    if (path.find('\0') != std::string_view::npos) {
        if (error)
            *error = {ErrorCode::Io, "TOML file path contains a null character", 0, 0};
        return {};
    }
    const std::string filename(path);
    std::unique_ptr<std::FILE, FileCloser> file(std::fopen(filename.c_str(), "rb"));
    if (!file) {
        if (error)
            *error = {ErrorCode::Io, "failed to open TOML file: " + filename, 0, 0};
        return {};
    }
    std::string source;
    char buffer[8192];
    while (const auto count = std::fread(buffer, 1, sizeof(buffer), file.get()))
        source.append(buffer, count);
    if (std::ferror(file.get())) {
        if (error)
            *error = {ErrorCode::Io, "failed to read TOML file: " + filename, 0, 0};
        return {};
    }
    return loads(source, error);
}

} // namespace rm::toml
