/**
 * @file toml.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief TOML 1.0 解析、只读节点树和类型转换
 * @version 1.0
 * @date 2026-11-06
 *
 * @copyright Copyright 2026 (c), zhaoxi
 *
 */

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "rmvl/core/rmvldef.hpp"

namespace rm::toml {

/**
 * @defgroup core_toml TOML 解析模块
 * @ingroup core
 * @brief 提供 TOML 1.0 配置加载、只读节点访问和 C++ 类型转换功能
 * @details
 * - load() 从文件加载，loads() 从 UTF-8 文本加载，成功时均返回 Table 类型的根节点，失败返回无效节点
 * - TOML 不包含 null 类型，Undefined 仅用于表示无效节点句柄
 * @see tutorial_modules_toml
 */

//! @addtogroup core_toml
//! @{

//! TOML 节点类型
enum class NodeType : uint8_t {
    Undefined, //!< 无效或不存在的节点，不是 TOML 值
    Table,     //!< 表，包含以字符串为键的子节点，也用于表示内联表
    Array,     //!< 数组，元素可以具有不同类型，表数组同样使用此类型
    String,    //!< UTF-8 字符串，读取时已完成转义处理
    Integer,   //!< 有符号 64 位整数
    Floating,  //!< 双精度浮点数，支持正负无穷和 NaN
    Boolean,   //!< 布尔值
    Date,      //!< 本地日期，不包含时间和 UTC 偏移
    Time,      //!< 本地时间，不包含日期和 UTC 偏移
    DateTime   //!< 日期时间，可包含 UTC 偏移
};

//! 本地日期，对应 TOML 的 local date
struct Date {
    int year{};         //!< 年份
    unsigned month{};   //!< 月份，范围为 1～12
    unsigned day{};     //!< 日期，范围为 1～31，实际范围取决于年月
};

/**
 * @brief 本地时间，对应 TOML 的 local time
 * @details 小数秒保留至纳秒，源文本中超出纳秒精度的部分截断
 */
struct Time {
    unsigned hour{};         //!< 时，范围为 0～23
    unsigned minute{};       //!< 分，范围为 0～59
    unsigned second{};       //!< 秒，范围为 0～59
    unsigned nanosecond{};   //!< 纳秒，范围为 0～999999999
};

/**
 * @brief 本地日期时间或带 UTC 偏移的日期时间
 * @details
 * - 不将本地日期时间自动换算为系统时区或时间戳
 * - Time 的小数秒保留至纳秒，源文本中超出纳秒精度的部分截断
 */
struct DateTime {
    Date date;                         //!< 日期部分
    Time time;                         //!< 时间部分
    std::optional<int> offsetMinutes;   //!< UTC 偏移分钟数，无值表示本地日期时间，0 表示 UTC，正值表示领先 UTC
};

//! TOML 加载错误类型
enum class ErrorCode : uint8_t {
    None,  //!< 无错误
    Parse, //!< UTF-8 编码、语法或定义冲突错误
    Io,    //!< 文件路径无效或文件打开、读取失败
};

/**
 * @brief TOML 加载错误信息
 * @details load() 和 loads() 可通过可选的错误输出参数填写本结构，成功时重置为空错误
 */
struct Error {
    ErrorCode code{ErrorCode::None}; //!< 错误类型
    std::string message;            //!< 错误描述，无错误时为空
    std::size_t line{};             //!< 解析错误行号，从 1 开始，不适用时为 0
    std::size_t column{};           //!< 解析错误列号，从 1 开始，不适用时为 0

    //! @return 是否包含错误
    explicit operator bool() const noexcept { return code != ErrorCode::None; }
};

/**
 * @brief 共享底层节点树所有权的只读节点句柄
 * @details
 * - 拷贝仅共享节点树所有权，不复制节点树，子节点可独立于根句柄存活
 * - 默认构造、查找失败或移出后的句柄无效，可用 valid() 或显式布尔转换判断
 * - 布尔转换只判断节点有效性，保存 false 的布尔节点仍是有效节点
 * - 节点树不可修改，同一棵节点树支持多个线程并发只读访问
 * - keys() 返回的键名视图在底层节点树最后一个有效句柄销毁前有效
 */
class RMVL_EXPORTS Node {
public:
    //! 构造不关联任何节点树的无效节点
    Node() noexcept = default;

    //! @return 是否关联有效节点，与节点保存的值无关
    [[nodiscard]] bool valid() const noexcept { return _impl && _node != nullptr; }

    //! @return 与 valid() 相同，可用于 if、!、&& 等条件表达式
    explicit operator bool() const noexcept { return valid(); }

    //! @return 节点类型，无效节点返回 NodeType::Undefined
    NodeType type() const noexcept;

    //! @return 是否为表节点，包含普通表和内联表，无效节点返回 false
    bool isTable() const noexcept { return type() == NodeType::Table; }

    //! @return 是否为数组节点，包含表数组，无效节点返回 false
    bool isArray() const noexcept { return type() == NodeType::Array; }

    //! @return 表的键数量或数组的元素数量，其他类型及无效节点返回 0
    std::size_t size() const noexcept;

    /**
     * @brief 判断表中是否存在指定键
     * @param[in] key 字面键名，不解释点分路径
     * @return 当前节点为表且键存在时返回 true，否则返回 false
     */
    bool contains(std::string_view key) const noexcept;

