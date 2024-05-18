#pragma once

#include <map>

#include "core/base/api.h"
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

    template<typename T> bool has() const {
        return has(Type<T>::id());
    }

    bool has(TypeId typeId) const {
        return _traits.find(typeId) != _traits.end();
    }

    template<typename T> Box& put(sp<T> trait) {
        return put(Type<T>::id(), trait);
    }

    Box& put(TypeId typeId, Box trait) {
        DCHECK_WARN(!has(typeId), "Overriding trait TypeId(%d)", typeId);
        Box& slot = _traits[typeId];
        slot = std::move(trait);
        return slot;
    }

    template<typename T> const sp<T>& get() const {
        const Optional<Box> optBox = get(Type<T>::id());
        return optBox ? optBox->template toPtr<T>() : sp<T>::null();
    }

    Optional<Box> get(TypeId typeId) const {
        const auto iter = _traits.find(typeId);
        return iter != _traits.end() ? Optional<Box>(iter->second) : Optional<Box>();
    }

    template<typename T> T getEnum(T defaultValue) const {
        static_assert(std::is_enum_v<T>);
        const auto iter = _traits.find(Type<T>::id());
        return iter != _traits.end() ? iter->second.toEnum<T>() : defaultValue;
    }

    template<typename T, typename... Args> const sp<T>& ensure(Args&&... args) {
        return instance_sfinae<T, Args...>(0, std::forward<Args>(args)...);
    }

private:
    template<typename T, typename... Args> const sp<T>& instance_sfinae(std::enable_if_t<std::is_constructible_v<T, Args...>, int32_t>, Args&&... args) {
        const sp<T>& inst = get<T>();
        if(inst)
            return inst;
        return put<T>(sp<T>::make(std::forward<Args>(args)...)).template toPtr<T>();
    }

    template<typename T> const sp<T>& instance_sfinae(...) {
        const sp<T>& inst = get<T>();
        DCHECK(inst, "Cannot get instance and there is no way to build one(it is abstract or has no default constructor)");
        return inst;
    }

    template<typename T, typename... Args> void _add_trait(Box trait, Args&&... args) {
        put(trait.typeId(), std::move(trait));
        if constexpr(sizeof...(Args) != 0)
            _add_trait(std::forward<Args>(args)...);
    }

private:
    std::map<TypeId, Box> _traits;
};

}
