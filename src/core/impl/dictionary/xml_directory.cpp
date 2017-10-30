#include "core/impl/dictionary/xml_directory.h"

#include "core/util/documents.h"
#include "core/types/null.h"

namespace ark {

XMLDirectory::XMLDirectory(const sp<Asset>& resource)
    : _resource(resource) {
    NOT_NULL(_resource);
}

document XMLDirectory::get(const String& name) {
    const sp<Readable> readable = _resource->get(name);
    return readable ? Documents::loadFromReadable(readable) : document::null();
}

}
