#pragma once

#include <sstream>
#include <atomic>

#include <phasephckr.hpp>

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
    unsigned int ctr;
    std::map<unsigned int, std::function<void(const T&)>> listeners;

public:
    SubValue() : ctr(0) {}
    
    unsigned int subscribe(std::function<void(const T&)> callback) {
        assert(listeners.size() < std::numeric_limits<unsigned int>::max());
        while (listeners.count(ctr)) { ctr++; }
        listeners.emplace(ctr, callback);
        return ctr;
    }
    
    void unsubscribe(unsigned int handle) {
        listeners.erase(handle);
    }

    void set(unsigned int handle, const T& newValue) const {
        for (const auto &l : listeners) {
            if (l.first != handle) {
                l.second(newValue);
            }
        }
    }
};
