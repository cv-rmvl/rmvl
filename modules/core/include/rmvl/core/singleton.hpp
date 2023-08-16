/**
 * @file singleton.hpp
 * @author RoboMaster Vision Community
 * @brief 单例类模板
 * @version 1.0
 * @date 2022-02-08
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <cassert>
#include <utility>

namespace rm
{

//! @addtogroup core
//! @{

/**
 * @brief 单例模板
 *
 * @tparam Tp 要更改为单例的模板参数的类型
 */
template <typename Tp>
class GlobalSingleton final
{
public:
    //! 获取实例化对象
    static Tp *Get() { return *GetPPtr(); }

    //! 构造单例
    template <typename... Args>
    static void New(Args &&...args)
    {
        assert(Get() == nullptr);
        *GetPPtr() = new Tp(std::forward<Args>(args)...);
    }

    //! 销毁单例
    static void Delete()
    {
        if (Get() != nullptr)
        {
            delete Get();
            *GetPPtr() = nullptr;
        }
    }

private:
    //! 获取二级指针
    static Tp **GetPPtr()
    {
        static Tp *ptr = nullptr;
        return &ptr;
    }
};

//! @} core

} // namespace rm
