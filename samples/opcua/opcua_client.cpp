#include <iostream>

#include "rmvl/opcua/client.hpp"

using namespace rm;

template <typename Tp>
void read(ClientView client, const rm::NodeId &node)
{
    auto val = client.read(node).cast<Tp>();
    std::cout << "\033[32mValue: "<< val << "\033[0m\n";
}

template <typename Tp>
void write(ClientView client, const rm::NodeId &node)
{
    std::cout << "请输入要写入的值: ";
    Tp val{};
    std::cin >> val;
    client.write(node, val);
    std::cout << "\033[32m写入成功\033[0m\n";
}

template <typename Tp>
void value(ClientView client, const rm::NodeId &node)
{
    std::cout << "- 0: 读取\n- 1: 写入\n";
    std::cout << "请输入要操作的编号: \n";
    std::string num{};
    std::cin >> num;
    if (num == "0")
        read<Tp>(client, node);
    else if (num == "1")
        write<Tp>(client, node);
    else
        printf("\033[31m无效的编号\033[0m\n");
}

void call(Client &client)
{
    int num1{}, num2{};
    std::cout << "计算两数之和 num1 + num2:\n";
    std::cout << "请输入 num1: ";
    std::cin >> num1;
    std::cout << "请输入 num2: ";
    std::cin >> num2;
    auto [result, oargs] = client.call("add", {num1, num2});
    if (result)
        std::cout << "\033[32m计算结果: " << oargs[0].cast<int>() << "\033[0m\n";
    else
        std::cout << "\033[31m调用失败\033[0m\n";
}

int main()
{
    Client cli("opc.tcp://127.0.0.1:4840");
    if (!cli.ok())
        return -1;
    while (true)
    {
        std::cout << "- 0: value_1\n- 1: value_2\n- 2: add\n- q: 退出程序\n";
        std::cout << "请输入要操作的编号: \n";
        std::string num{};
        std::cin >> num;
        if (num == "0")
            value<int>(cli, cli.find("value_1"));
        else if (num == "1")
            value<double>(cli, cli.find("value_2"));
        else if (num == "2")
            call(cli);
        else if (num == "q")
            break;
        else
            printf("\033[31m无效的编号\033[0m\n");
    }

    return 0;
}
