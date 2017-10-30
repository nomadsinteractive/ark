#ifndef ARK_CORE_TYPES_INTERFACES_H_
#define ARK_CORE_TYPES_INTERFACES_H_

#include <memory>
#include <map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/type.h"

namespace ark {

class ARK_API Interfaces {
private:
    class Node {
    public:
        virtual ~Node() = default;
        virtual Box as(TypeId id) = 0;
    };

public:
    Interfaces() = default;

    template<typename T> bool is() const {
        return isInstance(Type<T>::id());
    }

    template<typename T> SharedPtr<T> as() const {
        return as(Type<T>::id()).template unpack<T>();
    }

    Box as(TypeId id) const;
    bool isInstance(TypeId id) const;

private:
    std::map<TypeId, std::shared_ptr<Node>> _types;

    template<typename T> friend class SharedPtr;
};

}

#endif
