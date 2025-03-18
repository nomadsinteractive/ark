#include "python/extension/reference_manager.h"

namespace ark::plugin::python {

Vector<sp<PyInstanceRef>> ReferenceManager::refs()
{
    Vector<sp<PyInstanceRef>> r;

    for(auto iter = _refs.begin(); iter != _refs.end(); )
    {
        const sp<PyInstanceRef> ref = iter->lock();
        if(ref)
        {
            r.push_back(ref);
            ++iter;
        }
        else
            iter = _refs.erase(iter);
    }

    return r;
}

void ReferenceManager::track(const WeakPtr<PyInstanceRef>& ref)
{
    _refs.push_back(ref);
}

}
