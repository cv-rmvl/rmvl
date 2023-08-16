/**
 * @file loader.hpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 
 * @version 1.0
 * @date 2022-11-30
 * 
 * @copyright Copyright 2023 (c), zhaoxi
 * 
 */

#pragma once

//! @defgroup para 参数及加载模块

#include "rmvl/core/util.hpp"

namespace rm::para
{

//! @addtogroup para
//! @{

/**
 * @brief 参数加载
 * 
 * @tparam Tp 参数类型
 * @param[in] para_obj 参数对象
 * @param[in] file_path 参数 yml 文件
 */
template <typename Tp, typename Enable = typename Tp::paraId>
inline void load(Tp &para_obj, const std::string &file_path) { para_obj = Tp(file_path); }

/**
 * @brief 参数读取，忽略为空的节点
 * 
 * @param[in] n cv::FileNode 节点
 * @param[in] t 目标数据
 */
template<typename _FileNode, typename Tp>
inline void readExcludeNone(const _FileNode &n, Tp &t) { n.isNone() ? void(0) : n >> t; } 

//! @} rm::para

} // namespace para

namespace para = rm::para;
