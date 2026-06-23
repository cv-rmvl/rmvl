#include "rmvl/io/netapp.hpp"
#include <string>

using namespace rm;
using namespace std::string_view_literals;

int main(int argc, char *argv[]) {
    uint16_t port{12345};
    if (argc == 2) {
        if (argv[1] == "--help"sv || argv[1] == "-help"sv || argv[1] == "--h"sv || argv[1] == "-h"sv) {
            printf("Usage: %s [port]", argv[0]);
        } else
            port = static_cast<uint16_t>(std::stoi(argv[1]));
    }

#if __cplusplus >= 202002L
    async::IOContext io_context{};
    async::Webapp app(io_context);
    async::HttpServer server(app);

    app.use(cors());
    app.use(statics("/", "./"));

    server.listen(port, [=]() {
        printf("HTTP Server is listening on %d\n", port);
    });

    co_spawn(io_context, &async::HttpServer::spin, &server);
    io_context.run();

#else
    printf("Webapp is not supported.\n");
#endif
}
