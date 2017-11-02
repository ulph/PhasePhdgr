#pragma once

#include "JuceHeader.h"
#include "docs.hpp"
#include <sstream>
#include "json.hpp"

#if INTERCEPT_STD_STREAMS
struct InterceptStringStream {
    InterceptStringStream(std::ostream & stream)
        : newBuf()
    {
        oldBuf = stream.rdbuf(newBuf.rdbuf());
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


struct LambdaTimer : Timer {
    LambdaTimer(std::function<void()> *callBack) {
        cb = callBack;
    }
    virtual void timerCallback() {
        (*cb)();
    }
    virtual ~LambdaTimer() {
        delete cb;
    }
private:
    const std::function<void(void)> *cb;
};


template <class T>
class SubValue {
private:
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
        for (const auto &l : listeners) {
            if (l.first != handle) {
                l.second(newValue);
            }
        }
    }
};
