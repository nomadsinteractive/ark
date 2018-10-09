#ifndef ARK_CORE_TYPES_INTERFACES_H_
#define ARK_CORE_TYPES_INTERFACES_H_

#include <memory>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/type.h"

namespace ark {

class ARK_API Interfaces {
public:
    Interfaces(Class* clazz);

    template<typename T> bool is() const {
        return is(Type<T>::id());
    }

    template<typename T> bool isInstance() const {
        return isInstance(Type<T>::id());
    }

    template<typename T> SharedPtr<T> as(const Box& box) const {
        return as(box, Type<T>::id()).template unpack<T>();
    }

    void absorb(const Box& other);
    TypeId typeId() const;
    const std::unordered_set<TypeId>& implements() const;
    Box as(const Box& box, TypeId id) const;
    bool is(TypeId id) const;
    bool isInstance(TypeId id) const;
    void reset();

private:
    Class* _class;
    std::map<TypeId, Box> _attachments;

    template<typename T> friend class SharedPtr;
};

}

#endif
