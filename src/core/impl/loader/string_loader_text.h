#ifndef ARK_CORE_IMPL_LOADER_STRING_LOADER_TEXT_H_
#define ARK_CORE_IMPL_LOADER_STRING_LOADER_TEXT_H_

#include "core/forwarding.h"
#include "core/inf/loader.h"

namespace ark {

class StringLoaderText : public StringLoader {
public:

    virtual String load(const sp<Readable>& readable) override;

};

}

#endif
