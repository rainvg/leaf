// Includes

#include "tcp.h"

namespace drop :: connectors
{
    // sync

    // Static methods

    connection tcp :: sync :: connect(const address & remote)
    {
        sockets :: tcp socket;
        socket.connect(remote);
        return connection(socket);
    }

    // async

    // Settings

    constexpr interval tcp :: async :: settings :: timeout;
    constexpr interval tcp :: async :: settings :: interval;

    // Constructors

    tcp :: async :: async() : _alive(true)
    {
        int wake[2];
        pipe(wake);

        this->_wake.read = wake[0];
        this->_wake.write = wake[1];

        fcntl(this->_wake.read, F_SETFL, O_NONBLOCK);
        this->_queue.add <queue :: read> (this->_wake.read);

        this->_thread = std :: thread(&async :: run, this);
    }

    // Destructor

    tcp :: async :: ~async()
    {
        this->_alive = false;
        this->wake();
        this->_thread.join();

        close(this->_wake.read);
        close(this->_wake.write);
    }

    // Methods

    promise <connection> tcp :: async :: connect(const address & remote)
    {
        request request;
        request.socket.block(false);

        this->_mutex.lock();

        try
        {
            request.socket.connect(remote);
            this->_new.push(request);
            this->wake();
        }
        catch(...)
        {
            request.promise.reject(std :: current_exception());
        }

        this->_mutex.unlock();

        return request.promise;
    }

    // Private methods

    void tcp :: async :: run()
    {
        while(true)
        {
            size_t count = this->_queue.select(settings :: interval);

            if(!(this->_alive))
                break;

            for(size_t i = 0; i < count; i++)
            {
                if(this->_queue[i].type() == queue :: read)
                {
                    char buffer;
                    while(read(this->_wake.read, &buffer, 1) >= 0);
                }
                else
                {
                    request request = this->_pending[this->_queue[i].descriptor()];
                    this->_pending.erase(this->_queue[i].descriptor());

                    this->_queue.remove <queue :: write> (this->_queue[i].descriptor());

                    try
                    {
                        request.socket.rethrow();
                        request.promise.resolve(connection(request.socket));
                    }
                    catch(...)
                    {
                        request.promise.reject(std :: current_exception());
                    }
                }
            }

            timestamp target = now;
            while(this->_timeouts.size() && (this->_timeouts.front().timeout < target))
            {
                try
                {
                    request request = this->_pending.at(this->_timeouts.front().descriptor);

                    if(request.version == this->_timeouts.front().version)
                    {
                        this->_pending.erase(this->_timeouts.front().descriptor);
                        this->_queue.remove <queue :: write> (this->_timeouts.front().descriptor);

                        request.socket.close();
                        request.promise.reject(sockets :: exceptions :: connect_timeout());
                    }
                }
                catch(...)
                {
                }

                this->_timeouts.pop_front();
            }

            while(optional <request> request = this->_new.pop())
            {
                this->_queue.add <queue :: write> (request->socket.descriptor());

                request->version = this->_version++;
                this->_pending[request->socket.descriptor()] = *request;
                this->_timeouts.push_back({.descriptor = request->socket.descriptor(), .timeout = timestamp(now) + settings :: timeout, .version = request->version});
            }
        }
    }

    void tcp :: async :: wake()
    {
        char buffer = '\0';
        write(this->_wake.write, &buffer, 1);
    }
};