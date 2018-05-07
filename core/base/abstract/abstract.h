#pragma once

#include <stdexcept>


#define NOT_IMPLEMENT_FUNCTION_BODY {throw not_implement_exception();}

#define PURE_VIRTUAL_FUNCTION_BODY = 0;

class not_implement_exception : public std::logic_error
{
public:
    not_implement_exception() :std::logic_error("not implemented!") {}
};
