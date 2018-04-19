#include <iostream>
#include <fstream>

#include "poseidon/poseidon/poseidon.hpp"
#include "drop/network/connectors/tcp.h"
#include "drop/network/pool.hpp"
#include "drop/chrono/crontab.h"
#include "vine/dialers/local.h"
#include "vine/network/multiplexer.hpp"

using namespace drop;
using namespace vine;
using namespace poseidon;

static constexpr size_t nodes = 256;

std :: array <identifier, brahms :: settings :: view :: size> view(std :: array <signer, nodes> & signers, size_t exclude)
{
    std :: array <identifier, brahms :: settings :: view :: size> sample;

    for(size_t i = 0; i < brahms :: settings :: view :: size; i++)
    {
        size_t pick = rand() % nodes;

        while(pick == exclude)
            pick = rand() % nodes;

        sample[i] = signers[pick].publickey();
    }

    return sample;
}

void seppuku()
{
    while(true)
    {
        char buffer[1024];
        std :: cin >> buffer;

        if(strcmp("seppuku", buffer) == 0)
            *((int *) nullptr) = 99;
    }
}

int main()
{
    std :: thread seppuku_thread(seppuku);
    seppuku_thread.detach();

    std :: ofstream mute;
    mute.open("/dev/null", std :: ios :: out);

    connectors :: tcp :: async connector;
    pool pools[8];
    crontab crontabs[8];

    dialers :: local :: server server;

    std :: array <signer, nodes> signers;
    std :: array <multiplexer <dialers :: local :: client, 3> *, nodes> dialers;
    std :: array <class poseidon *, nodes> clients;

    std :: cout << "Creating nodes" << std :: endl;

    for(size_t i = 0; i < nodes; i++)
    {
        auto view = :: view(signers, i);
        dialers[i] = new multiplexer <dialers :: local :: client, 3> (server, signers[i], pools[i % 8]);
        clients[i] = new class poseidon(signers[i], view, *(dialers[i]), pools[i % 8], crontabs[i % 8], ((i == 0 && false) ? std :: cout : mute));
    }

    std :: cout << "Registering handlers" << std :: endl;

    clients[0]->on <events :: gossip> ([&](const statement & statement)
    {
        std :: cout << timestamp(now) << " Statement gossiped: " << statement.identifier() << " / " << statement.sequence() << ": " << statement.value() << std :: endl;
    });

    clients[0]->on <events :: accept> ([&](const statement & statement)
    {
        std :: cout << timestamp(now) << " Statement accepted: " << statement.identifier() << " / " << statement.sequence() << ": " << statement.value() << std :: endl;
    });

    std :: cout << "Starting nodes" << std :: endl;

    for(size_t i = 0; i < nodes; i++)
    {
        std :: cout << "Starting node " << i << std :: endl;
        clients[i]->start();
        sleep(20_ms);
    }

    std :: cout << "Started" << std :: endl;

    sleep(1_m);

    for(uint64_t i = 0;; i++)
    {
        char message[1024];
        sprintf(message, "Message number %llu", i);

        std :: cout << "Seeding " << message << std :: endl;
        clients[44]->publish(i, message);

        sleep(1_s);
    }

    sleep(10_h);
}
