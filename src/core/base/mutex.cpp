#include "core/base/mutex.h"


namespace ark {

void Mutex::wait()
{
    std::unique_lock<std::mutex> lk(_mutex);
    _condition_variable.wait(lk);
}

void Mutex::notify()
{
    _condition_variable.notify_one();
}

void Mutex::notifyAll()
{
    _condition_variable.notify_all();
}

}
