#pragma once

#include <set>
#include <memory>

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
    Class(TypeId id, const char* name, std::unique_ptr<IClass> delegate);
    DISALLOW_COPY_AND_ASSIGN(Class);

    TypeId id() const;
    const char* name() const;
    const std::set<TypeId>& implements() const;

    template<typename T> bool is() const {
        return is(Type<T>::id());
    }
    bool is(TypeId id) const;
    bool isInstance(TypeId id) const;

    Box cast(const Box& box, TypeId id) const;

    void setImplementation(std::set<TypeId>&& implementation);

    template<typename T> static Class* getClass() {
        return getClass(Type<T>::id());
    }
    static Class* getClass(TypeId id);
    static Class* addClass(TypeId id, const char* name, std::unique_ptr<IClass> impl);

private:
    template<typename T, typename... Args> void setImplementation() {
        _implements.insert(Type<T>::id());
        if constexpr(sizeof...(Args) > 0)
            setImplementation<Args...>();
    }

private:
    TypeId _id;
    const char* _name;
    std::set<TypeId> _implements;
    std::unique_ptr<IClass> _delegate;

    friend class ClassManager;
};

}
