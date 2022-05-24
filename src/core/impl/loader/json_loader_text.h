#ifndef ARK_CORE_IMPL_LOADER_JSON_LOADER_TEXT_H_
#define ARK_CORE_IMPL_LOADER_JSON_LOADER_TEXT_H_

#include "core/forwarding.h"
#include "core/inf/loader.h"

namespace ark {

class JsonLoaderText : public JsonLoader {
public:

    virtual Json load(const sp<Readable>& readable) override;

};

}

#endif
