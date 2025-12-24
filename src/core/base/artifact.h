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

//  [[script::bindings::property]]
    sp<Integer> index() const;
//  [[script::bindings::operator(*)]]
    static sp<Uploader> mul(sp<Artifact> lhs, uint32_t rhs);

    bool updateRepeat(uint32_t tick, uint32_t repeatCount, Vector<int8_t>& buffer);

private:
    struct Property {
        uint32_t _offset;
        sp<Uploader> _uploader;
    };
    class Index;

    size_t loadProperties(const Scope& kwargs);
    Table<String, Property> _properties;
    sp<Index> _index;
};

}
