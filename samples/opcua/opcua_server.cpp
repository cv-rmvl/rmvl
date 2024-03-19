#include <csignal>

#include "rmvl/opcua/server.hpp"

rm::Server *p_server{nullptr};

inline void onHandle(int) { p_server->stop(); }

int main()
{
    signal(SIGINT, onHandle);

    rm::Server server(4840);
    p_server = &server;

    rm::Variable position = 5500;
    position.display_name = "Position";
    position.browse_name = "position";
    server.addVariableNode(position);
    server.start();
    server.join();
    return 0;
}
