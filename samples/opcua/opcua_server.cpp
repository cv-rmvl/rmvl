#include <csignal>

#include "rmvl/opcua/server.hpp"

rm::Server server(4840);

static inline void onHandle(int) { server.stop(); }

int main()
{
    signal(SIGINT, onHandle);

    rm::Variable position = 5500;
    position.display_name = "Position";
    position.browse_name = "position";
    server.addVariableNode(position);
    server.start();
    server.join();
    return 0;
}
