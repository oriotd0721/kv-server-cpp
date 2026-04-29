#pragma once
#include "kv_store.hpp"

class Server {
public:
    explicit Server(int port);
    ~Server();

    // Blocks indefinitely, accepting and handling clients.
    void run();

private:
    int port_;
    KVStore store_;
};