    /**
     * @brief 查询表的子节点，不插入或修改节点
     * @param[in] key 字面键名，例如 "a.b" 表示单个键，嵌套访问应使用 node["a"]["b"]
     * @return 当前节点为表且键存在时返回共享节点树所有权的子节点，否则返回无效节点
     */
    Node operator[](std::string_view key) const noexcept;

    /**
     * @brief 查询数组元素
     * @param[in] index 从 0 开始的元素下标
     * @return 当前节点为数组且下标有效时返回共享节点树所有权的子节点，否则返回无效节点
     */
    Node operator[](std::size_t index) const noexcept;

    /**
     * @brief 获取表的全部键名
     * @return 键名视图数组，非表节点或无效节点返回空数组，不保证键的排列顺序
     * @note 返回的视图不持有节点树所有权，使用期间必须保留至少一个属于该节点树的有效 Node
     */
    std::vector<std::string_view> keys() const;

    /**
     * @brief 读取节点并转换为指定的 C++ 类型
     * @tparam T 支持 std::string、bool、整数、浮点、枚举、Date、Time、DateTime、std::optional、
     * std::vector、std::array、以 std::string 为键的 std::map/std::unordered_map，以及 ADL 自定义类型
     * @param[out] value 转换结果，返回 false 时保持其原值不变
     * @return 转换成功返回 true，节点类型不符、数值越界或容器元素转换失败返回 false
     * @details
     * - 严格区分字符串、整数、浮点和布尔，不进行字符串到数字或整数到浮点的隐式转换
     * - 整数转换检查目标类型范围，枚举通过其底层整数类型转换，不检查是否为已声明的枚举项
     * - 浮点转换检查有限值是否超出目标类型范围，允许精度损失，保留无穷和 NaN
     * - std::array 要求节点数组长度完全匹配，容器元素递归使用相同的转换规则
     * - 无效节点读取为 std::optional 时会清空目标并返回 true，有效节点按其包含的类型转换
     * - 自定义类型需能默认构造，在其命名空间提供 `bool toml_decode(const rm::toml::Node &, T &)`
     *   解码在临时对象上完成，返回 false 时不覆盖调用方对象
     * @note 本函数不捕获内存分配异常或用户自定义解码函数抛出的异常
     */
    template <typename T>
    bool read(T &value) const;

    /**
     * @brief 读取节点并返回可选的转换结果
     * @tparam T 目标类型，需能默认构造，支持范围和转换规则与 read() 相同
     * @return 成功时返回目标值，转换失败时返回 std::nullopt
     * @note T 为 `std::optional<U>` 时，读取无效节点得到的是外层有值、内层无值的结果
     */
    template <typename T>
    std::optional<T> as() const;

    /**
     * @brief 读取节点，转换失败时使用默认值
     * @tparam T 目标类型，需能默认构造，支持范围和转换规则与 read() 相同
     * @param[in] fallback 节点缺失、类型不符或数值越界等转换失败时使用的默认值
     * @return 成功时返回转换值，否则返回 fallback
     * @note 异常仍会向调用方传播，不会因发生异常而返回默认值
     */
    template <typename T>
    T valueOr(T fallback) const;

private:
    struct Impl;
    std::shared_ptr<const Impl> _impl;
    const void *_node{};
    Node(std::shared_ptr<const Impl> impl, const void *node) noexcept
        : _impl(std::move(impl)), _node(node) {}
    bool readValue(std::string &value) const;
    bool readValue(bool &value) const noexcept;
    bool readValue(int64_t &value) const noexcept;
    bool readValue(double &value) const noexcept;
    bool readValue(Date &value) const noexcept;
    bool readValue(Time &value) const noexcept;
    bool readValue(DateTime &value) const noexcept;
    friend Node loads(std::string_view source, Error *error);
};

/**
 * @brief 从文件加载并解析 TOML 1.0 配置
 * @param[in] path 文件路径，相对路径以进程当前工作目录为基准，不允许包含空字符
 * @param[out] error 可选的错误输出，默认为 nullptr，成功时清空已有错误
 * @return 成功时返回共享节点树所有权的 Table 根节点，空文件返回空 Table，失败返回无效节点
 * @details
 * - 路径或文件读写问题对应 ErrorCode::Io，编码、语法或定义冲突对应 ErrorCode::Parse
 * - 这些加载错误通过返回值和 error 报告，不抛出异常
 * @note
 * - 返回的节点不引用 path 的内存，底层节点树由节点句柄共享持有
 * - 内存分配失败等标准库异常不在此错误协议内，本函数不保证 noexcept
 * @see loads()
 */
RMVL_EXPORTS Node load(std::string_view path, Error *error = nullptr);

/**
 * @brief 从内存中的 UTF-8 文本解析 TOML 1.0 配置
 * @param[in] source TOML 文本，不要求以空字符结尾
 * @param[out] error 可选的错误输出，默认为 nullptr，成功时清空已有错误
 * @return 成功时返回共享节点树所有权的 Table 根节点，空文本返回空 Table，失败返回无效节点
 * @details 编码或语法错误、重复键和非法表定义等问题通过返回值和 ErrorCode::Parse 报告，不抛出异常
 * @note
 * - 解析完成后不依赖 source 的内存，调用方可以释放或修改原始文本
 * - 内存分配失败等标准库异常不在此错误协议内，本函数不保证 noexcept
 * @see load()
 */
RMVL_EXPORTS Node loads(std::string_view source, Error *error = nullptr);

//! @} core_toml

} // namespace rm::toml

#include "details/toml_impl.hpp"
