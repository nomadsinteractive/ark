#ifndef ARK_CORE_IMPL_INPUT_INPUT_VARIABLE_ARRAY_H_
#define ARK_CORE_IMPL_INPUT_INPUT_VARIABLE_ARRAY_H_

#include <vector>

#include "core/inf/holder.h"
#include "core/inf/uploader.h"
#include "core/inf/variable.h"
#include "core/inf/writable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"
#include "core/util/holder_util.h"


namespace ark {

template<typename T, size_t ALIGN = sizeof(T)> class InputVariableArray : public Uploader, public Holder, Implements<InputVariableArray<T, ALIGN>, Uploader, Holder> {
public:
    InputVariableArray(std::vector<sp<Variable<T>>> vector)
        : Uploader(vector.size() * ALIGN), _vector(std::move(vector)) {
    }

    virtual void upload(Writable& buf) override {
        for(size_t i = 0; i < _vector.size(); ++i)
            buf.writeObject(_vector[i]->val(), sizeof(T), i * ALIGN);
//            *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(buf) + i * ALIGN) = _vector[i]->val();
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = false;
        for(const sp<Variable<T>>& i : _vector)
            dirty = i->update(timestamp) || dirty;
        return dirty;
    }

    virtual void traverse(const Visitor& visitor) override {
        for(const sp<Variable<T>>& i : _vector)
            HolderUtil::visit(i, visitor);
    }

private:
    std::vector<sp<Variable<T>>> _vector;

};

}

#endif
