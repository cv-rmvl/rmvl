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
    std::cout << "Please input the value to write: ";
    Tp val{};
    std::cin >> val;
    client.write(node, val);
    std::cout << "\033[32mSuccess to write\033[0m\n";
}

template <typename Tp>
void value(ClientView client, const rm::NodeId &node)
{
    std::cout << "- 0: Read\n- 1: Write\n";
    std::cout << "Please input the number to operate: \n";
    std::string num{};
    std::cin >> num;
    if (num == "0")
        read<Tp>(client, node);
    else if (num == "1")
        write<Tp>(client, node);
    else
        printf("\033[31mInvalid number\033[0m\n");
}

void call(Client &client)
{
    int num1{}, num2{};
    std::cout << "Calculate the sum of 'num1' and 'num2':\n";
    std::cout << "Input num1: ";
    std::cin >> num1;
    std::cout << "Input num2: ";
    std::cin >> num2;
    auto [result, oargs] = client.call("add", {num1, num2});
    if (result)
        std::cout << "\033[32mResult: " << oargs[0].cast<int>() << "\033[0m\n";
    else
        std::cout << "\033[31mFail to call the method node\033[0m\n";
}

int main()
{
    Client cli("opc.tcp://127.0.0.1:4840");
    if (!cli.ok())
        return -1;
    while (true)
    {
        std::cout << "- 0: value_1\n- 1: value_2\n- 2: add\n- q: Exit\n";
        std::cout << "Please input the number to operate:\n";
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
            printf("\033[31mInvalid number\033[0m\n");
    }

    return 0;
}
