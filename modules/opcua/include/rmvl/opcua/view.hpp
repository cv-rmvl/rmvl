/**
 * @file view.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 视图
 * @version 1.0
 * @date 2023-11-27
 *
 * @copyright Copyright 2023 (c), zhaoxi
 *
 */

#pragma once

#include "variable.hpp"

namespace rm
{

//! @addtogroup opcua
//! @{

//! OPC UA 视图
class View final
{
public:
    /**
     * @brief 浏览名称 BrowseName
     * @brief
     * - 属于非服务器层面的 ID 号，可用于完成路径搜索
     * @brief
     * - 作为视图类型节点、视图节点之间链接的依据
     * @brief
     * - 同一个命名空间 `ns` 下该名称不能重复
     */
    std::string browse_name{};

    /**
     * @brief 展示名称 DisplayName
     * @brief
     * - 在服务器上对外展示的名字 - `en-US`
     * @brief
     * - 同一个命名空间 `ns` 下该名称可以相同
     */
    std::string display_name{};
    //! 视图的描述 - `zh-CN`
    std::string description{};

private:
    std::vector<UA_NodeId> _nodes; //!< 视图下的节点 ID 列表

public:
    View() = default;

    View(const View &view) : browse_name(view.browse_name), display_name(view.display_name),
                             description(view.description), _nodes(view._nodes) {}

    View(View &&view) : browse_name(std::move(view.browse_name)), display_name(std::move(view.display_name)),
                        description(std::move(view.description)), _nodes(std::move(view._nodes)) {}

    View &operator=(const View &view);
    View &operator=(View &&view);

    /**
     * @brief 添加节点 ID
     * @brief
     * - 使用逗号分隔，可添加多个节点 ID，可参考以下示例
     * @code{.cpp}
     * View view;
     * view.add(node_1, node_2, node_3);
     * @endcode
     *
     * @tparam UA_NodeId_ 节点 ID 类型，指代 `UA_NodeId`
     * @param[in] node_id 既存的待添加的节点 ID
     */
    template <typename... UA_NodeId_>
    inline void add(UA_NodeId_ &&...node_id) { [[maybe_unused]] int _[]{(_nodes.emplace_back(node_id), 0)...}; }

    //! 获取节点 ID 列表
    inline const auto &data() const { return _nodes; }
};

//! @} opcua

} // namespace rm
