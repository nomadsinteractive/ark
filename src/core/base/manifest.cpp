#include "core/base/manifest.h"

namespace ark {

Manifest::Manifest(String src, sp<DOMDocument> descriptor)
    : _src(std::move(src)), _descriptor(std::move(descriptor)) {
}

const String& Manifest::src() const
{
    return _src;
}

const document& Manifest::descriptor() const
{
    return _descriptor;
}

}
