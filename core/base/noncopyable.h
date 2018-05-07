#pragma once

class noncopyable
{
protected:
    noncopyable() = default;
    noncopyable(noncopyable &) = delete;
    noncopyable(noncopyable &&) = delete;
    void operator=(noncopyable &) = delete;
    void operator=(noncopyable &&) = delete;
};