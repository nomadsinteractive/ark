#ifndef ARK_CORE_INF_FUTURE_H_
#define ARK_CORE_INF_FUTURE_H_

#include "core/base/api.h"

namespace ark {

class ARK_API Future {
public:
    Future();

    void cancel();
    void done();

    bool isCancelled() const;
    bool isDone() const;

private:
    bool _cancelled;
    bool _done;
};

}

#endif
