/**
 * @file yaml.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief YAML 树模型、类型转换和文件读写
 * @version 1.0
 * @date 2026-08-21
 *
 * @copyright Copyright 2026 (c), zhaoxi
 */

#pragma once

#include <rmvl/rmvl_modules.hpp>

#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>

#ifdef HAVE_OPENCV
#include <charconv>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/traits.hpp>
#else
#include <array>
#include <string>
#endif

#include "rmvl/core/rmvldef.hpp"

namespace rm::yaml {

//! @defgroup core_yaml YAML 解析模块
//! @ingroup core
//! @{
//! @brief 提供基于 rapidyaml 的高性能 YAML 解析、树访问、类型转换和文件读写功能
//! @details Node 是共享底层 YAML 树所有权的轻量句柄。子节点可安全脱离根节点保存，但同一棵树不提供内部线程同步。

//! YAML 节点类型
enum class NodeType : uint8_t {
    Undefined, //!< 无效或不存在的节点
    Null,      //!< 空值
    Scalar,    //!< 标量
    Sequence,  //!< 序列
    Map,       //!< 映射
};

//! YAML 操作错误类型
enum class ErrorCode : uint8_t {
    None,  //!< 无错误
    Parse, //!< YAML 语法错误
    Io,    //!< 文件读写错误
};

//! YAML 操作错误
struct Error {
    ErrorCode code{ErrorCode::None}; //!< 错误类型
    std::string message{};           //!< 错误描述
    std::size_t line{};              //!< 语法错误行号，不适用时为 0
    std::size_t column{};            //!< 语法错误列号，不适用时为 0

    //! @return 是否包含错误
    explicit operator bool() const noexcept { return code != ErrorCode::None; }
};

struct Result;

/**
 * @brief YAML 树节点句柄
 * @details
 * - 拷贝 Node 仅复制共享所有权和节点标识，不复制 YAML 数据
 * - scalar()、key() 和 keys() 返回的视图会在同一棵树发生写入后失效
 * - 删除节点后，指向被删除节点及其后代的既有句柄不可继续使用
 * @note 同一棵树上的并发写入，或读写并发，需要由调用方同步。
 */
class RMVL_EXPORTS_W Node {
public:
    //! 构造无效节点
    Node() noexcept = default;

    //! 创建空映射根节点
    static Node createMap();

    //! 创建空序列根节点
    static Node createSequence();

    /**
     * @brief 创建标量根节点
     *
     * @param[in] value 标量文本
     * @return 新建的标量节点
     */
    static Node createScalar(std::string_view value);

    //! @return 节点是否有效
    bool valid() const noexcept;

    //! @return 节点类型
    NodeType type() const noexcept;

    //! @return 是否为空值节点
    bool isNull() const noexcept { return type() == NodeType::Null; }

    //! @return 是否为标量节点
    bool isScalar() const noexcept { return type() == NodeType::Scalar; }

    //! @return 是否为序列节点
    bool isSequence() const noexcept { return type() == NodeType::Sequence; }

    //! @return 是否为映射节点
    bool isMap() const noexcept { return type() == NodeType::Map; }

    //! @return 子节点数量，非容器节点返回 0
    std::size_t size() const noexcept;

    //! @return 节点键名，根节点和序列元素返回空视图
    std::string_view key() const noexcept;

    //! @return 标量文本，非标量和空值节点返回空视图
    std::string_view scalar() const noexcept;

    //! @return 节点标签，未设置标签时返回空视图
    std::string_view tag() const noexcept;

    /**
     * @brief 判断映射是否包含指定键
     *
     * @param[in] name 键名
     * @return 是否存在对应子节点
     */
    bool contains(std::string_view name) const noexcept;

    /**
     * @brief 查找映射子节点
     *
     * @param[in] name 键名
     * @return 找到时返回子节点，否则返回无效节点
     */
    Node operator[](std::string_view name) const noexcept;

    /**
     * @brief 获取指定位置的容器子节点
     *
     * @param[in] index 从 0 开始的子节点位置
     * @return 索引有效时返回子节点，否则返回无效节点
     */
    Node operator[](std::size_t index) const noexcept;

