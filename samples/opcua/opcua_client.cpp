#include <iostream>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include "rmvl/opcua/client.hpp"

using namespace rm;

enum class Direction
{
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4,
    ENTER = 0,
    OTHER = -1
};

#ifdef _WIN32
//! 获取方向
Direction get_direction()
{
    int ch = _getch();
    if (ch == 0xE0)
    {
        ch = _getch();
        switch (ch)
        {
        case 72:
            return Direction::UP;
        case 80:
            return Direction::DOWN;
        case 75:
            return Direction::LEFT;
        case 77:
            return Direction::RIGHT;
        }
    }
    else if (ch == '\r' || ch == '\n')
        return Direction::ENTER;
    return Direction::OTHER;
}
#else
// 设置终端为非阻塞模式
void setNonBlocking(bool enable)
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if (enable)
    {
        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO;
    }
    else
    {
        tty.c_lflag |= ICANON;
        tty.c_lflag |= ECHO;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

// 读取单个字符
char _getch()
{
    char buf = 0;
    setNonBlocking(true);
    auto size = read(STDIN_FILENO, &buf, 1);
    if (size < 0)
        return 0;
    setNonBlocking(false);
    return buf;
}

//! 获取方向
Direction get_direction()
{
    char ch = _getch();
    if (ch == '\033')
    {
        _getch(); // 跳过 '['
        switch (_getch())
        {
        case 'A':
            return Direction::UP;
        case 'B':
            return Direction::DOWN;
        case 'C':
            return Direction::RIGHT;
        case 'D':
            return Direction::LEFT;
        }
    }
    else if (ch == '\r' || ch == '\n')
        return Direction::ENTER;
    return Direction::OTHER;
}
#endif

//! 清屏
static void clear_sreen()
{
#ifdef _WIN32
    int ch = system("cls");
#else
    int ch = system("clear");
#endif
    if (ch == -1)
    {
        printf("\033[2J");
        printf("\033[0;0H");
    }
}

//! 清除标记
static void clear_mark()
{
    printf("\033[1;3H0\033[2;3H1\033[3;3H2\033[4;3Hq\033[0m\033[1;14H%*s\033[2;14H%*s\033[3;14H%*s\033[4;14H%*s",
           13, " ", 13, " ", 13, " ", 13, " ");
    fflush(stdout);
}

//! 查找节点，并标记
static rm::NodeId find_node_and_mark(OpcuaClient &cli, int id)
{
    constexpr const char *names[] = {"value_1", "value_2", "add"};
    std::cout << "\033[" << id + 1 << ";14H\033[1;5;33m<--\033[0m" << std::flush;
    return cli.find(names[id]);
}

//! 提示输入操作数
static std::string tips_op()
{
    std::cout << "\033[1;3H\033[33m0\033[2;3H1\033[3;3H2\033[4;3Hq\033[0m"
                 "\033[1;26H\033[K┌───────────────────────┐"
                 "\033[2;26H\033[K│     Input the ID:     │"
                 "\033[3;26H\033[K│                       │"
                 "\033[4;26H\033[K└───────────────────────┘"
              << std::flush;
    constexpr const char *choice_map[4] = {"0", "1", "2", "q"};
    uint8_t choice_idx{};
    Direction ch{Direction::OTHER};
    while (true)
    {
        if (choice_idx == 0)
            std::cout << "\033[3;28H\033[3;4;33m'0'\033[0m   '1'   '2'   'q'\033[5;1H" << std::flush;
        else if (choice_idx == 1)
            std::cout << "\033[3;28H'0'   \033[3;4;33m'1'\033[0m   '2'   'q'\033[5;1H" << std::flush;
        else if (choice_idx == 2)
            std::cout << "\033[3;28H'0'   '1'   \033[3;4;33m'2'\033[0m   'q'\033[5;1H" << std::flush;
        else
            std::cout << "\033[3;28H'0'   '1'   '2'   \033[3;4;33m'q'\033[0m\033[5;1H" << std::flush;

        ch = get_direction();
        if (ch == Direction::RIGHT)
            choice_idx = (choice_idx + 1) % 4;
        else if (ch == Direction::LEFT)
            choice_idx = (choice_idx + 3) % 4;
        else if (ch == Direction::ENTER)
            break;
    }
    clear_mark();
    return choice_map[choice_idx];
}

//! 提示输入读 or 写
static int tips_rw()
{
    std::cout << "\033[1;26H┌───────────────────────┐"
                 "\033[2;26H│ Select the operation: │"
                 "\033[3;26H│    Read      Write    │"
                 "\033[4;26H└───────────────────────┘"
              << std::flush;
    uint8_t choice_idx{};
    auto ch{Direction::OTHER};
    while (true)
    {
        if (choice_idx == 0)
            std::cout << "\033[3;31H\033[3;4;33mRead\033[0m      Write\033[5;1H" << std::flush;
        else
            std::cout << "\033[3;31HRead      \033[3;4;33mWrite\033[0m\033[5;1H" << std::flush;

        ch = get_direction();
        if (ch == Direction::ENTER)
            break;
        else if (ch != Direction::OTHER)
            choice_idx = !choice_idx;
    }

    return choice_idx;
}

//! 提示输入写入值
template <typename Tp>
static Tp tips_val(int id)
{
    std::cout << "\033[" << id + 1
              << ";18H\033[1;5;33mWrite\033[0m"
                 "\033[1;26H┌───────────────────────┐"
                 "\033[2;26H│    Input the value    │"
                 "\033[3;26H│ -->                   │"
                 "\033[4;26H└───────────────────────┘"
                 "\033[3;32H"
              << std::flush;
    Tp val{};
    std::cin >> val;
    return val;
}

//! 提示输入参数
static int tips_arg(std::string_view str)
{
    std::cout << "\033[1;26H┌─ Calculate the sum of 2 numbers ─┐"
                 "\033[2;26H│                                  │"
                 "\033[3;26H│ -->                              │"
                 "\033[4;26H└──────────────────────────────────┘"
                 "\033[2;28H"
              << "Input the \033[33m" << str << "\033[0m argument (int): " << "\033[3;32H"
              << std::flush;
    int val{};
    std::cin >> val;
    return val;
}

//! 显示值
template <typename Tp>
static void show_value(int id, Tp val)
{
    clear_mark();
    std::cout << "\033[" << id + 1 << ";14H\033[1;32m<--\033[0m \033[32m" << val << "\033[0m" << std::flush;
}

//! 询问是否退出
static bool ask_exit()
{
    std::cout << "\033[1;24H ┌────────────────────────────────────────┐"
                 "\033[2;24H │                                        │"
                 "\033[3;24H │   Are you sure to exit this program?   │"
                 "\033[4;24H │                                        │"
                 "\033[5;24H │                                        │"
                 "\033[6;24H │                                        │"
                 "\033[7;24H └────────────────────────────────────────┘"
              << std::flush;
    int choice{}; // 0: Yes, 1: No
    Direction ch{Direction::OTHER};
    while (true)
    {
        if (choice == 0)
            std::cout << "\033[5;35H\033[3;4;33mYes\033[0m                 No\033[5;1H" << std::flush;
        else
            std::cout << "\033[5;35HYes                 \033[3;4;33mNo\033[0m\033[5;1H" << std::flush;

        ch = get_direction();
        if (ch == Direction::ENTER)
            break;
        else if (ch != Direction::OTHER)
            choice = !choice;
    }
    std::cout << "\033[1;24H\033[K\033[2;24H\033[K\033[3;24H\033[K\033[4;24H\033[K\033[5;24H\033[K\033[6;24H\033[K\033[7;24H\033[K" << std::flush;
    return choice == 0;
}

int main()
{
    OpcuaClient cli("opc.tcp://127.0.0.1:4840");
    if (!cli.ok())
        return -1;
    clear_sreen();
    std::cout << "- 0: value_1\n- 1: value_2\n- 2:     add\n- q:    Exit\n";
    while (true)
    {
        auto num = tips_op();
        if (num == "0")
        {
            auto nd = find_node_and_mark(cli, 0);
            int rw = tips_rw();
            if (rw == 0)
            {
                int val = cli.read(nd);
                show_value(0, val);
            }
            else
            {
                int val = tips_val<int>(0);
                cli.write(nd, val);
                show_value(0, val);
            }
        }
        else if (num == "1")
        {
            auto nd = find_node_and_mark(cli, 1);
            int rw = tips_rw();
            if (rw == 0)
            {
                double val = cli.read(nd);
                show_value(1, val);
            }
            else
            {
                double val = tips_val<double>(1);
                cli.write(nd, val);
                show_value(1, val);
            }
        }
        else if (num == "2")
        {
            find_node_and_mark(cli, 2);
            int v1 = tips_arg("first");
            int v2 = tips_arg("second");
            auto [result, oargs] = cli.call("add", {v1, v2});
            if (result)
                show_value(2, oargs[0].cast<int>());
            else
                std::cout << "\033[3;14H\033[31m\u2716\033[0m" << std::flush;
        }
        else
        {
            if (ask_exit())
            {
                std::cout << "\033[5;1H" << std::flush;
                break;
            }
        }
    }

    return 0;
}
