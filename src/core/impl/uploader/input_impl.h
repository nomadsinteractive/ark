#pragma once

#include <map>
#include <vector>

#include "core/collection/list.h"
#include "core/inf/uploader.h"
#include "core/types/shared_ptr.h"

namespace ark {

class UploaderImpl : public Uploader {
public:
    UploaderImpl(size_t size);
    UploaderImpl(const std::map<size_t, sp<Uploader>>& inputMap, size_t size = 0);

    virtual bool update(uint64_t timestamp) override;
    virtual void upload(Writable& writable) override;

    void addInput(size_t offset, sp<Uploader> input);
    void removeInput(size_t offset);

    void reset(sp<Uploader> uploader);
    void markDirty();

private:
    struct UploaderStub {
        UploaderStub(size_t offset, sp<Uploader> input, sp<Boolean> disposed);

        bool isDiscarded() const;

        size_t _offset;
        sp<Uploader> _input;
        bool _dirty_updated;
        bool _dirty_marked;
        sp<Boolean> _disposed;
    };

    size_t calculateUploaderSize();

private:
    std::map<size_t, sp<Boolean::Impl>> _uploader_states;
    List<UploaderStub, ListFilters::Disposable<UploaderStub>> _uploaders;
};

}
