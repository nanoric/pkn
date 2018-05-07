#pragma once

#include <stdexcept>

#include "../base/noncopyable.h"


namespace pkn
{
    class instance_not_set_error :public std::exception
    {};

    class instance_already_set_error : public std::exception
    {};

    template <class T>
    class SingletonInjector : protected noncopyable
    {
    public:
        static T & get()
        {
            if (_instance != nullptr)
                return *_instance;
            throw instance_not_set_error();
        }
        static void set(T *instance)
        {
            if (_instance == nullptr)
            {
                _instance = instance;
                return;
            }
            throw instance_already_set_error();
        }
        static T *_instance;
    };

    template <class T>
    T * pkn::SingletonInjector<T>::_instance = nullptr;

}
