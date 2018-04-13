// Forward declarations

namespace poseidon
{
    template <size_t> class checkpool;
};

#if !defined(__forward__) && !defined(__poseidon__poseidon__checkpool__h)
#define __poseidon__poseidon__checkpool__h

// Libraries

#include <vector>
#include <unordered_set>

// Forward includes

#define __forward__
#include "poseidon.h"
#undef __forward__

// Includes

#include "statement.hpp"
#include "drop/data/buffer.hpp"
#include "drop/crypto/shorthash.hpp"

namespace poseidon
{
    using namespace drop;
    using namespace vine;

    template <size_t size> class checkpool
    {
        // Friends

        friend class poseidon;

        // Members

        std :: vector <index> _indexes;
        std :: vector <optional <buffer>> _slots[size];

        size_t _version;

        // Methods

        size_t init(const std :: unordered_set <index, shorthash> &);
        void set(const size_t &, const size_t &, const std :: vector <optional <buffer>> &);

        template <size_t threshold, typename alambda, typename rlambda> void evaluate(const alambda &, const rlambda &);
    };
};

#endif