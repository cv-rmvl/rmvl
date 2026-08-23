#include "rmvl/core/yaml.hpp"

#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <system_error>

#include "c4/yml/std/string.hpp"
#include "c4/yml/yml.hpp"

namespace rm::yaml {

namespace {

class RymlError final : public std::runtime_error {
public:
    RymlError(std::string message, std::size_t line = 0, std::size_t column = 0)
        : std::runtime_error(std::move(message)), line(line), column(column) {}

    std::size_t line;
    std::size_t column;
};

std::string toString(c4::csubstr value) { return {value.str, value.len}; }

c4::csubstr toSubstr(std::string_view value) noexcept { return {value.data(), value.size()}; }

std::string_view normalizeOpenCvYaml(std::string_view source) noexcept {
    if (source.rfind("%YAML:", 0) != 0)
        return source;
    const auto line_end = source.find('\n');
    source = line_end == std::string_view::npos ? std::string_view{} : source.substr(line_end + 1);
    if (source.rfind("---", 0) == 0) {
        const auto document_end = source.find('\n');
        source = document_end == std::string_view::npos ? std::string_view{} : source.substr(document_end + 1);
    }
    return source;
}

[[noreturn]] void onBasicError(c4::csubstr message, const c4::yml::ErrorDataBasic &, void *) {
    throw RymlError(toString(message));
}

[[noreturn]] void onParseError(c4::csubstr message, const c4::yml::ErrorDataParse &data, void *) {
    const auto line = data.ymlloc.line == c4::yml::npos ? 0 : data.ymlloc.line;
    const auto column = data.ymlloc.col == c4::yml::npos ? 0 : data.ymlloc.col;
    throw RymlError(toString(message), line, column);
}

[[noreturn]] void onVisitError(c4::csubstr message, const c4::yml::ErrorDataVisit &, void *) {
    throw RymlError(toString(message));
}

c4::yml::Callbacks makeCallbacks() {
    c4::yml::Callbacks callbacks;
    callbacks.set_error_basic(onBasicError).set_error_parse(onParseError).set_error_visit(onVisitError);
    return callbacks;
}

std::string errorMessage(std::string_view action, std::string_view path) {
    std::string message(action);
    message.append(": ");
    message.append(path);
    return message;
}

bool saveText(std::string_view path, std::string_view source, Error &err) {
    const std::string file_path(path);
    std::FILE *file = std::fopen(file_path.c_str(), "wb");
    if (!file) {
        err = {ErrorCode::Io, errorMessage("failed to open YAML file", path), 0, 0};
        return false;
    }
    const auto written = source.empty() ? 0 : std::fwrite(source.data(), 1, source.size(), file);
    const bool stream_error = written != source.size() || std::ferror(file) != 0;
    const bool close_error = std::fclose(file) != 0;
    if (stream_error || close_error) {
        err = {ErrorCode::Io, errorMessage("failed to write YAML file", path), 0, 0};
        return false;
    }
    return true;
}

#ifdef HAVE_OPENCV

char depthCode(int depth) noexcept {
    switch (depth) {
    case CV_8U: return 'u';
    case CV_8S: return 'c';
    case CV_16U: return 'w';
    case CV_16S: return 's';
    case CV_32S: return 'i';
    case CV_32F: return 'f';
    case CV_64F: return 'd';
    case CV_16F: return 'h';
    default: return '\0';
    }
}

int codeDepth(char code) noexcept {
    switch (code) {
    case 'u': return CV_8U;
    case 'c': return CV_8S;
    case 'w': return CV_16U;
    case 's': return CV_16S;
    case 'i': return CV_32S;
    case 'f': return CV_32F;
    case 'd': return CV_64F;
    case 'h': return CV_16F;
    default: return -1;
    }
}

#endif

} // namespace

#ifdef HAVE_OPENCV

namespace opencv_detail {

std::string dataType(int type) {
    const int channels = CV_MAT_CN(type);
    const char code = depthCode(CV_MAT_DEPTH(type));
    if (code == '\0' || channels < 1)
        return {};
    return channels == 1 ? std::string(1, code) : std::to_string(channels) + code;
}

bool parseDataType(std::string_view value, int &type) noexcept {
    if (value.empty())
        return false;
    int channels = 1;
    if (value.size() > 1) {
        const auto result = std::from_chars(value.data(), value.data() + value.size() - 1, channels);
        if (result.ec != std::errc{} || result.ptr != value.data() + value.size() - 1)
            return false;
    }
    const int depth = codeDepth(value.back());
    if (depth < 0 || channels < 1 || channels > CV_CN_MAX)
        return false;
    type = CV_MAKETYPE(depth, channels);
    return true;
}

bool matrixTag(const Node &node, bool nd) noexcept {
    const auto tag = node.tag();
    if (tag.empty())
        return true;
    return tag.find(nd ? "opencv-nd-matrix" : "opencv-matrix") != std::string_view::npos;
}

bool encodeMatrixData(Node &data, const cv::Mat &matrix) {
    switch (matrix.depth()) {
    case CV_8U: return encodeMatrixData<uchar>(data, matrix);
    case CV_8S: return encodeMatrixData<schar>(data, matrix);
    case CV_16U: return encodeMatrixData<ushort>(data, matrix);
    case CV_16S: return encodeMatrixData<short>(data, matrix);
    case CV_32S: return encodeMatrixData<int>(data, matrix);
    case CV_32F: return encodeMatrixData<float>(data, matrix);
    case CV_64F: return encodeMatrixData<double>(data, matrix);
    case CV_16F: {
        cv::Mat converted;
        matrix.convertTo(converted, CV_32F);
        return encodeMatrixData<float>(data, converted);
    }
    default: return false;
    }
}

bool decodeMatrixData(const Node &data, cv::Mat &matrix) {
    switch (matrix.depth()) {
    case CV_8U: return decodeMatrixData<uchar>(data, matrix);
    case CV_8S: return decodeMatrixData<schar>(data, matrix);
    case CV_16U: return decodeMatrixData<ushort>(data, matrix);
    case CV_16S: return decodeMatrixData<short>(data, matrix);
    case CV_32S: return decodeMatrixData<int>(data, matrix);
    case CV_32F: return decodeMatrixData<float>(data, matrix);
    case CV_64F: return decodeMatrixData<double>(data, matrix);
    case CV_16F: {
        cv::Mat converted;
        matrix.convertTo(converted, CV_32F);
        if (!decodeMatrixData<float>(data, converted))
            return false;
        converted.convertTo(matrix, CV_16F);
        return true;
    }
    default: return false;
    }
}

} // namespace opencv_detail

#endif

struct Node::Impl {
    c4::yml::Tree tree{makeCallbacks()};

