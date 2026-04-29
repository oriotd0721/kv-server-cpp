#include "server.hpp"
#include <iostream>

int main() {
    try {
        Server server(6379);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
