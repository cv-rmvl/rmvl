/**
 * @file sample_version.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-06-25
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <iostream>

#include <opencv2/core.hpp>

#include "rmvl/core.hpp"

const char *keys = "{ help h usage ? |  | 帮助信息 }"
                   "{ verbose v      |  | 编译配置信息 }";

int main(int argc, char *argv[])
{
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.has("help"))
        parser.printMessage();
    else if (parser.has("verbose"))
        std::cout << rm::getBuildInformation() << std::endl;
    else
        std::cout << RMVL_VERSION << std::endl;
    return 0;
}
