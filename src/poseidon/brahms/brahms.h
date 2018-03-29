// Forward declarations

namespace poseidon
{
    class brahms;
};

#if !defined(__forward__) && !defined(__poseidon__brahms__brahms__h)
#define __poseidon__brahms__brahms__h

// Libraries

#include <sodium.h>

// Includes

#include "sampler.h"
#include "vine/dialers/directory.hpp"

namespace poseidon
{
    using namespace drop;
    using namespace vine;

    class brahms
    {
    public:

        // Settings

        struct settings
        {
            struct view
            {
                static constexpr size_t size = 32;
            };

            struct sample
            {
                static constexpr size_t size = 8;
            };

            static constexpr size_t alpha = 12;
            static constexpr size_t beta = 28;
            static constexpr size_t gamma = sample :: size - (alpha + beta);

            static constexpr interval interval = 1_m;
        };

        // Nested classes

        class node : public vine :: identifier
        {
            // Friends

            friend class brahms;

            // Members

            brahms & _brahms;

            // Private constructors

            node(const identifier &, brahms &);

        public:

            // Methods

            promise <dial> connect() const;
        };

    private:

        // Service nested classes

        struct pullslot
        {
            bool completed;
            identifier view[settings :: view :: size];
        };

        // Members

        signer _signer;

        identifier _view[settings :: view :: size];
        sampler _sample[settings :: sample :: size];

        pullslot _pullslots[settings :: view :: size];
        size_t _version;

        std :: mutex _mutex;

        dialers :: directory :: client _directory;
        connectors :: tcp :: async & _connector;
        pool & _pool;
        crontab & _crontab;

    public:

        // Constructors

        brahms(const identifier (&)[settings :: view :: size], const address &, connectors :: tcp :: async &, pool &, crontab &);
        brahms(const signer &, const identifier (&)[settings :: view :: size], const address &, connectors :: tcp :: async &, pool &, crontab &);

        // Getters

        const identifier & identifier() const;
        signer & signer();

    private:

        // Private methods

        void dispatch(const vine :: identifier &);
        promise <void> pull(vine :: identifier, size_t);
        promise <void> serve(pool :: connection);

        promise <void> run();
    };
};

#endif