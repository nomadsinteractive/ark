#include "core/forwarding.h"

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/base/identifier.h"
#include "core/base/scope.h"
#include "core/util/documents.h"
#include "core/util/string_convert.h"

namespace ark {

template<typename T> class EnumBuilder final : public IBuilder<T> {
public:
    EnumBuilder(const document& manifest, const String& name, T defaultValue)
        : EnumBuilder(Documents::getAttribute<Identifier>(manifest, name, {}), defaultValue)
    {
    }
    EnumBuilder(Identifier id, T defaultValue)
        : _id(std::move(id)), _value(defaultValue) {
        if(_id.isVal())
            _value = StringConvert::eval<T>(_id.val());
        static_assert(std::is_enum_v<T>, "Not an Enum type");
    }

    T build(const Scope& args) override {
        if(!_id || _id.isVal())
            return _value;
        CHECK(_id.isArg(), "Only value and argument Enum Builders are supported");
        if(const Optional<Box> optVar = args.getObject(_id.arg()))
        {
            if(optVar->isEnum())
                return static_cast<T>(optVar->toInteger());
            if(const sp<Integer> intval = optVar->as<Integer>())
                return static_cast<T>(intval->val());
            return StringConvert::eval<T>(*optVar->as<String>());
        }
        return _value;
    }

private:
    Identifier _id;
    T _value;
};

}