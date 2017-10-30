#ifndef ARK_CORE_IMPL_DIRECTORY_XML_DIRECTORY_H_
#define ARK_CORE_IMPL_DIRECTORY_XML_DIRECTORY_H_

#include "core/inf/readable.h"
#include "core/types/shared_ptr.h"

#include "core/inf/dictionary.h"

namespace ark {

class XMLDirectory : public Dictionary<document> {
public:
    XMLDirectory(const sp<Asset>& resource);

    virtual document get(const String& name);

private:
    sp<Asset> _resource;
};

}

#endif
