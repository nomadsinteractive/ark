#ifndef ARK_CORE_IMPL_INPUT_INPUT_VARIABLE_ARRAY_H_
#define ARK_CORE_IMPL_INPUT_INPUT_VARIABLE_ARRAY_H_

#include <vector>

#include "core/inf/array.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/input.h"
#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/base/identifier.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"
#include "core/util/holder_util.h"


namespace ark {

template<typename T, size_t ALIGN = sizeof(T)> class InputVariableArray : public Input, public Holder, Implements<InputVariableArray<T, ALIGN>, Input, Holder> {
public:
    InputVariableArray(std::vector<sp<Variable<T>>> vector)
        : _vector(std::move(vector)) {
    }

    virtual void flat(void* buf) override {
        for(size_t i = 0; i < _vector.size(); ++i)
            *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(buf) + i * ALIGN) = _vector[i]->val();
    }

    virtual uint32_t size() override {
        return static_cast<uint32_t>(_vector.size() * ALIGN);
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

    class BUILDER : public Builder<Input> {
    public:
        BUILDER(BeanFactory& factory, const String& value)
            : _id(Identifier::parse(value)), _array(factory.getBuilder<Array<sp<Variable<T>>>>(value)) {
        }

        virtual sp<Input> build(const Scope& args) override {
            if(_id.isArg())
            {
//                array<sp<Variable<T>>> mats = args.build<Array<sp<Variable<T>>>>(_id.arg(), args);
//                if(mats)
//                    return sp<InputMat4fv>::make(std::move(mats));

                sp<Input> flatable = args.build<Input>(_id.arg(), args);
                if(flatable)
                    return flatable;

//                array<sp<Input>> flatables = args.build<Array<sp<Input>>>(_id.arg(), args);
//                if(flatables)
//                    return sp<InputMat4fvInputArray>::make(std::move(flatables));
            }

//            return sp<InputMat4fv>::make(_array->build(args));
            return nullptr;
        }

    private:
        Identifier _id;
        sp<Builder<Array<sp<Variable<T>>>>> _array;
    };

private:
    std::vector<sp<Variable<T>>> _vector;

};

}

#endif
