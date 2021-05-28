#include "core/impl/runnable/runnable_composite.h"

namespace ark {

RunnableComposite::RunnableComposite(std::vector<sp<Runnable>> composites)
    : _composites(std::move(composites))
{
}

void RunnableComposite::run()
{
    for(const sp<Runnable>& i : _composites)
        i->run();
}

}
