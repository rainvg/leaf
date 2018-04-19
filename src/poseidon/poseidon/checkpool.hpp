#ifndef __poseidon__poseidon__checkpool__hpp
#define __poseidon__poseidon__checkpool__hpp

// Includes

#include "checkpool.h"
#include "poseidon.h"

namespace poseidon
{
    using namespace drop;
    using namespace vine;

    // Constructors

    template <size_t size> checkpool <size> :: checkpool(poseidon & poseidon, const size_t & threshold) : _poseidon(poseidon), _threshold(threshold)
    {
    }

    // Getters

    template <size_t size> std :: vector <index> checkpool <size> :: indexes() const
    {
        std :: vector <index> indexes;
        indexes.reserve(this->_indexes.size());

        for(const index & index : this->_indexes)
            indexes.push_back(index);

        return indexes;
    }

    // Methods

    template <size_t size> void checkpool <size> :: add(const index & index)
    {
        this->_indexes.insert(index);
    }

    template <size_t size> bool checkpool <size> :: find(const index & index)
    {
        return this->_indexes.count(index);
    }

    template <size_t size> void checkpool <size> :: set(const size_t & version, const size_t & slot, const index & index, const value & value)
    {
        if(version != this->_version || !(this->_indexes.count(index)))
            return;

        this->_values[slot][index] = value;

        size_t total = 0;
        size_t pro = 0;

        for(size_t i = 0; i < size; i++)
        {
            try
            {
                if(this->_values[i].at(index) == value)
                    pro++;

                total++;
            }
            catch(...)
            {
            }
        }

        if(pro >= this->_threshold)
        {
            this->_poseidon.accept(statement(index, value));
            this->_indexes.erase(index);
        }

        if(total < this->_threshold)
            return;

        std :: unordered_map <class value, size_t, shorthash> histogram;

        for(size_t i = 0; i < size; i++)
        {
            try
            {
                class value value = this->_values[slot].at(index);

                try
                {
                    size_t count = histogram.at(value);
                    histogram[value] = count + 1;
                }
                catch(...)
                {
                    histogram[value] = 1;
                }
            }
            catch(...)
            {
            }
        }

        struct
        {
            class value value;
            size_t score = 0;
        } best;

        for(const auto & bin : histogram)
        {
            if(bin.second > best.score)
            {
                best.value = bin.first;
                best.score = bin.second;
            }
        }

        if((total - best.score) > (size - this->_threshold))
            this->_indexes.erase(index);
    }

    template <size_t size> size_t checkpool <size> :: clear()
    {
        for(size_t i = 0; i < size; i++)
            this->_values[i].clear();

        this->_version++;
        return this->_version;
    }
};

#endif
