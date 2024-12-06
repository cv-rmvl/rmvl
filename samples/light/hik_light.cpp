/**
 * @file hik_light.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 海康机器人光源控制示例程序
 * @version 1.0
 * @date 2024-12-04
 *
 * @copyright Copyright 2024 (c), zhaoxi
 *
 */

#include <iostream>

#include <rmvl/light/hik_light_control.h>
#include <rmvlpara/light/hik_light_control.h>

static void program_help()
{
    constexpr const char *usage = "Program Usage:\n"
                                  "  rmvl_hik_lightctl <serial>\n";
    std::cout << usage << std::endl;
}

static void cmd_help()
{
    constexpr const char *usage = "\n\033[34mCommon functions usage:\033[0m\n"
                                  "  h, help, ?, usage \033[32m# show this help message\033[0m\n"
                                  "  exit, quit, q     \033[32m# exit the program\033[0m\n"
                                  "\n\033[34mParameters control usage:\033[0m\n"
                                  "  ctl get delay       \033[32m# get the delay time after writing\033[0m\n"
                                  "  ctl set delay <val> \033[32m# set the delay time after writing\033[0m\n"
                                  "\n\033[34mCommands usage:\033[0m\n"
                                  "  open            \033[32m# open all the channels\033[0m\n"
                                  "  close           \033[32m# close all the channels\033[0m\n"
                                  "  get <chn>       \033[32m# get the brightness of the specified channel\033[0m\n"
                                  "  set <chn> <val> \033[32m# set the brightness of the specified channel\033[0m\n";
    std::cout << usage << std::endl;
}

static void warning()
{
    std::cout << "\033[33mWarning: The command is invalid.\033[0m\n";
}

static std::vector<std::string> split(std::string_view str, char delim)
{
    std::vector<std::string> res;
    if (str.empty())
        return res;
    std::string::size_type start = str.find_first_not_of(delim);
    std::string::size_type index = str.find(delim, start);
    while (index != std::string::npos)
    {
        res.emplace_back(str.substr(start, index - start));
        start = str.find_first_not_of(delim, index);
        index = str.find(delim, start);
    }
    res.emplace_back(str.substr(start));
    return res;
}

static void parametersCtl(std::vector<std::string> &cmds)
{
    if (cmds.size() < 3)
    {
        warning();
        return;
    }
    if (cmds.size() == 3)
    {
        if (cmds[1] == "get")
        {
            if (cmds[2] == "delay")
                std::cout << rm::para::hik_light_control_param.DELAY_AFTER_WRITE << '\n';
            else
                warning();
        }
        else
            warning();
    }
    else if (cmds.size() == 4)
    {
        if (cmds[1] == "set")
        {
            if (cmds[2] == "delay")
            {
                uint32_t val = static_cast<uint32_t>(std::stoi(cmds[3]));
                rm::para::hik_light_control_param.DELAY_AFTER_WRITE = val;
                std::cout << "'Success'\n";
            }
            else
                warning();
        }
        else
            warning();
    }
    else
        warning();
}

static void functionCtl(rm::HikLightController &lc, bool &stop, std::vector<std::string> &cmds)
{
    if (cmds.size() == 1)
    {
        if (cmds[0] == "h" || cmds[0] == "help" || cmds[0] == "?" || cmds[0] == "usage")
            cmd_help();
        else if (cmds[0] == "open")
        {
            if (!lc.open())
                std::cout << "'Failed to open all the channels.'\n";
            else
                std::cout << "'Success'\n";
        }
        else if (cmds[0] == "close")
        {
            if (!lc.close())
                std::cout << "'Failed to close all the channels.'\n";
            else
                std::cout << "'Success'\n";
        }
        else if (cmds[0] == "exit" || cmds[0] == "quit" || cmds[0] == "q")
            stop = true;
        else
            warning();
    }
    else if (cmds.size() == 2)
    {
        if (cmds[0] == "get")
        {
            int chn = std::stoi(cmds[1]);
            int val = lc.get(chn);
            if (val < 0)
                std::cout << "'Failed to get the brightness of channel " << chn << "'\n";
            else
                std::cout << val << '\n';
        }
        else
            warning();
    }
    else if (cmds.size() == 3)
    {
        if (cmds[0] == "set")
        {
            int chn = std::stoi(cmds[1]);
            int val = std::stoi(cmds[2]);
            if (!lc.set(chn, val))
                std::cout << "'Failed to set the brightness of channel " << chn << "'\n";
            else
                std::cout << "'Success'\n";
        }
        else
            warning();
    }
    else if (cmds.size() > 3)
        warning();
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        program_help();
        return -1;
    }

    rm::LightConfig cfg{};
    cfg.handle_mode = rm::LightHandleMode::Serial;
    std::string id{argv[1]};
    auto lc = rm::HikLightController(cfg, id);
    if (!lc.isOpened())
    {
        ERROR_("Failed to open the light controller.");
        return -1;
    }

    bool stop{};
    while (!stop)
    {
        std::cout << ">>> ";
        std::string cmd{};
        std::getline(std::cin, cmd);
        auto cmds = split(cmd, ' ');
        if (cmds.empty())
            continue;
        // Parameters control
        if (cmds[0] == "ctl")
            parametersCtl(cmds);
        // Function commands
        else
            functionCtl(lc, stop, cmds);
    }
}
