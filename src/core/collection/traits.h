#pragma once

#include "core/base/api.h"
#include "core/collection/table.h"
#include "core/types/box.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"
#include "core/types/type.h"

namespace ark {

class ARK_API Traits {
public:
    Traits() noexcept = default;
    template<typename... Args> Traits(Args&&... args) {
        _add_trait(std::forward<Args>(args)...);
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Traits);

    typedef Table<TypeId, Box> TableType;

    template<typename T> bool has() const {
        return has(Type<T>::id());
    }

    bool has(TypeId typeId) const {
        return _traits.find(typeId) != _traits.end();
    }

    template<typename T> Box& put(sp<T> trait) {
        return put(Type<T>::id(), Box(std::move(trait)));
    }
    template<typename T> Box& add(sp<T> trait) {
        return add(Type<T>::id(), Box(std::move(trait)));
    }

    Box& put(TypeId typeId, Box trait) {
        Box& slot = _traits[typeId];
        slot = std::move(trait);
        return slot;
    }

    Box& add(const TypeId typeId, Box trait) {
        Box& slot = _traits[typeId];
        if(slot) {
            Box& listBox = _traits[toVectorTypeId(typeId)];
            sp<Vector<Box>> list = listBox.toPtr<Vector<Box>>();
            if(!list) {
                list = sp<Vector<Box>>::make();
                listBox = Box(list);
            }
            list->push_back(trait);
        }
        slot = std::move(trait);
        return slot;
    }

    template<typename T> sp<T> get() const {
        const Optional<Box> optBox = get(Type<T>::id());
        return optBox ? optBox->as<T>() : nullptr;
    }

    Optional<Box> get(TypeId typeId) const {
        const auto iter = _traits.find(typeId);
        return iter != _traits.end() ? Optional<Box>(iter->second) : Optional<Box>();
    }

    template<typename T> T getEnum(T defaultValue) const {
        static_assert(std::is_enum_v<T>);
        const auto iter = _traits.find(Type<T>::id());
        return iter != _traits.end() ? iter->second.template toEnum<T>() : defaultValue;
    }

    template<typename T, typename... Args> sp<T> ensure(Args&&... args) {
        return instance_sfinae<T, Args...>(0, std::forward<Args>(args)...);
    }

    const TableType& traits() const {
        return _traits;
    }

    TableType& traits() {
        return _traits;
    }

private:
    template<typename T, typename... Args> sp<T> instance_sfinae(std::enable_if_t<std::is_constructible_v<T, Args...>, int32_t>, Args&&... args) {
        if(sp<T> inst = get<T>())
            return inst;
        return put<T>(sp<T>::make(std::forward<Args>(args)...)).template toPtr<T>();
    }

    template<typename T> sp<T> instance_sfinae(...) {
        sp<T> inst = get<T>();
        DCHECK(inst, "Cannot get instance and there is no way to build one(it is abstract or has no default constructor)");
        return inst;
    }

    template<typename T, typename... Args> void _add_trait(T trait, Args&&... args) {
        Box box(std::move(trait));
        put(box.typeId(), std::move(box));
        if constexpr(sizeof...(Args) != 0)
            _add_trait(std::forward<Args>(args)...);
    }

private:
    TableType _traits;
};

}
