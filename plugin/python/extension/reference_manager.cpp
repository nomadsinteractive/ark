#include "python/extension/reference_manager.h"

#include "core/inf/asset.h"
#include "core/util/strings.h"

namespace ark {
namespace plugin {
namespace python {

std::vector<sp<PyInstance>> ReferenceManager::refs()
{
    std::vector<sp<PyInstance>> r;

    for(auto iter = _refs.begin(); iter != _refs.end();)
    {
        const sp<PyInstance> ref = iter->lock();
        if(ref)
        {
            r.push_back(ref);
            ++iter;
        }
        else
        {
            iter = _refs.erase(iter);
            if(iter == _refs.end())
                break;
        }
    }

    return r;
}

void ReferenceManager::track(const WeakPtr<PyInstance>& ref)
{
    _refs.push_back(ref);
}

}
}
}
