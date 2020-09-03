#include "core/impl/loader/document_loader_xml.h"

#include "core/types/shared_ptr.h"
#include "core/util/documents.h"

namespace ark {

document DocumentLoaderXML::load(const sp<Readable>& readable)
{
    return Documents::loadFromReadable(readable);
}

}
