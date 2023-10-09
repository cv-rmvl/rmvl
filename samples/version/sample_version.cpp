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

#include "rmvl/core.hpp"

const char *help = "\t-?, -h, -help, -usage\n\t\t帮助信息\n"
                   "\t-v, -verbose\n\t\t编译配置信息";

int main(int argc, char *argv[])
{
    if (argc == 1)
        printf("%s\n", RMVL_VERSION);
    if (argc > 2)
        printf("Bad arguments, please run: \033[33mrmvl_version -h\033[0m first!\n");
    if (argc == 2)
    {
        std::string argv_1 = argv[1];
        if (argv_1 == "-h" || argv_1 == "-?" || argv_1 == "-help" || argv_1 == "-usage")
            printf("Usage of \033[33m%s\033[0m:\n%s\n", argv[0], help);
        else if (argv_1 == "-v" || argv_1 == "-e")
            printf("%s\n", rm::getBuildInformation());
        else
            printf("Bad arguments, please run: \033[33mrmvl_version -h\033[0m first!\n");
    }
    return 0;
}
