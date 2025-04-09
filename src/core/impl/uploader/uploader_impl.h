#pragma once

#include "core/collection/list.h"
#include "core/inf/uploader.h"
#include "core/types/shared_ptr.h"

namespace ark {

class UploaderImpl final : public Uploader {
public:
    UploaderImpl(size_t size);
    UploaderImpl(const Map<size_t, sp<Uploader>>& uploaderMap, size_t size = 0);

    bool update(uint64_t timestamp) override;
    void upload(Writable& writable) override;

    void put(size_t offset, sp<Uploader> uploader);
    void remove(size_t offset);

    void reset(sp<Uploader> uploader);
    void markDirty();

private:
    struct UploaderStub {
        size_t _offset;
        sp<Uploader> _uploader;
        sp<Boolean> _discarded;
        bool _dirty_updated = true;
        bool _dirty_marked = true;
    };

    size_t calculateUploaderSize();

    class Discardable {
    public:
        Discardable() = default;

        FilterAction operator() (const UploaderStub& item) const {
            return item._discarded->val() ? FILTER_ACTION_REMOVE : FILTER_ACTION_NONE;
        }
    };

private:
    Map<size_t, sp<Boolean::Impl>> _uploader_states;
    FList<UploaderStub, Discardable> _uploaders;
};

}