    Impl() = default;
    explicit Impl(c4::yml::Tree &&source) noexcept : tree(std::move(source)) {}
};

Node Node::createMap() {
    auto impl = std::make_shared<Impl>();
    Node node{impl, impl->tree.root_id()};
    node.makeMap();
    return node;
}

Node Node::createSequence() {
    auto impl = std::make_shared<Impl>();
    Node node{impl, impl->tree.root_id()};
    node.makeSequence();
    return node;
}

Node Node::createScalar(std::string_view value) {
    auto impl = std::make_shared<Impl>();
    Node node{impl, impl->tree.root_id()};
    node.setScalar(value);
    return node;
}

bool Node::valid() const noexcept { return _impl && _id != c4::yml::NONE && _id < _impl->tree.capacity(); }

NodeType Node::type() const noexcept {
    if (!valid())
        return NodeType::Undefined;
    const auto &tree = _impl->tree;
    if (tree.is_map(_id))
        return NodeType::Map;
    if (tree.is_seq(_id))
        return NodeType::Sequence;
    if (!tree.has_val(_id) || tree.val_is_null(_id))
        return NodeType::Null;
    return NodeType::Scalar;
}

std::size_t Node::size() const noexcept {
    return valid() && (isMap() || isSequence()) ? _impl->tree.num_children(_id) : 0;
}

std::string_view Node::key() const noexcept {
    if (!valid() || !_impl->tree.has_key(_id))
        return {};
    const auto value = _impl->tree.key(_id);
    return {value.str, value.len};
}

std::string_view Node::scalar() const noexcept {
    if (!isScalar())
        return {};
    const auto value = _impl->tree.val(_id);
    return {value.str, value.len};
}

std::string_view Node::tag() const noexcept {
    if (!valid() || !_impl->tree.has_val_tag(_id))
        return {};
    const auto value = _impl->tree.val_tag(_id);
    return {value.str, value.len};
}

bool Node::contains(std::string_view name) const noexcept {
    return isMap() && _impl->tree.find_child(_id, toSubstr(name)) != c4::yml::NONE;
}

Node Node::operator[](std::string_view name) const noexcept {
    if (!isMap())
        return {};
    const auto child = _impl->tree.find_child(_id, toSubstr(name));
    return child == c4::yml::NONE ? Node{} : Node{_impl, child};
}

Node Node::operator[](std::size_t index) const noexcept {
    if (!valid() || (!isMap() && !isSequence()))
        return {};
    const auto child = _impl->tree.child(_id, index);
    return child == c4::yml::NONE ? Node{} : Node{_impl, child};
}

std::vector<std::string_view> Node::keys() const {
    std::vector<std::string_view> result;
    if (!isMap())
        return result;
    result.reserve(size());
    for (auto child = _impl->tree.first_child(_id); child != c4::yml::NONE; child = _impl->tree.next_sibling(child)) {
        const auto name = _impl->tree.key(child);
        result.emplace_back(name.str, name.len);
    }
    return result;
}

Node Node::ensure(std::string_view name) {
    if (!valid())
        return {};
    if (!isMap())
        makeMap();
    if (const auto child = _impl->tree.find_child(_id, toSubstr(name)); child != c4::yml::NONE)
        return {_impl, child};
    const auto child = _impl->tree.append_child(_id);
    _impl->tree.set_key(child, _impl->tree.copy_to_arena(toSubstr(name)));
    _impl->tree.set_val(child, _impl->tree.copy_to_arena("~"));
    return {_impl, child};
}

Node Node::append() {
    if (!valid())
        return {};
    if (!isSequence())
        makeSequence();
    const auto child = _impl->tree.append_child(_id);
    _impl->tree.set_val(child, _impl->tree.copy_to_arena("~"));
    return {_impl, child};
}

bool Node::erase(std::string_view name) {
    if (!isMap())
        return false;
    const auto child = _impl->tree.find_child(_id, toSubstr(name));
    if (child == c4::yml::NONE)
        return false;
    _impl->tree.remove(child);
    return true;
}

bool Node::erase(std::size_t index) {
    if (!valid() || (!isMap() && !isSequence()))
        return false;
    const auto child = _impl->tree.child(_id, index);
    if (child == c4::yml::NONE)
        return false;
    _impl->tree.remove(child);
    return true;
}

void Node::setNull() { setScalar("~"); }

void Node::makeMap() {
    if (!valid())
        return;
    _impl->tree.remove_children(_id);
    _impl->tree.change_type(_id, _impl->tree.has_key(_id) ? c4::yml::KEYMAP : c4::yml::MAP);
}

void Node::makeSequence() {
    if (!valid())
        return;
    _impl->tree.remove_children(_id);
    _impl->tree.change_type(_id, _impl->tree.has_key(_id) ? c4::yml::KEYSEQ : c4::yml::SEQ);
}

void Node::setScalar(std::string_view value) {
    if (!valid())
        return;
    _impl->tree.remove_children(_id);
    _impl->tree.change_type(_id, _impl->tree.has_key(_id) ? c4::yml::KEYVAL : c4::yml::VAL);
    _impl->tree.set_val(_id, _impl->tree.copy_to_arena(toSubstr(value)));
}

void Node::setTag(std::string_view value) {
    if (!valid() || value.empty() || (!isScalar() && !isMap() && !isSequence()))
        return;
    _impl->tree.set_val_tag(_id, _impl->tree.copy_to_arena(toSubstr(value)));
}

bool Node::readBool(bool &value) const noexcept {
    const auto text = scalar();
    if (text == "true" || text == "True" || text == "TRUE" || text == "1") {
        value = true;
        return true;
    }
    if (text == "false" || text == "False" || text == "FALSE" || text == "0") {
        value = false;
        return true;
    }
    return false;
}

bool Node::readSigned(int64_t &value) const noexcept {
    const auto text = scalar();
    if (text.empty())
        return false;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

bool Node::readUnsigned(uint64_t &value) const noexcept {
    const auto text = scalar();
    if (text.empty() || text.front() == '-')
        return false;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

bool Node::readFloating(double &value) const noexcept {
    const auto text = scalar();
    if (text.empty())
        return false;
    const std::string copy(text);
    char *end{};
    errno = 0;
    const auto parsed = std::strtod(copy.c_str(), &end);
    if (errno == ERANGE || end != copy.c_str() + copy.size() || !std::isfinite(parsed))
        return false;
    value = parsed;
    return true;
}

bool Node::writeBool(bool value) {
    setScalar(value ? "true" : "false");
    return true;
}

bool Node::writeSigned(int64_t value) {
    char buffer[32];
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    if (result.ec != std::errc{})
        return false;
    setScalar({buffer, static_cast<std::size_t>(result.ptr - buffer)});
    return true;
}

bool Node::writeUnsigned(uint64_t value) {
    char buffer[32];
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    if (result.ec != std::errc{})
        return false;
    setScalar({buffer, static_cast<std::size_t>(result.ptr - buffer)});
    return true;
}

bool Node::writeFloating(double value) {
    if (!std::isfinite(value))
        return false;
    char buffer[64];
    const int length = std::snprintf(buffer, sizeof(buffer), "%.17g", value);
    if (length < 0 || static_cast<std::size_t>(length) >= sizeof(buffer))
        return false;
    setScalar({buffer, static_cast<std::size_t>(length)});
    return true;
}

Result parse(std::string_view source) {
    try {
        source = normalizeOpenCvYaml(source);
        const auto callbacks = makeCallbacks();
        c4::yml::Tree tree(callbacks);
        tree.reserve(c4::yml::estimate_tree_capacity(toSubstr(source)));
        c4::yml::EventHandlerTree handler(&tree, tree.root_id());
        c4::yml::Parser parser(&handler);
        c4::yml::parse_in_arena(&parser, toSubstr(source), &tree);
        auto impl = std::make_shared<Node::Impl>(std::move(tree));
        return {Node{impl, impl->tree.root_id()}, {}};
    } catch (const RymlError &error) {
        return {{}, {ErrorCode::Parse, error.what(), error.line, error.column}};
    } catch (const std::exception &error) {
        return {{}, {ErrorCode::Parse, error.what(), 0, 0}};
    }
}

Result load(std::string_view path) {
    const std::string file_path(path);
    std::FILE *file = std::fopen(file_path.c_str(), "rb");
    if (!file)
        return {{}, {ErrorCode::Io, errorMessage("failed to open YAML file", path), 0, 0}};

    if (std::fseek(file, 0, SEEK_END) != 0) {
        std::fclose(file);
        return {{}, {ErrorCode::Io, errorMessage("failed to seek YAML file", path), 0, 0}};
    }
    const long length = std::ftell(file);
    if (length < 0 || std::fseek(file, 0, SEEK_SET) != 0) {
        std::fclose(file);
        return {{}, {ErrorCode::Io, errorMessage("failed to inspect YAML file", path), 0, 0}};
    }

    std::string source(static_cast<std::size_t>(length), '\0');
    const auto read_size = source.empty() ? 0 : std::fread(source.data(), 1, source.size(), file);
    const bool read_error = read_size != source.size() || std::ferror(file) != 0;
    std::fclose(file);
    if (read_error)
        return {{}, {ErrorCode::Io, errorMessage("failed to read YAML file", path), 0, 0}};
    return parse(source);
}

std::string dump(const Node &node) {
    if (!node.valid())
        return {};
    try {
        return c4::yml::emitrs_yaml<std::string>(node._impl->tree, node._id);
    } catch (const RymlError &) {
        return {};
    }
}

bool save(std::string_view path, const Node &node, Error &err) {
    err = {};
    if (!node.valid()) {
        err = {ErrorCode::Io, "cannot save an invalid YAML node", 0, 0};
        return false;
    }

    const auto source = dump(node);
    return saveText(path, source, err);
}

#ifdef HAVE_OPENCV

std::string dumpOpenCv(const Node &node) {
    auto source = dump(node);
    if (source.empty())
        return {};
    source.insert(0, "%YAML:1.0\n---\n");
    return source;
}

bool saveOpenCv(std::string_view path, const Node &node) {
    if (!node.valid())
        return false;
    Error err;
    return saveText(path, dumpOpenCv(node), err);
}

#endif

} // namespace rm::yaml
