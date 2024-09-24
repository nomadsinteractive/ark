#pragma once

#include <vector>

#include "core/inf/uploader.h"
#include "core/inf/variable.h"
#include "core/inf/writable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"


namespace ark {

template<typename T, size_t ALIGN = sizeof(T)> class InputVariableArray final : public Uploader, Implements<InputVariableArray<T, ALIGN>, Uploader> {
public:
    InputVariableArray(std::vector<sp<Variable<T>>> vector)
        : Uploader(vector.size() * ALIGN), _vector(std::move(vector)) {
    }

    void upload(Writable& buf) override {
        for(size_t i = 0; i < _vector.size(); ++i)
            buf.writeObject(_vector[i]->val(), sizeof(T), i * ALIGN);
    }

    bool update(uint64_t timestamp) override {
        bool dirty = false;
        for(const sp<Variable<T>>& i : _vector)
            dirty = i->update(timestamp) || dirty;
        return dirty;
    }

private:
    std::vector<sp<Variable<T>>> _vector;
};

}
