#pragma once

#include "JuceLibraryCode/JuceHeader.h"
#include "docs.hpp"
#include <sstream>
#include "design_json.hpp"

#if INTERCEPT_STD_STREAMS
struct InterceptStringStream {
    InterceptStringStream(std::ostream & stream)
        : oldBuf(stream.rdbuf(newBuf.rdbuf()))
    {
    }
    ~InterceptStringStream() {
        std::cout.rdbuf(oldBuf);
    }
    void readAll(std::string & target) {
        char ch = newBuf.rdbuf()->sbumpc();
        while (ch != EOF) {
            target += ch;
            ch = newBuf.rdbuf()->sbumpc();
        }
    }
private:
    std::streambuf * oldBuf;
    std::stringstream newBuf;
};
#endif

template <class T>
class SubValue {
private:
    T value;
    int ctr;
    std::map<int, std::function<void(const T&)>> listeners;
public:
    SubValue() : ctr(0) {}
    int subscribe(std::function<void(const T&)> callback) {
        listeners.emplace(ctr, callback);
        return ctr++;
    }
    void unsubscribe(int handle) {
        listeners.erase(handle);
    }
    void set(int handle, const T& newValue) {
        value = newValue;
        for (const auto &l : listeners) {
            if (l.first != handle) {
                l.second(value);
            }
        }
    }
};
