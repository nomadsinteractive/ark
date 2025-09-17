#pragma once

#include <condition_variable>
#include <mutex>

namespace ark {

class Mutex {
public:

    void wait();

    template<typename T> bool wait(T duration) {
        std::unique_lock<std::mutex> lk(_mutex);
        return _condition_variable.wait_for(lk, duration) == std::cv_status::timeout;
    }

    template<typename T, typename U> bool wait(T duration, U pred) {
        std::unique_lock<std::mutex> lk(_mutex);
        return _condition_variable.wait_for(lk, duration, std::move(pred));
    }

    void notify();
    void notifyAll();

private:
    std::mutex _mutex;
    std::condition_variable _condition_variable;
};

}
