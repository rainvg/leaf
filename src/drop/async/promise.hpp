#ifndef __drop__async__promise__hpp
#define __drop__async__promise__hpp

// Includes

#include "promise.h"
#include "drop/data/optional.hpp"
#include "drop/utils/sfinae.hpp"

namespace drop
{
    // Traits

    template <typename type> template <typename lambda> constexpr bool promise <type> :: traits :: is_callback()
    {
        if constexpr (std :: is_same <type, void> :: value)
            return sfinae :: is_valid([](auto && callback) -> decltype(callback()) {}).template satisfied <lambda> ();
        else
            return sfinae :: is_valid([](auto && callback) -> decltype(callback(std :: declval <const type &> ())) {}).template satisfied <lambda> ();
    }

    template <typename type> template <typename lambda> constexpr bool promise <type> :: traits :: is_chainable()
    {
        if constexpr (!(is_callback <lambda> ()))
            return false;
        else
        {
            if constexpr (std :: is_same <type, void> :: value)
                return is_promise <decltype(std :: declval <lambda> ()())> :: value;
            else
                return is_promise <decltype(std :: declval <lambda> ()(std :: declval <const type &> ()))> :: value;
        }
    }

    template <typename type> template <typename lambda> constexpr bool promise <type> :: traits :: is_handler()
    {
        return sfinae :: is_valid([](auto && handler) -> decltype(handler(std :: declval <const std :: exception_ptr &> ())) {}).template satisfied <lambda> ();
    }

    template <typename type> template <typename lambda> auto promise <type> :: traits :: declthen()
    {
        if constexpr (!(is_chainable <lambda> ()))
            return;
        else
        {
            if constexpr (std :: is_same <type, void> :: value)
                return std :: declval <lambda> ()();
            else
                return std :: declval <lambda> ()(std :: declval <const type &> ());
        }
    }

    // Constraints

    template <typename type> template <typename lambda> constexpr bool promise <type> :: constraints :: callback()
    {
        return traits :: template is_callback <lambda> ();
    }

    template <typename type> template <typename lambda> constexpr bool promise <type> :: constraints :: handler()
    {
        return traits :: template is_handler <lambda> ();
    }

    template <typename type> template <typename... vtype> constexpr bool promise <type> :: constraints :: value()
    {
        if constexpr (std :: is_same <type, void> :: value)
            return (sizeof...(vtype) == 0);
        else
            return (sizeof...(vtype) == 1) && (... && (std :: is_same <vtype, type> ()));
    }

    // Exceptions

    template <typename type> const char * promise <type> :: exceptions :: unforwardable_reject :: what() const throw()
    {
        return "Failed to forward a promise rejection to all branches of the promise's chain.";
    }

    template <typename type> const char * promise <type> :: exceptions :: already_resolved :: what() const throw()
    {
        return "Promise was already resolved.";
    }

    template <typename type> const char * promise <type> :: exceptions :: already_rejected :: what() const throw()
    {
        return "Promise was already rejected.";
    }

    // resolver_interface

    // Destructor

    template <typename type> promise <type> :: resolver_interface :: ~resolver_interface()
    {
    }

    // resolver <lambda, false>

    // Constructors

    template <typename type> template <typename lambda> promise <type> :: resolver <lambda, false> :: resolver(const lambda & callback) : _callback(callback)
    {
    }

    // Methods

    template <typename type> template <typename lambda> void promise <type> :: resolver <lambda, false> :: resolve(const arc & arc)
    {
        arc.call(this->_callback);
    }

    template <typename type> template <typename lambda> void promise <type> :: resolver <lambda, false> :: reject(const std :: exception_ptr & exception)
    {
        throw (class exceptions :: unforwardable_reject){};
    }

    // resolver <lambda, true>

    // Constructors

    template <typename type> template <typename lambda> promise <type> :: resolver <lambda, true> :: resolver(const lambda & callback) : _callback(callback)
    {
    }

    // Getters

    template <typename type> template <typename lambda> const auto & promise <type> :: resolver <lambda, true> :: chain() const
    {
        return this->_chain;
    }

    // Methods

    template <typename type> template <typename lambda> void promise <type> :: resolver <lambda, true> :: resolve(const arc & arc)
    {
        this->_chain.alias(arc.call(this->_callback));
    }

    template <typename type> template <typename lambda> void promise <type> :: resolver <lambda, true> :: reject(const std :: exception_ptr & exception)
    {
        this->_chain.reject(exception);
    }

    // rejector_interface

    // Destructor

    template <typename type> promise <type> :: rejector_interface :: ~rejector_interface()
    {
    }

    // rejector

    // Constructors

    template <typename type> template <typename lambda> promise <type> :: rejector <lambda> :: rejector(const lambda & handler) : _handler(handler)
    {
    }

    // Methods

    template <typename type> template <typename lambda> void promise <type> :: rejector <lambda> :: reject(const std :: exception_ptr & exception)
    {
        this->_handler(exception);
    }

    // arc

    // Constructors

    template <typename type> promise <type> :: arc :: arc() : _value(), _size(0), _rejector(nullptr)
    {
    }

    // Destructor

    template <typename type> promise <type> :: arc :: ~arc()
    {
        if(!(this->_alias))
        {
            if(this->_exception)
            {
                if(this->_rejector)
                    this->_rejector->reject(this->_exception);
                else
                {
                    for(size_t i = 0; i < this->_size; i++)
                        this->_resolvers[i]->reject(this->_exception);
                }
            }

            if(!(this->_value))
            {
                for(size_t i = 0; i < this->_size; i++)
                    delete this->_resolvers[i];
            }

            if(this->_rejector)
                delete this->_rejector;
        }
    }

