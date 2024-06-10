#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"
#include "core/util/documents.h"

namespace ark {

class ARK_API Manifest {
public:
//  [[script::bindings::auto]]
    Manifest(String src, sp<DOMDocument> descriptor = nullptr);

//  [[script::bindings::property]]
    const String& src() const;
//  [[script::bindings::property]]
    const document& descriptor() const;

    template<typename T> T getAttribute(const String& name, const T& defValue) const {
        return _descriptor ? Documents::getAttribute<T>(_descriptor, name, defValue) : defValue;
    }

private:
    String _src;
    document _descriptor;
};

}
