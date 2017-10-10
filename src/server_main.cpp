#include "Server/Server.h"

#undef main

int main() {
    bool should_run = true;
    Server server;
    server.run(&should_run);

    return 0;
}
