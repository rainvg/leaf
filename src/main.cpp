#include <iostream>
#include <fstream>

#include "poseidon/poseidon/crawler.h"
#include "poseidon/poseidon/gossiper.h"
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

int main()
{
    std :: ofstream mute;
    mute.open("/dev/null", std :: ios :: out);

    connectors :: tcp :: async connector;
    pool pool;
    crontab crontab;

    dialers :: local :: server server;

    std :: array <signer, nodes> signers;
    std :: array <multiplexer <dialers :: local :: client, 3> *, nodes> dialers;
    std :: array <gossiper *, nodes> gossipers;
    std :: array <crawler *, nodes> crawlers;

    std :: cout << "Creating nodes" << std :: endl;

    for(size_t i = 0; i < nodes; i++)
    {
        auto view = :: view(signers, i);
        dialers[i] = new multiplexer <dialers :: local :: client, 3> (server, signers[i], pool);
        gossipers[i] = new gossiper(crontab);

        crawlers[i] = new crawler(signers[i], view, *(gossipers[i]), *(dialers[i]), pool, crontab, ((i == 0) ? std :: cout : mute));
    }

    std :: cout << "Starting nodes" << std :: endl;

    for(size_t i = 0; i < nodes; i++)
    {
        std :: cout << "Starting node " << i << std :: endl;
        crawlers[i]->start();
        sleep(30_ms);
    }

    std :: cout << "Started" << std :: endl;

    while(true)
    {
        char buffer[1024];
        std :: cin.getline(buffer, 1024);
        if(strcmp(buffer, "seppuku") == 0)
            (*(int *) nullptr) = -1;
    }
}
