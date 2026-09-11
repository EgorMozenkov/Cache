#pragma once

class ICache {
    public:

    virtual ~ICache () = default;

    virtual bool request (int key) = 0;
    virtual void read_cache () = 0;
};