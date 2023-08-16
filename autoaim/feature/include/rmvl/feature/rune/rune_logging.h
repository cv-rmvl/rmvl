/**
 * @file rune_logging.h
 * @author RoboMaster Vision Community
 * @brief
 * @version 0.1
 * @date 2023-07-16
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#pragma once

#include <cstdio>

#ifdef rune_debug
#define DEBUG_RUNE_WARNING_(msg) RUNE_WARNING_(msg)
#define DEBUG_RUNE_INFO_(msg...) RUNE_INFO_(msg)
#define DEBUG_RUNE_PASS_(msg) RUNE_PASS_(msg)
#else
#define DEBUG_RUNE_WARNING_(msg) ((void)0)
#define DEBUG_RUNE_INFO_(msg...) ((void)0)
#define DEBUG_RUNE_PASS_(msg) ((void)0)
#endif

#define RUNE_PASS_(msg...)                    \
    do                                        \
    {                                         \
        printf("\033[32m[ RUNE-PASS ] " msg); \
        printf("\033[0m\n");                  \
    } while (false)
    
#define RUNE_WARNING_(msg...)                    \
    do                                           \
    {                                            \
        printf("\033[33m[ RUNE-WARNING ] " msg); \
        printf("\033[0m\n");                     \
    } while (false)

#define RUNE_INFO_(msg...)            \
    do                                \
    {                                 \
        printf("[ RUNE-INFO ] " msg); \
        printf("\n");                 \
    } while (false)
