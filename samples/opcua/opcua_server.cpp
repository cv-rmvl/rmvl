#include <csignal>

#include "rmvl/opcua/server.hpp"

using namespace rm;

Server server(4840);

static void onHandle(int) { server.shutdown(); }

//! 清屏
static void clearSreen()
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

//! 光标复位
static void resetCursor()
{
    printf("\033[10;0H");
    fflush(stdout);
}

//! 显示 OPC UA 服务器表格
static void show()
{
    constexpr const char *data = "  ┌────────────────┬────────────────┬────────────────┬────────────────┐\n"
                                 "  │     NodeId     │  DisplayName   │   BrowseName   │     Value      │\n"
                                 "  ├────────────────┼────────────────┼────────────────┼────────────────┤\n"
                                 "  │                │                │                │                │\n"
                                 "  ├────────────────┼────────────────┼────────────────┼────────────────┤\n"
                                 "  │                │                │                │                │\n"
                                 "  ├────────────────┼────────────────┼────────────────┼────────────────┤\n"
                                 "  │                │                │                │                │\n"
                                 "  └────────────────┴────────────────┴────────────────┴────────────────┘\n";
    printf("%s", data);
}

static std::string nodestr(const NodeId &node) { return "ns=" + std::to_string(node.ns) + "," + "s=" + std::to_string(node.id); }

/**
 * @brief 更新表格
 *
 * @param[in] row 表格行，取值范围 [1, 3]
 * @param[in] col 表格列，取值范围 [1, 4]
 * @param[in] msg 更新信息，不得超过 14 个字符
 */
static void update(int row, int col, std::string_view msg)
{
    if (row < 1 || row > 3 || col < 1 || col > 4)
        return;
    constexpr int width = 14;
    int size = msg.size();
    if (size > width)
    {
        msg = msg.substr(0, width);
        size = width;
    }
    int real_row = 2 * row + 2;
    int real_col = 17 * col - 12;

    int padding = (width - size) / 2;
    printf("\033[%d;%dH", real_row, real_col);
    printf("%*s%s%*s", padding, "", msg.data(), padding + (width - size) % 2, "");
    resetCursor();
}

int main()
{
    signal(SIGINT, onHandle);
    clearSreen();
    show();

    Variable value_1 = 42;
    value_1.display_name = "Value 1";
    value_1.browse_name = "value_1";
    Variable value_2 = 3.14;
    value_2.display_name = "Value 2";
    value_2.browse_name = "value_2";

    std::string last_call = "-- None --";
    Method add = [&](ServerView, const Variables &iargs) -> std::pair<bool, Variables> {
        int num1 = iargs[0], num2 = iargs[1];
        int res = num1 + num2;
        last_call = "Result: " + std::to_string(res);
        return {true, {res}};
    };
    add.display_name = "Add";
    add.browse_name = "add";
    add.iargs = {{"num1", tpInt32}, {"num2", tpInt32}};
    add.oargs = {{"result", tpInt32}};
    auto nd_value_1 = server.addVariableNode(value_1);
    auto nd_value_2 = server.addVariableNode(value_2);
    auto nd_add = server.addMethodNode(add);

    update(1, 1, nodestr(nd_value_1));
    update(1, 2, value_1.display_name);
    update(1, 3, value_1.browse_name);
    update(2, 1, nodestr(nd_value_2));
    update(2, 2, value_2.display_name);
    update(2, 3, value_2.browse_name);
    update(3, 1, nodestr(nd_add));
    update(3, 2, add.display_name);
    update(3, 3, add.browse_name);

    rm::ServerTimer timer(server, 50, [&](ServerView sv) {
        auto value_1 = sv.read(sv.find("value_1"));
        update(1, 4, std::to_string(value_1.cast<int>()));
        auto value_2 = sv.read(sv.find("value_2"));
        update(2, 4, std::to_string(value_2.cast<double>()));
        update(3, 4, last_call);
    });

    server.spin();
    resetCursor();
    return 0;
}
