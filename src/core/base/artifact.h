#pragma once

#include "core/base/scope.h"
#include "core/collection/table.h"
#include "core/inf/uploader.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Uploader)]]
class ARK_API Artifact final : public Uploader {
public:
//  [[script::bindings::auto]]
    Artifact(const Scope& kwargs = {});

    bool update(uint32_t tick) override;
    void upload(Writable& buf) override;

//  [[script::bindings::classmethod]]
    static sp<Uploader> makeUploader(sp<Artifact> artifact, uint32_t repeatCount = 1, sp<IntegerWrapper> index = nullptr);

    bool updateRepeat(uint32_t tick, uint32_t repeatCount, Vector<int8_t>& buffer, const sp<IntegerWrapper>& index, int32_t baseIndex);

private:
    struct Property {
        uint32_t _offset;
        sp<Uploader> _uploader;
    };

    size_t loadProperties(const Scope& kwargs);
    Table<String, Property> _properties;
};

}
