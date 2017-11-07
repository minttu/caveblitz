#include <CLI/CLI.hpp>

#include "Server/Server.h"

#undef main

int main(int argc, char **argv) {
    CLI::App app("caveblitz-server");

    uint16_t port = 30320;
    app.add_option("-p,--port", port, "Port to host server on");

    CLI11_PARSE(app, argc, argv);

    bool should_run = true;
    Server server(port);
    server.run(&should_run);

    return 0;
}
