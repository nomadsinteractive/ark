#ifndef ARK_CORE_TYPES_CLASS_H_
#define ARK_CORE_TYPES_CLASS_H_

#include <set>
#include <unordered_set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/type.h"

namespace ark {

class ARK_API IClass {
public:
    virtual ~IClass() = default;

    virtual Box cast(const Box& box, TypeId id) = 0;
};

class ARK_API Class {
public:
    Class();
    Class(TypeId id);
    Class(TypeId id, const char* name, IClass* delegate);
    Class(const Class& other) = default;
    Class(Class&& other) = default;

    TypeId id() const;
    const char* name() const;
    const std::unordered_set<TypeId>& implements() const;

    bool isInstance(TypeId id) const;

    Class& operator =(const Class& other) = default;
    Class& operator =(Class&& other) = default;

    Box cast(const Box& box, TypeId id);

    void setImplementation(std::unordered_set<TypeId>&& implementation);

    template<typename T> static Class* getClass() {
        return getClass(Type<T>::id());
    }
    static Class* getClass(TypeId id);
    static Class* putClass(TypeId id, const char* name, IClass* impl);

private:
    template<typename T = void, typename... Args> void setImplementation() {
        TypeId id = Type<T>::id();
        if(id != Type<void>::id()) {
            _implements.insert(Type<T>::id());
            setImplementation<Args...>();
        }
    }

private:
    TypeId _id;
    const char* _name;
    std::unordered_set<TypeId> _implements;
    IClass* _delegate;

    friend class ClassManager;
};

}

#endif