    //! @return 映射的全部键名，非映射节点返回空数组
    std::vector<std::string_view> keys() const;

    /**
     * @brief 获取或创建映射子节点
     *
     * @param[in] name 键名
     * @return 对应子节点，当前节点无效时返回无效节点
     * @note 当前节点不是映射时，其原有内容会被替换为空映射。
     */
    Node ensure(std::string_view name);

    /**
     * @brief 向序列末尾添加空值节点
     *
     * @return 新增子节点，当前节点无效时返回无效节点
     * @note 当前节点不是序列时，其原有内容会被替换为空序列。
     */
    Node append();

    /**
     * @brief 删除映射子节点
     *
     * @param[in] name 键名
     * @return 是否找到并删除节点
     */
    bool erase(std::string_view name);

    /**
     * @brief 删除指定位置的容器子节点
     *
     * @param[in] index 从 0 开始的子节点位置
     * @return 是否找到并删除节点
     */
    bool erase(std::size_t index);

    //! 将节点替换为空值
    void setNull();

    //! 将节点替换为空映射
    void makeMap();

    //! 将节点替换为空序列
    void makeSequence();

    /**
     * @brief 将节点替换为标量
     *
     * @param[in] value 标量文本
     */
    void setScalar(std::string_view value);

    /**
     * @brief 设置节点标签
     *
     * @param[in] value 标签文本，例如 `!!opencv-matrix`
     * @note 当前节点需要是标量或容器节点。
     */
    void setTag(std::string_view value);

    /**
     * @brief 读取并转换节点值
     *
     * @tparam T 目标类型，支持标量、枚举、optional、vector、array、以 string 为键的 map/unordered_map 和 ADL 自定义类型
     * @param[out] value 目标对象
     * @return 是否转换成功
     */
    template <typename T>
    bool read(T &value) const;

    /**
     * @brief 读取并转换节点值
     *
     * @tparam T 目标类型
     * @return 成功时返回目标值，否则返回 std::nullopt
     */
    template <typename T>
    std::optional<T> as() const;

    /**
     * @brief 读取节点值，失败时返回默认值
     *
     * @tparam T 目标类型
     * @param[in] fallback 默认值
     * @return 转换结果或默认值
     */
    template <typename T>
    T valueOr(T fallback) const;

    /**
     * @brief 写入任意受支持类型
     *
     * @tparam T 输入类型
     * @param[in] value 输入值
     * @return 是否写入成功
     */
    template <typename T>
    bool write(const T &value);

    /**
     * @brief 设置映射字段
     *
     * @tparam T 输入类型
     * @param[in] name 键名
     * @param[in] value 输入值
     * @return 是否写入成功
     */
    template <typename T>
    bool set(std::string_view name, const T &value);

    /**
     * @brief 向序列追加一个值
     *
     * @tparam T 输入类型
     * @param[in] value 输入值
     * @return 是否写入成功
     */
    template <typename T>
    bool push(const T &value);

private:
    struct Impl;
    std::shared_ptr<Impl> _impl{};
    std::size_t _id{std::numeric_limits<std::size_t>::max()};

    Node(std::shared_ptr<Impl> impl, std::size_t id) noexcept : _impl(std::move(impl)), _id(id) {}

    bool readBool(bool &value) const noexcept;
    bool readSigned(int64_t &value) const noexcept;
    bool readUnsigned(uint64_t &value) const noexcept;
    bool readFloating(double &value) const noexcept;
    bool writeBool(bool value);
    bool writeSigned(int64_t value);
    bool writeUnsigned(uint64_t value);
    bool writeFloating(double value);

    friend struct Result;
    friend Result parse(std::string_view source);
    friend Result load(std::string_view path);
    friend std::string dump(const Node &node);
};

//! YAML 解析或加载结果
struct Result {
    Node root{};   //!< 成功时的根节点
    Error error{}; //!< 失败时的错误

    //! @return 操作是否成功
    explicit operator bool() const noexcept { return root.valid() && !error; }

    //! @return 根节点的常指针
    const Node *operator->() const noexcept { return &root; }

    //! @return 根节点指针
    Node *operator->() noexcept { return &root; }

    //! @return 根节点的常引用
    const Node &operator*() const noexcept { return root; }

