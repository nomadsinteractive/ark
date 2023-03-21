#pragma once

#include <map>
#include <vector>

#include "core/collection/list.h"
#include "core/inf/uploader.h"
#include "core/types/shared_ptr.h"

namespace ark {

class InputImpl : public Uploader {
public:
    InputImpl(size_t size);
    InputImpl(const std::map<size_t, sp<Uploader>>& inputMap, size_t size = 0);

    virtual bool update(uint64_t timestamp) override;

    virtual void upload(Writable& writable) override;

    void addInput(size_t offset, sp<Uploader> input);
    void removeInput(size_t offset);

    void markDirty();

private:
    struct InputStub {
        InputStub(size_t offset, sp<Uploader> input, sp<Boolean> disposed);

        bool isDisposed() const;

        size_t _offset;
        sp<Uploader> _input;
        bool _dirty_updated;
        bool _dirty_marked;
        sp<Boolean> _disposed;
    };

    size_t calculateUploaderSize();

private:
    std::map<size_t, sp<Boolean::Impl>> _inputs_disposed;
    List<InputStub, ListFilters::Disposable<InputStub>> _inputs;
};

}
