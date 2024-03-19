#include "rmvl/opcua/client.hpp"

int main()
{
    rm::Client client("opc.tcp://localhost:4840");
    auto position_id = rm::nodeObjectsFolder | client.find("position");
    auto position = client.read(position_id);
    printf("\033[32mValue: %d\033[0m\n", position.cast<int>());
    return 0;
}
