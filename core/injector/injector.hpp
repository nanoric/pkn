#pragma once

#include <stdexcept>

#include "../base/noncopyable.h"


namespace pkn
{
    class instance_not_set_error :public std::logic_error
    {};

    class instance_already_set_error : public std::logic_error
    {};

    template <class T>
    class SingletonInjector : protected noncopyable
    {
    public:
        inline static T &get()
        {
            if (_instance != nullptr)
                return *_instance;
            //throw instance_not_set_error();
            return *(T*)nullptr; // !!! reference to nullptr?!
        }
        inline static bool set(T *instance)
        {
            if (_instance == nullptr)
            {
                _instance = instance;
                return true;
            }
            return false;
            //throw instance_already_set_error();
        }
        static T *_instance;
    };

    template <class T>
    T * pkn::SingletonInjector<T>::_instance = nullptr;

}
