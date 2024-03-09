#include "rmvl/opcua/client.hpp"

int main()
{
    rm::Client client("opc.tcp://localhost:4840");
    auto var_id = rm::nodeObjectsFolder | client.find("var_demo");
    auto var = client.read(var_id);
    printf("\033[32mValue: %d\033[0m\n", var.cast<int>());
    return 0;
}
