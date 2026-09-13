#pragma once

template <typename Key>
class ICache {
    public:

    virtual ~ICache () = default;

    virtual bool request (const Key& key) = 0;
    virtual void read_cache () = 0;
};