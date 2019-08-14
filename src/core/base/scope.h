#ifndef ARK_CORE_BASE_SCOPE_IMPL_H_
#define ARK_CORE_BASE_SCOPE_IMPL_H_

#include <map>

#include "core/base/api.h"
#include "core/inf/dictionary.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"

namespace ark {

/*

class BoxBundle {
    [[script::bindings::getprop]]
    virtual Box get(const String& name) override;
};

*/

class ARK_API Scope : public BoxBundle {
public:
    Scope() = default;
    Scope(std::map<String, Box> variables);
    DEFAULT_COPY_AND_ASSIGN(Scope);

    template<typename T> sp<T> get(const String& name) {
        return get(name).template as<T>();
    }

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
