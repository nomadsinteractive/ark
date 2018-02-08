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
    typedef std::function<void(void*)> Destructor;

    Box() noexcept = default;
    Box(const Box& other) = default;
    Box(Box&& other) = default;

    Box& operator =(const Box& other) = default;
    Box& operator =(Box&& other) = default;

    explicit operator bool() const;

    TypeId typeId() const;

    template<typename T> const sp<T>& unpack() const {
        return _instance ? _instance->unpack<T>() : sp<T>::null();
    }

    template<typename T> sp<T>& unpack() {
        return _instance ? _instance->unpack<T>() : sp<T>::null();
    }

    template<typename T> sp<T> as() const {
        if(!_instance)
            return sp<T>::null();

        TypeId typeId = Type<T>::id();
        sp<T> inst = typeId == _instance->typeId() ? _instance->unpack<T>() : _instance->interfaces()->as(*this, typeId).unpack<T>();
        if(!inst) {
            const sp<Duck<T>> duck = _instance->interfaces()->as(*this, Type<Duck<T>>::id()).template unpack<Duck<T>>();
            if(duck)
                duck->to(inst);
        }
        return inst;
    }

private:
    Box(void* instance, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor&& destructor);

    class ARK_API Stub {
    public:
        Stub(void* ptr, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor&& destructor);
        ~Stub();

        void* ptr() const;
        TypeId typeId() const;
        const std::shared_ptr<Interfaces>& interfaces() const;

        template<typename T> const sp<T>& unpack() const {
            DCHECK(_type_id == Type<T>::id(), "Wrong type being unpacked");
            return _ptr ? *reinterpret_cast<sp<T>*>(_ptr) : sp<T>::null();
        }

        template<typename T> sp<T>& unpack() {
            DCHECK(_type_id == Type<T>::id(), "Wrong type being unpacked");
            return _ptr ? *reinterpret_cast<sp<T>*>(_ptr) : sp<T>::null();
        }

    private:
        void* _ptr;
        TypeId _type_id;
        std::shared_ptr<Interfaces> _interfaces;

        Destructor _destructor;
    };

private:
    std::shared_ptr<Stub> _instance;

    template<typename T> friend class SharedPtr;
};

}

#endif