    //! @return 根节点引用
    Node &operator*() noexcept { return root; }
};

/**
 * @brief 从内存解析 YAML
 *
 * @param[in] source YAML 文本
 * @return 解析结果
 */
RMVL_EXPORTS_W Result parse(std::string_view source);

/**
 * @brief 从文件加载 YAML
 *
 * @param[in] path 文件路径
 * @return 加载与解析结果
 */
RMVL_EXPORTS_W Result load(std::string_view path);

/**
 * @brief 将节点及其后代序列化为 YAML
 *
 * @param[in] node 待序列化节点
 * @return YAML 文本，节点无效时返回空字符串
 */
RMVL_EXPORTS_W std::string dump(const Node &node);

/**
 * @brief 将节点保存为 YAML 文件并返回错误信息
 *
 * @param[in] path 文件路径
 * @param[in] node 待保存节点
 * @param[out] err 错误信息，保存成功时重置为空错误
 * @return 是否保存成功
 */
RMVL_EXPORTS_W bool save(std::string_view path, const Node &node, Error &err);

/**
 * @brief 将节点保存为 YAML 文件
 *
 * @param[in] path 文件路径
 * @param[in] node 待保存节点
 * @return 是否保存成功
 */
RMVL_EXPORTS_W inline bool save(std::string_view path, const Node &node) {
    Error err;
    return save(path, node, err);
}

#ifdef HAVE_OPENCV

//! @addtogroup core_yaml
//! @{

/**
 * @brief 序列化为 OpenCV FileStorage 兼容的 YAML 文本
 *
 * @param[in] node 待序列化节点
 * @return 带 OpenCV YAML 版本头的文本，节点无效时返回空字符串
 */
RMVL_EXPORTS_W std::string dumpOpenCv(const Node &node);

/**
 * @brief 保存为 OpenCV FileStorage 兼容的 YAML 文件
 *
 * @param[in] path 文件路径
 * @param[in] node 待保存节点
 * @return 是否保存成功
 */
RMVL_EXPORTS_W bool saveOpenCv(std::string_view path, const Node &node);

namespace opencv_detail {

template <typename T, std::size_t N>
bool decodeSequence(const Node &node, T (&values)[N]) {
    if (!node.isSequence() || node.size() != N)
        return false;
    for (std::size_t i = 0; i < N; ++i)
        if (!node[i].read(values[i]))
            return false;
    return true;
}

template <typename T>
bool encodeSequence(Node &node, const T *values, std::size_t size) {
    node.makeSequence();
    for (std::size_t i = 0; i < size; ++i)
        if (!node.push(values[i]))
            return false;
    return true;
}

inline char depthCode(int depth) noexcept {
    switch (depth) {
    case CV_8U:
        return 'u';
    case CV_8S:
        return 'c';
    case CV_16U:
        return 'w';
    case CV_16S:
        return 's';
    case CV_32S:
        return 'i';
    case CV_32F:
        return 'f';
    case CV_64F:
        return 'd';
    case CV_16F:
        return 'h';
    default:
        return '\0';
    }
}

inline int codeDepth(char code) noexcept {
    switch (code) {
    case 'u':
        return CV_8U;
    case 'c':
        return CV_8S;
    case 'w':
        return CV_16U;
    case 's':
        return CV_16S;
    case 'i':
        return CV_32S;
    case 'f':
        return CV_32F;
    case 'd':
        return CV_64F;
    case 'h':
        return CV_16F;
    default:
        return -1;
    }
}

inline std::string dataType(int type) {
    const int channels = CV_MAT_CN(type);
    const char code = depthCode(CV_MAT_DEPTH(type));
    if (code == '\0' || channels < 1)
        return {};
    return channels == 1 ? std::string(1, code) : std::to_string(channels) + code;
}

inline bool parseDataType(std::string_view value, int &type) noexcept {
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

inline bool matrixTag(const Node &node, bool nd) noexcept {
    const auto tag = node.tag();
    if (tag.empty())
        return true;
    return tag.find(nd ? "opencv-nd-matrix" : "opencv-matrix") != std::string_view::npos;
}

template <typename T>
bool encodeMatrixData(Node &data, const cv::Mat &matrix) {
    return encodeSequence(data, matrix.ptr<T>(), matrix.total() * matrix.channels());
}

inline bool encodeMatrixData(Node &data, const cv::Mat &matrix) {
    switch (matrix.depth()) {
    case CV_8U:
        return encodeMatrixData<uchar>(data, matrix);
    case CV_8S:
        return encodeMatrixData<schar>(data, matrix);
    case CV_16U:
        return encodeMatrixData<ushort>(data, matrix);
    case CV_16S:
        return encodeMatrixData<short>(data, matrix);
    case CV_32S:
        return encodeMatrixData<int>(data, matrix);
    case CV_32F:
        return encodeMatrixData<float>(data, matrix);
    case CV_64F:
        return encodeMatrixData<double>(data, matrix);
    case CV_16F: {
        cv::Mat converted;
        matrix.convertTo(converted, CV_32F);
        return encodeMatrixData<float>(data, converted);
    }
    default:
        return false;
    }
}

template <typename T>
bool decodeMatrixData(const Node &data, cv::Mat &matrix) {
    const auto count = matrix.total() * matrix.channels();
    if (!data.isSequence() || data.size() != count)
        return false;
    auto *values = matrix.ptr<T>();
    for (std::size_t i = 0; i < count; ++i)
        if (!data[i].read(values[i]))
            return false;
    return true;
}

inline bool decodeMatrixData(const Node &data, cv::Mat &matrix) {
    switch (matrix.depth()) {
    case CV_8U:
        return decodeMatrixData<uchar>(data, matrix);
    case CV_8S:
        return decodeMatrixData<schar>(data, matrix);
    case CV_16U:
        return decodeMatrixData<ushort>(data, matrix);
    case CV_16S:
        return decodeMatrixData<short>(data, matrix);
    case CV_32S:
        return decodeMatrixData<int>(data, matrix);
    case CV_32F:
        return decodeMatrixData<float>(data, matrix);
    case CV_64F:
        return decodeMatrixData<double>(data, matrix);
    case CV_16F: {
        cv::Mat converted;
        matrix.convertTo(converted, CV_32F);
        if (!decodeMatrixData<float>(data, converted))
            return false;
        converted.convertTo(matrix, CV_16F);
        return true;
    }
    default:
        return false;
    }
}

} // namespace opencv_detail

template <typename T>
bool yaml_decode(const Node &node, cv::Point_<T> &value) {
    T data[2]{};
    if (!opencv_detail::decodeSequence(node, data))
        return false;
    value = {data[0], data[1]};
    return true;
}

template <typename T>
bool yaml_encode(Node &node, const cv::Point_<T> &value) {
    const T data[]{value.x, value.y};
    return opencv_detail::encodeSequence(node, data, 2);
}

template <typename T>
bool yaml_decode(const Node &node, cv::Point3_<T> &value) {
    T data[3]{};
    if (!opencv_detail::decodeSequence(node, data))
        return false;
    value = {data[0], data[1], data[2]};
    return true;
}

template <typename T>
bool yaml_encode(Node &node, const cv::Point3_<T> &value) {
    const T data[]{value.x, value.y, value.z};
    return opencv_detail::encodeSequence(node, data, 3);
}

template <typename T>
bool yaml_decode(const Node &node, cv::Size_<T> &value) {
    T data[2]{};
    if (!opencv_detail::decodeSequence(node, data))
        return false;
    value = {data[0], data[1]};
    return true;
}

template <typename T>
bool yaml_encode(Node &node, const cv::Size_<T> &value) {
    const T data[]{value.width, value.height};
    return opencv_detail::encodeSequence(node, data, 2);
}

template <typename T>
bool yaml_decode(const Node &node, cv::Complex<T> &value) {
    T data[2]{};
    if (!opencv_detail::decodeSequence(node, data))
        return false;
    value = {data[0], data[1]};
    return true;
}

template <typename T>
bool yaml_encode(Node &node, const cv::Complex<T> &value) {
    const T data[]{value.re, value.im};
    return opencv_detail::encodeSequence(node, data, 2);
}

template <typename T>
bool yaml_decode(const Node &node, cv::Rect_<T> &value) {
    T data[4]{};
    if (!opencv_detail::decodeSequence(node, data))
        return false;
    value = {data[0], data[1], data[2], data[3]};
    return true;
}

template <typename T>
bool yaml_encode(Node &node, const cv::Rect_<T> &value) {
    const T data[]{value.x, value.y, value.width, value.height};
    return opencv_detail::encodeSequence(node, data, 4);
}

template <typename T, int N>
bool yaml_decode(const Node &node, cv::Vec<T, N> &value) {
    if (!node.isSequence() || node.size() != static_cast<std::size_t>(N))
        return false;
    cv::Vec<T, N> result;
    for (int i = 0; i < N; ++i)
        if (!node[static_cast<std::size_t>(i)].read(result[i]))
            return false;
    value = result;
    return true;
}

template <typename T, int N>
bool yaml_encode(Node &node, const cv::Vec<T, N> &value) {
    return opencv_detail::encodeSequence(node, value.val, N);
}

template <typename T>
bool yaml_decode(const Node &node, cv::Scalar_<T> &value) {
    T data[4]{};
    if (!opencv_detail::decodeSequence(node, data))
        return false;
    value = {data[0], data[1], data[2], data[3]};
    return true;
}

template <typename T>
bool yaml_encode(Node &node, const cv::Scalar_<T> &value) {
    return opencv_detail::encodeSequence(node, value.val, 4);
}

inline bool yaml_decode(const Node &node, cv::Range &value) {
    int data[2]{};
    if (!opencv_detail::decodeSequence(node, data))
        return false;
    value = {data[0], data[1]};
    return true;
}

inline bool yaml_encode(Node &node, const cv::Range &value) {
    const int data[]{value.start, value.end};
    return opencv_detail::encodeSequence(node, data, 2);
}

inline bool yaml_decode(const Node &node, cv::KeyPoint &value) {
    if (!node.isSequence() || node.size() != 7)
        return false;
    cv::KeyPoint result;
    if (!node[0].read(result.pt.x) || !node[1].read(result.pt.y) || !node[2].read(result.size) ||
        !node[3].read(result.angle) || !node[4].read(result.response) || !node[5].read(result.octave) ||
        !node[6].read(result.class_id))
        return false;
    value = result;
    return true;
}

inline bool yaml_encode(Node &node, const cv::KeyPoint &value) {
    node.makeSequence();
    return node.push(value.pt.x) && node.push(value.pt.y) && node.push(value.size) && node.push(value.angle) &&
           node.push(value.response) && node.push(value.octave) && node.push(value.class_id);
}

inline bool yaml_decode(const Node &node, cv::DMatch &value) {
    if (!node.isSequence() || node.size() != 4)
        return false;
    cv::DMatch result;
    if (!node[0].read(result.queryIdx) || !node[1].read(result.trainIdx) || !node[2].read(result.imgIdx) ||
        !node[3].read(result.distance))
        return false;
    value = result;
    return true;
}

inline bool yaml_encode(Node &node, const cv::DMatch &value) {
    node.makeSequence();
    return node.push(value.queryIdx) && node.push(value.trainIdx) && node.push(value.imgIdx) && node.push(value.distance);
}

inline bool yaml_decode(const Node &node, cv::Mat &value) {
    if (!node.isMap())
        return false;
    const bool nd = node.contains("sizes");
    if (!opencv_detail::matrixTag(node, nd))
        return false;

    std::string data_type;
    int type{};
    if (!node["dt"].read(data_type) || !opencv_detail::parseDataType(data_type, type))
        return false;

    cv::Mat result;
    if (nd) {
        std::vector<int> sizes;
        if (!node["sizes"].read(sizes) || sizes.size() < 2)
            return false;
        for (const int size : sizes)
            if (size < 0)
                return false;
        result.create(static_cast<int>(sizes.size()), sizes.data(), type);
    } else {
        int rows{}, cols{};
        if (!node["rows"].read(rows) || !node["cols"].read(cols) || rows < 0 || cols < 0)
            return false;
        result.create(rows, cols, type);
    }
    if (!opencv_detail::decodeMatrixData(node["data"], result))
        return false;
    value = std::move(result);
    return true;
}

inline bool yaml_encode(Node &node, const cv::Mat &value) {
    if (value.empty()) {
        node.makeMap();
        node.setTag("!!opencv-matrix");
        auto data = node.ensure("data");
        data.makeSequence();
        return node.set("rows", 0) && node.set("cols", 0) && node.set("dt", "u");
    }
    if (value.dims < 2)
        return false;
    const std::string data_type = opencv_detail::dataType(value.type());
    if (data_type.empty())
        return false;
    const cv::Mat matrix = value.isContinuous() ? value : value.clone();
    node.makeMap();
    if (matrix.dims == 2) {
        node.setTag("!!opencv-matrix");
        if (!node.set("rows", matrix.rows) || !node.set("cols", matrix.cols))
            return false;
    } else {
        node.setTag("!!opencv-nd-matrix");
        std::vector<int> sizes(matrix.size.p, matrix.size.p + matrix.dims);
        if (!node.set("sizes", sizes))
            return false;
    }
    if (!node.set("dt", data_type))
        return false;
    auto data = node.ensure("data");
    return opencv_detail::encodeMatrixData(data, matrix);
}

template <typename T, int M, int N>
bool yaml_decode(const Node &node, cv::Matx<T, M, N> &value) {
    cv::Mat matrix;
    if (!yaml_decode(node, matrix) || matrix.dims != 2 || matrix.rows != M || matrix.cols != N || matrix.channels() != 1)
        return false;
    cv::Mat converted;
    matrix.convertTo(converted, cv::traits::Type<T>::value);
    cv::Matx<T, M, N> result;
    const auto *data = converted.ptr<T>();
    for (int i = 0; i < M * N; ++i)
        result.val[i] = data[i];
    value = result;
    return true;
}

template <typename T, int M, int N>
bool yaml_encode(Node &node, const cv::Matx<T, M, N> &value) {
    return yaml_encode(node, cv::Mat(value, true));
}

//! @} core_yaml
#endif

//! @cond

namespace detail {

template <typename>
struct always_false : std::false_type {};

template <typename T>
struct is_optional : std::false_type {};
template <typename T>
struct is_optional<std::optional<T>> : std::true_type {
    using value_type = T;
};

template <typename T>
struct is_vector : std::false_type {};
template <typename T, typename A>
struct is_vector<std::vector<T, A>> : std::true_type {
    using value_type = T;
};

template <typename T>
struct is_array : std::false_type {};
template <typename T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type {
    using value_type = T;
    static constexpr std::size_t size = N;
};

template <typename T>
struct is_string_map : std::false_type {};
template <typename V, typename C, typename A>
struct is_string_map<std::map<std::string, V, C, A>> : std::true_type {
    using mapped_type = V;
};
template <typename V, typename H, typename E, typename A>
struct is_string_map<std::unordered_map<std::string, V, H, E, A>> : std::true_type {
    using mapped_type = V;
};

template <typename T>
auto decodeAdl(const Node &node, T &value, int) -> decltype(yaml_decode(node, value), bool()) {
    return static_cast<bool>(yaml_decode(node, value));
}

template <typename T>
bool decodeAdl(const Node &, T &, long) {
    static_assert(always_false<T>::value, "unsupported YAML decode type; provide yaml_decode(const rm::yaml::Node &, T &)");
    return false;
}

template <typename T>
auto encodeAdl(Node &node, const T &value, int) -> decltype(yaml_encode(node, value), bool()) {
    return static_cast<bool>(yaml_encode(node, value));
}

template <typename T>
bool encodeAdl(Node &, const T &, long) {
    static_assert(always_false<T>::value, "unsupported YAML encode type; provide yaml_encode(rm::yaml::Node &, const T &)");
    return false;
}

} // namespace detail

template <typename T>
bool Node::read(T &value) const {
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<U, std::string>) {
        if (!isScalar())
            return false;
        value.assign(scalar());
        return true;
    } else if constexpr (std::is_same_v<U, bool>) {
        return readBool(value);
    } else if constexpr (std::is_enum_v<U>) {
        using E = std::underlying_type_t<U>;
        E raw{};
        if (!read(raw))
            return false;
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (std::is_integral_v<U> && std::is_signed_v<U>) {
        int64_t raw{};
        if (!readSigned(raw) || raw < static_cast<int64_t>(std::numeric_limits<U>::min()) || raw > static_cast<int64_t>(std::numeric_limits<U>::max()))
            return false;
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (std::is_integral_v<U> && std::is_unsigned_v<U>) {
        uint64_t raw{};
        if (!readUnsigned(raw) || raw > static_cast<uint64_t>(std::numeric_limits<U>::max()))
            return false;
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (std::is_floating_point_v<U>) {
        double raw{};
        if (!readFloating(raw) || raw < -static_cast<double>(std::numeric_limits<U>::max()) || raw > static_cast<double>(std::numeric_limits<U>::max()))
            return false;
        value = static_cast<U>(raw);
        return true;
    } else if constexpr (detail::is_optional<U>::value) {
        if (isNull()) {
            value.reset();
            return true;
        }
        typename detail::is_optional<U>::value_type item{};
        if (!read(item))
            return false;
        value = std::move(item);
        return true;
    } else if constexpr (detail::is_vector<U>::value) {
        if (!isSequence())
            return false;
        U result;
        result.reserve(size());
        for (std::size_t i = 0; i < size(); ++i) {
            typename detail::is_vector<U>::value_type item{};
            if (!(*this)[i].read(item))
                return false;
            result.push_back(std::move(item));
        }
        value = std::move(result);
        return true;
    } else if constexpr (detail::is_array<U>::value) {
        if (!isSequence() || size() != detail::is_array<U>::size)
            return false;
        U result{};
        for (std::size_t i = 0; i < result.size(); ++i)
            if (!(*this)[i].read(result[i]))
                return false;
        value = std::move(result);
        return true;
    } else if constexpr (detail::is_string_map<U>::value) {
        if (!isMap())
            return false;
        U result;
        for (const auto name : keys()) {
            typename detail::is_string_map<U>::mapped_type item{};
            if (!(*this)[name].read(item))
                return false;
            result.emplace(std::string(name), std::move(item));
        }
        value = std::move(result);
        return true;
    } else {
        return detail::decodeAdl(*this, value, 0);
    }
}

template <typename T>
std::optional<T> Node::as() const {
    T value{};
    if (!read(value))
        return std::nullopt;
    return value;
}

template <typename T>
T Node::valueOr(T fallback) const {
    T value{};
    return read(value) ? value : std::move(fallback);
}

template <typename T>
bool Node::write(const T &value) {
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    if (!valid()) {
        return false;
    } else if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, std::string_view> || std::is_convertible_v<const T &, std::string_view>) {
        setScalar(std::string_view(value));
        return true;
    } else if constexpr (std::is_same_v<U, bool>) {
        return writeBool(value);
    } else if constexpr (std::is_enum_v<U>) {
        return write(static_cast<std::underlying_type_t<U>>(value));
    } else if constexpr (std::is_integral_v<U> && std::is_signed_v<U>) {
        return writeSigned(static_cast<int64_t>(value));
    } else if constexpr (std::is_integral_v<U> && std::is_unsigned_v<U>) {
        return writeUnsigned(static_cast<uint64_t>(value));
    } else if constexpr (std::is_floating_point_v<U>) {
        return writeFloating(static_cast<double>(value));
    } else if constexpr (detail::is_optional<U>::value) {
        if (!value) {
            setNull();
            return true;
        }
        return write(*value);
    } else if constexpr (detail::is_vector<U>::value || detail::is_array<U>::value) {
        makeSequence();
        for (const auto &item : value)
            if (!push(item))
                return false;
        return true;
    } else if constexpr (detail::is_string_map<U>::value) {
        makeMap();
        for (const auto &[name, item] : value)
            if (!set(name, item))
                return false;
        return true;
    } else {
        return detail::encodeAdl(*this, value, 0);
    }
}

template <typename T>
bool Node::set(std::string_view name, const T &value) {
    auto child = ensure(name);
    return child.write(value);
}

template <typename T>
bool Node::push(const T &value) {
    auto child = append();
    return child.write(value);
}

//! @endcond

//! @} yaml

} // namespace rm::yaml
