/**
 * @file default_group.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 默认序列组
 * @version 1.0
 * @date 2024-11-16
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include "rmvl/group/group.h"

namespace rm {

group::ptr DefaultGroup::clone() {
    auto retval = std::make_shared<DefaultGroup>(*this);
    // 更新内部所有追踪器
    for (auto &p_tracker : retval->_trackers)
        p_tracker = p_tracker->clone();
    return retval;
}

void DefaultGroup::sync(const ImuData &, double) {}

} // namespace rm
