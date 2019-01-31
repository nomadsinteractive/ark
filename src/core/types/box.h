#ifndef ARK_CORE_TYPES_BOX_H_
#define ARK_CORE_TYPES_BOX_H_

#include <functional>
#include <memory>

#include "core/base/api.h"
#include "core/inf/duck.h"
#include "core/forwarding.h"
#include "core/types/interfaces.h"
#include "core/types/null.h"
#include "core/types/type.h"

namespace ark {

class ARK_API Box {
public:
    typedef void(*Destructor)(void*);

    Box() noexcept = default;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Box);

    explicit operator bool() const;

    TypeId typeId() const;

    template<typename T> const sp<T>& unpack() const {
        return _stub ? _stub->unpack<T>() : sp<T>::null();
    }

    template<typename T> sp<T> as() const {
        if(!_stub)
            return sp<T>::null();

        TypeId typeId = Type<T>::id();
        sp<T> inst = typeId == _stub->typeId() ? _stub->unpack<T>() : _stub->interfaces()->as(*this, typeId).unpack<T>();
        if(!inst) {
            const sp<Duck<T>> duck = _stub->interfaces()->as(*this, Type<Duck<T>>::id()).template unpack<Duck<T>>();
            if(duck)
                duck->to(inst);
        }
        return inst;
    }

    Box toConcrete() const;

    void* ptr() const;
    const std::shared_ptr<Interfaces>& interfaces() const;

private:
    Box(void* instance, void* ptr, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor destructor);

    class ARK_API Stub {
    public:
        Stub(void* instance, void* ptr, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor destructor);
        ~Stub();

        void* ptr() const;
        TypeId typeId() const;
        const std::shared_ptr<Interfaces>& interfaces() const;

        template<typename T> const sp<T>& unpack() const {
            DCHECK(_type_id == Type<T>::id(), "Wrong type being unpacked");
            return _shared_ptr ? *reinterpret_cast<sp<T>*>(_shared_ptr) : sp<T>::null();
        }

        template<typename T> sp<T>& unpack() {
            DCHECK(_type_id == Type<T>::id(), "Wrong type being unpacked");
            return _shared_ptr ? *reinterpret_cast<sp<T>*>(_shared_ptr) : sp<T>::null();
        }

    private:
        void* _shared_ptr;
        void* _ptr;
        TypeId _type_id;
        std::shared_ptr<Interfaces> _interfaces;

        Destructor _destructor;

        friend class Box;
    };

private:
    std::shared_ptr<Stub> _stub;

    template<typename T> friend class SharedPtr;
};

}

#endif
