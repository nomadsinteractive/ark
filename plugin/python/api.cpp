#include "python/api.h"

#include "core/base/thread.h"

namespace ark {
namespace plugin {
namespace python {

namespace {

struct PythonThreadFlag {
    PythonThreadFlag()
        : _flag(0) {
    }

    uint32_t _flag;
};

}

void setThreadFlag()
{
    Thread::local<PythonThreadFlag>()._flag = 1;
}

bool getThreadFlag()
{
    return Thread::local<PythonThreadFlag>()._flag == 1;
}

void checkThreadFlag()
{
    CHECK(Thread::local<PythonThreadFlag>()._flag == 1, "Not calling from main python interpreter thread");
}

}
}
}

