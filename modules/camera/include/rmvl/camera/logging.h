/**
 * @file logging.h
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2022-11-18
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <stdio.h>

//! @addtogroup camera
//! @{

#define CAM_WARNING(msg...)                  \
    do                                       \
    {                                        \
        printf("\033[33m[ CAM-WARN ] " msg); \
        printf("\033[0m\n");                 \
    } while (false)

#define CAM_PASS(msg...)                     \
    do                                       \
    {                                        \
        printf("\033[32m[ CAM-PASS ] " msg); \
        printf("\033[0m\n");                 \
    } while (false)

#define CAM_ERROR(msg...)                    \
    do                                       \
    {                                        \
        printf("\033[31m[ CAM-ERR  ] " msg); \
        printf("\033[0m\n");                 \
    } while (false)

#define CAM_INFO(msg...)             \
    do                               \
    {                                \
        printf("[ CAM-INFO ] " msg); \
        printf("\n");                \
    } while (false)

//! @} camera
