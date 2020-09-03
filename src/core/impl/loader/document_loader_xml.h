#ifndef ARK_CORE_IMPL_LOADER_DOCUMENT_LOADER_XML_H_
#define ARK_CORE_IMPL_LOADER_DOCUMENT_LOADER_XML_H_

#include "core/forwarding.h"
#include "core/inf/loader.h"

namespace ark {

class DocumentLoaderXML : public DocumentLoader {
public:

    virtual document load(const sp<Readable>& readable) override;

};

}

#endif
