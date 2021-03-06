// Includes

#include "local.h"

namespace vine :: dialers
{
    using namespace drop;

    // server

    // Private methods

    void local :: server :: add(const identifier & identifier, client * client)
    {
        this->_mutex.lock();
        this->_clients[identifier] = client;
        this->_mutex.unlock();
    }

    void local :: server :: remove(const identifier & identifier)
    {
        this->_mutex.lock();
        this->_clients.erase(identifier);
        this->_mutex.unlock();
    }

    promise <dial> local :: server :: connect(const identifier & fromid, const identifier & toid)
    {
        try
        {
            this->_mutex.lock();

            client * from;
            client * to;

            try
            {
                from = this->_clients.at(fromid);
                to = this->_clients.at(toid);
            }
            catch(...)
            {
                throw exceptions :: node_not_found();
            }

            sockets :: socketpair socketpair;

            connection fromconn(socketpair.alpha);
            connection toconn(socketpair.beta);

            class secretbox :: nonce fromtxnonce = secretbox :: nonce :: random();
            class secretbox :: nonce totxnonce = secretbox :: nonce :: random();

            fromconn.authenticate(from->keyexchanger(), to->keyexchanger().publickey(), fromtxnonce, totxnonce);
            toconn.authenticate(to->keyexchanger(), from->keyexchanger().publickey(), totxnonce, fromtxnonce);

            to->emit <dial> ({fromid, toconn});

            this->_mutex.unlock();
            return promise <dial> :: resolved(dial(toid, fromconn));
        }
        catch(...)
        {
            this->_mutex.unlock();
            return promise <dial> :: rejected(std :: current_exception());
        }
    }

    // client

    // Constructors

    local :: client :: client(server & server) : _server(server)
    {
        this->_server.add(this->_signer.publickey(), this);
    }

    local :: client :: client(server & server, const class signer & signer) : _server(server), _signer(signer)
    {
        this->_server.add(this->_signer.publickey(), this);
    }

    local :: client :: client(server & server, const class signer & signer, const class keyexchanger & keyexchanger) : _server(server), _signer(signer), _keyexchanger(keyexchanger)
    {
        this->_server.add(this->_signer.publickey(), this);
    }

    // Destructor

    local :: client :: ~client()
    {
        this->_server.remove(this->_signer.publickey());
    }

    // Getters

    const identifier & local :: client :: identifier() const
    {
        return this->_signer.publickey();
    }

    signer & local :: client :: signer()
    {
        return this->_signer;
    }

    keyexchanger & local :: client :: keyexchanger()
    {
        return this->_keyexchanger;
    }

    // Methods

    promise <dial> local :: client :: connect(const vine :: identifier & identifier)
    {
        return this->_server.connect(this->_signer.publickey(), identifier);
    }
}
