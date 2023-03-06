#pragma once

#include <map>
#include <vector>

#include "core/collection/list.h"
#include "core/inf/input.h"
#include "core/types/shared_ptr.h"

namespace ark {

class InputImpl : public Input {
public:
    InputImpl(size_t size);
    InputImpl(const std::map<size_t, sp<Input>>& inputMap, size_t size = 0);

    virtual bool update(uint64_t timestamp) override;

    virtual void upload(Writable& writable) override;

    void addInput(size_t offset, sp<Input> input);
    void removeInput(size_t offset);

    void markDirty();

private:
    struct InputStub {
        InputStub(size_t offset, sp<Input> input, sp<Boolean> disposed);

        bool isDisposed() const;

        size_t _offset;
        sp<Input> _input;
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
