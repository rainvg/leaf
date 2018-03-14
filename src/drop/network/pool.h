// Forward declarations

namespace drop
{
    class pool;
};

#if !defined(__forward__) && !defined(__drop__network__pool__h)
#define __drop__network__pool__h

// Libraries

#include <thread>
#include <exception>
#include <unordered_map>
#include <deque>
#include <memory>

// Includes

#include "connection.hpp"
#include "queue.h"
#include "drop/thread/wakepipe.h"
#include "drop/async/promise.h"
#include "drop/chrono/time.hpp"

namespace drop
{
    class pool
    {
        // Settings

        struct settings
        {
            static constexpr interval timeout = 30_s;
            static constexpr interval interval = 1_s;
        };

    public:

        // Nested classes

        class connection
        {
            // Friends

            friend class pool;

            // Members

            pool & _pool;
            std :: shared_ptr <:: drop :: connection :: arc> _arc;

            // Private constructors

            connection(pool &, const :: drop :: connection &);

        public:

            promise <void> send(const buffer &) const;
        };

    private:

        // Members

        queue _queue;

        volatile bool _alive;
        wakepipe _wakepipe;

        std :: thread _thread;

    public:

        // Constructors

        pool();

        // Destructor

        ~pool();

        // Methods

        connection bind(const :: drop :: connection &);

    private:

        // Private methods

        promise <void> send(const std :: shared_ptr <:: drop :: connection :: arc> &, const buffer &);

        void run();
    };
};

#endif
