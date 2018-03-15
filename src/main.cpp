#include <iostream>
#include <thread>

// Includes

#include "drop/network/connectors/tcp.h"
#include "drop/network/pool.hpp"

using namespace drop;

int main()
{
    pool my_pool;

    connection my_connection = connectors :: tcp :: sync :: connect({"127.0.0.1", 1234});
    pool :: connection my_pool_connection = my_pool.bind(my_connection);

    my_pool_connection.send(buffer("Hello World!")).then([&]()
    {
        std :: cout << "Send successful." << std :: endl;
    });

    sleep(1_h);
}
