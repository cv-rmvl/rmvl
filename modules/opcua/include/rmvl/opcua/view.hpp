/**
 * @file view.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 视图
 * @version 2.2
 * @date 2024-03-29
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "utilities.hpp"

namespace rm {

//! @addtogroup opcua
//! @{

//! OPC UA 视图
class RMVL_EXPORTS_W View final {
public:
    RMVL_W View() = default;

    /**
     * @brief 添加节点 ID
     * @brief
     * - 使用逗号分隔，可添加多个节点 ID，可参考以下示例
     * @code{.cpp}
     * View view;
     * view.add(node_1, node_2, node_3);
     * @endcode
     *
     * @tparam NodeId_ 节点 ID 类型，指代 `NodeId`
     * @param[in] nds 既存的待添加的节点 ID
     */
    template <typename... NodeId_>
    RMVL_W_SUBST("View_Add")
    inline void add(NodeId_ &&...nds) { (..., _nodes.emplace_back(nds)); }

    //! 获取节点 ID 列表
    RMVL_W inline const std::vector<NodeId> &data() const { return _nodes; }

    //! 命名空间索引，默认为 `1`
    RMVL_W_RW uint16_t ns{1U};

    /**
     * @brief 浏览名称 BrowseName
     * @brief
     * - 属于非服务器层面的 ID 号，可用于完成路径搜索
     * @brief
     * - 作为视图类型节点、视图节点之间链接的依据
     * @brief
     * - 同一个命名空间 `ns` 下该名称不能重复
     */
    RMVL_W_RW std::string browse_name{};

    /**
     * @brief 展示名称 DisplayName
     * @brief
     * - 在服务器上对外展示的名字 - `en-US`
     * @brief
     * - 同一个命名空间 `ns` 下该名称可以相同
     */
    RMVL_W_RW std::string display_name{};
    //! 视图的描述 - `zh-CN`
    RMVL_W_RW std::string description{};

private:
    std::vector<NodeId> _nodes; //!< 视图下的节点 ID 列表
};

//! @} opcua

} // namespace rm
