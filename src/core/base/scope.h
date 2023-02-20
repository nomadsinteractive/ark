#ifndef ARK_CORE_BASE_SCOPE_H_
#define ARK_CORE_BASE_SCOPE_H_

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
    Scope(std::map<String, Box> variables, sp<Queries> queries = nullptr);
    DEFAULT_COPY_AND_ASSIGN(Scope);

    template<typename T> sp<T> build(const String& name, const Scope& args) const;

    virtual Box get(const String& name) override;

    void put(const String& name, Box value);

    const std::map<String, Box>& variables() const;

    Box getObject(const String& name) const;

private:
    std::map<String, Box> _variables;
    sp<Queries> _queries;
};

}

#endif

#ifndef ARK_CORE_BASE_SCOPE_H_APPENDIX_
#define ARK_CORE_BASE_SCOPE_H_APPENDIX_

#include "core/base/bean_factory.h"
#include "core/base/queries.h"

namespace ark {

template<typename T> sp<T> Scope::build(const String& name, const Scope& args) const {
    const sp<T> obj = getObject(name).template as<T>();
    if(!obj && _queries)
        return _queries->_bean_factory.ensure().buildWithQueries<T>(name, _queries, args);
    return obj;
}

}

#endif
