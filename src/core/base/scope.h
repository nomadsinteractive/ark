#ifndef ARK_CORE_BASE_SCOPE_IMPL_H_
#define ARK_CORE_BASE_SCOPE_IMPL_H_

#include <map>

#include "core/base/api.h"
#include "core/inf/dictionary.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Scope : public Dictionary<Box> {
public:
    Scope() = default;
    Scope(const Scope& other) = default;
    Scope(Scope&& other) = default;
    Scope(const std::map<String, Box>& variables);

    template<typename T> sp<T> get(const String& name) {
        return get(name).template as<T>();
    }

    template<typename T> void put(const String& name, const sp<T>& object) {
        put(name, object.pack());
    }

//  [[script::bindings::getprop]]
    virtual Box get(const String& name) override;

    void put(const String& name, const Box& value);
    void remove(const String& name);

    const std::map<String, Box>& variables() const;
    std::map<String, Box>& variables();

private:
    std::map<String, Box> _variables;
};

}

#endif
