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

    bool has(const TypeId typeId) const {
        return _table.find(typeId) != _table.end();
    }

    template<typename T> Box& put(sp<T> trait) {
        return put(Type<T>::id(), Box(std::move(trait)));
    }
    template<typename T> void add(sp<T> trait) {
        add(Type<T>::id(), Box(std::move(trait)));
    }

    Box& put(const TypeId typeId, Box trait) {
        Box& slot = _table[typeId];
        slot = std::move(trait);
        return slot;
    }

    void add(const TypeId typeId, Box trait) {
        if(_table.find(typeId) != _table.end()) {
            Box& listBox = _table[toVectorTypeId(typeId)];
            sp<Vector<Box>> list = listBox.toPtr<Vector<Box>>();
            if(!list) {
                list = sp<Vector<Box>>::make();
                list->push_back(_table[typeId]);
                listBox = Box(list);
            }
            list->push_back(std::move(trait));
        }
        else
            _table[typeId] = std::move(trait);
    }

    template<typename T> sp<T> get() const {
        const Optional<Box> optBox = get(Type<T>::id());
        return optBox ? optBox->as<T>() : nullptr;
    }

    Optional<Box> get(const TypeId typeId) const {
        const auto iter = _table.find(typeId);
        return iter != _table.end() ? Optional<Box>(iter->second) : Optional<Box>();
    }

    template<typename T> T getEnum(T defaultValue) const {
        static_assert(std::is_enum_v<T>);
        const auto iter = _table.find(Type<T>::id());
        return iter != _table.end() ? iter->second.template toEnum<T>() : defaultValue;
    }

    template<typename T, typename... Args> sp<T> ensure(Args&&... args) {
        return instance_sfinae<T, Args...>(0, std::forward<Args>(args)...);
    }

    const TableType& table() const {
        return _table;
    }

    TableType& table() {
        return _table;
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
    TableType _table;
};

}
