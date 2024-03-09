#include "rmvl/opcua/server.hpp"

int main()
{
    rm::Server server(4840U);
    rm::Variable var = 42;
    var.display_name = "VarDemo";
    var.browse_name = "var_demo";
    server.addVariableNode(var);
    server.start();

    printf("\033[32mServer started for 20 seconds ...\033[0m\n");
    std::this_thread::sleep_for(std::chrono::seconds(20));
    printf("\033[32mServer stopped ...\033[0m\n");
    server.stop();
    server.join();
    return 0;
}
