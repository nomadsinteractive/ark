#ifndef ARK_CORE_TYPES_SHARED_PTR_H_
#define ARK_CORE_TYPES_SHARED_PTR_H_

#include <functional>
#include <memory>

#include "core/forwarding.h"
#include "core/inf/duck.h"
#include "core/types/box.h"
#include "core/types/class.h"
#include "core/types/interfaces.h"
#include "core/types/owned_ptr.h"
#include "core/types/type.h"

namespace ark {

template<typename T> class SharedPtr {
private:
    template<typename U> class SharedPtrNode : public Interfaces::Node {
    public:
        SharedPtrNode(const SharedPtr<U>& ptr)
            : _ptr(ptr) {
        }

        virtual Box as(TypeId id) override {
            return _ptr.interfaces()->as(id);
        }

    private:
        SharedPtr<U> _ptr;
    };

    class WeakPtrNode : public Interfaces::Node {
    public:
        WeakPtrNode(Class* clazz, const SharedPtr<T>& ptr)
            : _class(clazz), _weak_ptr(ptr.ptr()), _weak_interfaces(ptr.interfaces()) {
        }

        virtual Box as(TypeId id) override {
            const std::shared_ptr<T> ptr = _weak_ptr.lock();
            const std::shared_ptr<Interfaces> interfaces = _weak_interfaces.lock();
            return (ptr && interfaces) ? _class->cast(SharedPtr<T>(ptr, interfaces).pack(), id) : Box();
        }

    private:
        Class* _class;

        std::weak_ptr<T> _weak_ptr;
        std::weak_ptr<Interfaces> _weak_interfaces;
    };

public:
    _CONSTEXPR SharedPtr() noexcept
        : _ptr(nullptr), _interfaces(nullptr) {
    }
    _CONSTEXPR SharedPtr(std::nullptr_t null) noexcept
        : _ptr(null), _interfaces(null) {
    }
    SharedPtr(const SharedPtr& other) noexcept = default;
    SharedPtr(SharedPtr&& other) noexcept = default;
    SharedPtr(const std::shared_ptr<T>& ptr, const std::shared_ptr<Interfaces>& interfaces) noexcept
        : _ptr(ptr), _interfaces(interfaces) {
        addSelfType();
    }

    template<typename U> SharedPtr(const SharedPtr<U>& ptr) noexcept
        : _ptr(std::static_pointer_cast<T>(ptr._ptr)), _interfaces(ptr._interfaces) {
        addSelfType();
    }

    template<typename U> SharedPtr(SharedPtr<U>&& ptr) noexcept
        : _ptr(std::move(ptr._ptr)), _interfaces(std::move(ptr._interfaces)) {
        addSelfType();
    }

    typedef T _PtrType;

    static SharedPtr<T> adopt(T* instance) {
        return SharedPtr<T>(instance);
    }

    template<typename... Args> static SharedPtr<T> make(Args&&... args) {
        return SharedPtr<T>(new T(std::forward<Args>(args)...));
    }

    static SharedPtr<T>& null() {
        static SharedPtr<T> inst = nullptr;
        return inst;
    }

    SharedPtr<T>& operator =(const SharedPtr<T>& other) = default;
    SharedPtr<T>& operator =(SharedPtr<T>&& other) = default;

    bool operator == (const SharedPtr<T>& sp) const {
        return _ptr == sp._ptr;
    }

    bool operator != (const SharedPtr<T>& sp) const {
        return _ptr != sp._ptr;
    }

    bool operator < (const SharedPtr<T>& sp) const {
        return _ptr < sp._ptr;
    }

    explicit operator bool() const {
        return _ptr.get() != nullptr;
    }

    T* operator ->() const {
        return _ptr.get();
    }

    operator T&() const {
        return *_ptr.get();
    }

    T& operator *() const {
        return *_ptr.get();
    }

    T* get() const {
        return _ptr.get();
    }

    Box pack() const {
        if(!_ptr)
            return Box();
        SharedPtr<T>* copy = new SharedPtr<T>(*this);
        Box box(copy, Type<T>::id(), _interfaces, [] (void* instance) {
                    SharedPtr<T>* b = reinterpret_cast<SharedPtr<T>*>(instance);
                    delete b;
                });
        return box;
    }

    const std::shared_ptr<T>& ptr() const {
        return _ptr;
    }

    const std::shared_ptr<Interfaces>& interfaces() const {
        return _interfaces;
    }

    template<typename U> const SharedPtr<U> cast() const {
        return SharedPtr<U>(std::static_pointer_cast<U>(_ptr), _interfaces);
    }

    template<typename U> SharedPtr<T>& absorb(const SharedPtr<U>& beingAbsorbed) {
        if(beingAbsorbed.interfaces() != _interfaces) {
            const std::shared_ptr<Interfaces::Node> node = std::make_shared<SharedPtrNode<U>>(beingAbsorbed);
            for(const auto iter : beingAbsorbed.interfaces()->_types)
                if(_interfaces->_types.find(iter.first) == _interfaces->_types.end())
                    _interfaces->_types[iter.first] = node;
        }
        return *this;
    }

    template<typename U> bool is() const {
        return _interfaces ? _interfaces->is<U>() : false;
    }

    template<typename U> SharedPtr<U> as() const {
        if(_interfaces) {
            sp<U> ptr = _interfaces->as<U>();
            if(!ptr) {
                const sp<Duck<U>> duckType = _interfaces->as<Duck<U>>();
                if(duckType)
                    duckType->to(ptr);
            }
            return ptr;
        }
        return nullptr;
    }

    bool unique() const {
        return _ptr.unique();
    }

private:
    SharedPtr(T* instance) noexcept
        : _ptr(instance), _interfaces(new Interfaces()) {
        addWeakType();
    }
    SharedPtr(T* ptr, const std::shared_ptr<Interfaces>& interfaces, std::function<void(T*)> deleter) noexcept
        : _ptr(ptr, deleter), _interfaces(interfaces) {
    }

    void addWeakType() {
        Class* clazz = Class::getClass<T>();
        const std::shared_ptr<Interfaces::Node> node = std::make_shared<WeakPtrNode>(clazz, *this);
        for(const TypeId i : clazz->implements())
            _interfaces->_types[i] = node;
    }

    void addSelfType() noexcept {
        if(_interfaces && !_interfaces->is<T>())
            addWeakType();
    }

    template<typename U> friend class SharedPtr;

    friend class MemoryPool;
    friend class ObjectPool;

private:
    std::shared_ptr<T> _ptr;
    std::shared_ptr<Interfaces> _interfaces;

};


template<> class SharedPtr<void> {
public:
    SharedPtr() {
        DFATAL("Illegal SharedPtr<void> class instance creation");
    }

    SharedPtr(std::nullptr_t) {
        DFATAL("Illegal SharedPtr<void> class instance creation");
    }

    SharedPtr(const std::shared_ptr<void>&, const std::shared_ptr<Interfaces>&) {
        DFATAL("Illegal SharedPtr<void> class instance creation");
    }

    Box pack() const {
        return Box();
    }

    operator bool() const {
        return false;
    }

    template<typename U> const SharedPtr<U> cast() const {
        return SharedPtr<U>();
    }

    const std::shared_ptr<Interfaces>& interfaces() const {
        return _interfaces;
    }

    template<typename U> sp<U> as() const {
        return nullptr;
    }

    template<typename U> bool is() const {
        return false;
    }

    static SharedPtr<void>& null() {
        static SharedPtr<void> s;
        return s;
    }

private:
    std::shared_ptr<Interfaces> _interfaces;

};

}

#endif
