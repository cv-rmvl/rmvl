/**
 * @file timer.hpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-01-30
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <thread>

//! @addtogroup core
//! @{
//!     @defgroup timer 定时、计时模块
//! @}

namespace rm
{

//! @addtogroup timer
//! @{

/**
 * @brief 获取当前系统时间
 * @note 当参数是 std::string 时，返回 yyyy-mm-dd hh:mm:sss 的 std::string 类型
 *       当参数是 int64_t 时，返回 Linux 发行时间到当前的毫秒数
 * @tparam Tp 可选类型，std::string 或 int64_t
 * @return 系统时间
 */
template <typename Tp,
          typename Enable = typename std::enable_if<std::is_same<Tp, std::string>::value ||
                                                    std::is_same<Tp, int64_t>::value>::type>
Tp getSystemTick();

/**
 * @brief 计时器
 */
class Timer
{
    bool _started;                                    //!< 开始计时标志位
    bool _paused;                                     //!< 停止计时标志位
    std::chrono::steady_clock::time_point _reference; //!< 当前时间
    std::chrono::duration<long double> _accumulated;  //!< 累计时间

public:
    /**
     * @brief 创建计时器对象
     */
    Timer() : _started(false), _paused(false),
              _reference(std::chrono::steady_clock::now()),
              _accumulated(std::chrono::duration<long double>(0)) {}

    Timer(const Timer &) = delete;
    Timer(Timer &&) = delete;
    ~Timer() = default;

    /**
     * @brief 开始计时
     */
    void start();

    /**
     * @brief 结束计时
     */
    void stop();

    /**
     * @brief 重置计时器
     */
    void reset();

    /**
     * @brief 计算计时时长
     * @note 计算 start() 和 stop() 函数之间的时长
     *
     * @return 计时时长，单位是 ms
     */
    std::chrono::milliseconds::rep duration() const;
};

/**
 * @brief 同步定时，当前进程挂起time_length时长后，再继续当前进程
 *
 * @param[in] time_length 定时时长
 */
void syncWait(int64_t time_length);

/**
 * @brief 异步定时，另外开一个线程，在定时time_length ms后执行某个函数
 *
 * @tparam Callable 可调用对象
 * @tparam Args 可调用参数
 * @param[in] after 定时时长
 * @param[in] fn 可调用对象
 * @param[in] args 函数的参数
 */
template <typename Callable, class... Args>
void asyncWait(int after, Callable &&fn, Args &&...args)
{
    // 函数包装器包装输入的函数指针和函数参数
    std::function<std::result_of_t<Callable(Args...)>()> task(std::bind(std::forward<Callable>(fn), std::forward<Args>(args)...));
    // 另外开一个线程进行定时
    std::thread(
        [after, task]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(after));
            task();
        })
        .detach();
}

//! @} timer

} // namespace rm