    template <typename type> template <typename lambda> typename promise <type> :: traits :: template chain <lambda> promise <type> :: arc :: then(const lambda & callback)
    {
        if(this->_alias)
            return this->_alias->then(callback);
        else
        {
            if(this->_value)
                return this->call(callback);
            else
            {
                resolver <lambda> * resolver = new promise <type> :: resolver <lambda> (callback);
                this->_resolvers[this->_size++] = resolver;

                if constexpr (traits :: template is_chainable <lambda> ())
                    return resolver->chain();
                else
                    return;
            }
        }
    }

    template <typename type> template <typename lambda> void promise <type> :: arc :: except(const lambda & callback)
    {
        if(!(this->_rejector))
            this->_rejector = new promise <type> :: rejector <lambda> (callback);
    }

    template <typename type> template <typename vtype> void promise <type> :: arc :: resolve(const vtype & value)
    {
        if(this->_value)
            throw (class exceptions :: already_resolved){};

        if(this->_exception)
            throw (class exceptions :: already_rejected){};

        if constexpr (!(std :: is_same <type, void> :: value))
            this->_value = value;
        else
            this->_value = true;

        for(size_t i = 0; i < this->_size; i++)
        {
            this->_resolvers[i]->resolve(*this);
            delete this->_resolvers[i];
        }
    }

    template <typename type> template <typename etype> void promise <type> :: arc :: reject(const etype & exception)
    {
        this->reject(std :: make_exception_ptr(exception));
    }

    template <typename type> void promise <type> :: arc :: reject(const std :: exception_ptr & exception)
    {
        if(this->_value)
            throw (class exceptions :: already_resolved){};

        if(this->_exception)
            throw (class exceptions :: already_rejected){};

        this->_exception = exception;
    }

    template <typename type> template <typename lambda> typename promise <type> :: traits :: template chain <lambda> promise <type> :: arc :: call(const lambda & callback) const
    {
        if constexpr (std :: is_same <type, void> :: value)
            return callback();
        else
            return callback(*(this->_value));
    }

    template <typename type> void promise <type> :: arc :: alias(const std :: shared_ptr <arc> & that)
    {
        if(this->_value) // TODO: Check if I need this check.
            throw (class exceptions :: already_resolved){};

        if(this->_exception) // TODO: Check if I need this check.
            throw (class exceptions :: already_rejected){};

        if(that->_value)
        {
            for(size_t i = 0; i < this->_size; i++)
            {
                this->_resolvers[i]->resolve(*that);
                delete this->_resolvers[i];
            }

            if(this->_rejector)
                delete this->_rejector;
        }
        else
        {
            if(!(that->_rejector))
                that->_rejector = this->_rejector;
            else if(this->_rejector)
                delete this->_rejector;

            if(that->_exception && that->_rejector)
                for(size_t i = 0; i < this->_size; i++)
                    delete this->_resolvers[i];
            else
                for(size_t i = 0; i < this->_size; i++)
                    that->_resolvers[(that->_size)++] = this->_resolvers[i];
        }

        this->_alias = that;
    }

    template <typename type> void promise <type> :: arc :: lock()
    {
        this->_mutex.lock();
    }

    template <typename type> void promise <type> :: arc :: unlock()
    {
        this->_mutex.unlock();
    }

    // promise

    // Constructors

    template <typename type> promise <type> :: promise() : _arc(new arc)
    {
    }

    // Methods

    template <typename type> template <typename lambda, std :: enable_if_t <promise <type> :: constraints :: template callback <lambda> ()> *> auto promise <type> :: then(const lambda & callback) const
    {
        if constexpr (traits :: template is_chainable <lambda> ())
        {
            this->_arc->lock();
            auto chain = this->_arc->then(callback);
            this->_arc->unlock();

            return chain;
        }
        else
        {
            this->_arc->lock();
            this->_arc->then(callback);
            this->_arc->unlock();

            return (*this);
        }
    }

    template <typename type> template <typename lambda, std :: enable_if_t <promise <type> :: constraints :: template handler <lambda> ()> *> const promise <type> & promise <type> :: except(const lambda & handler) const
    {
        this->_arc->lock();
        this->_arc->except(handler);
        this->_arc->unlock();

        return (*this);
    }

    template <typename type> template <typename... vtype, std :: enable_if_t <promise <type> :: constraints :: template value <vtype...> ()> *> void promise <type> :: resolve(const vtype & ... value) const
    {
        this->_arc->lock();
        this->_arc->resolve(value...);
        this->_arc->unlock();
    }

    template <typename type> template <typename etype> void promise <type> :: reject(const etype & exception) const
    {
        this->_arc->lock();
        this->_arc->reject(exception);
        this->_arc->unlock();
    }

    // Private methods

    template <typename type> void promise <type> :: alias(const promise & that) const
    {
        this->_arc->lock();
        that._arc->lock();

        this->_arc->alias(that._arc);

        that._arc->unlock();
        this->_arc->unlock();
    }

    // Coroutine interface

    // promise <void>

    // Object

    auto promise <void> :: promise_type :: get_return_object()
    {
        return this->_promise;
    }

    // Suspends

    auto promise <void> :: promise_type :: initial_suspend()
    {
        return std :: experimental :: suspend_never();
    }

    auto promise <void> :: promise_type :: final_suspend()
    {
        return std :: experimental :: suspend_never();
    }

    // Exceptions

    void promise <void> :: promise_type :: unhandled_exception()
    {
        std :: cout << "Exception!" << std :: endl;
        std :: terminate();
    }

    // Returns

    void promise <void> :: promise_type :: return_void()
    {
        this->_promise.resolve();
    }
};

#endif