#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/safe_var.h"

namespace ark {

class ARK_API LayoutLength {
public:
//  [[script::bindings::enumeration]]
    enum LengthType {
        LENGTH_TYPE_AUTO,
        LENGTH_TYPE_PIXEL,
        LENGTH_TYPE_PERCENTAGE
    };

public:
    LayoutLength();
    LayoutLength(float pixels);
    LayoutLength(float value, LengthType type);
//  [[script::bindings::auto]]
    LayoutLength(const String& value, LayoutLength::LengthType type = LayoutLength::LENGTH_TYPE_AUTO);
//  [[script::bindings::auto]]
    LayoutLength(sp<Numeric> value, LayoutLength::LengthType type = LayoutLength::LENGTH_TYPE_PIXEL);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(LayoutLength);

    bool update(uint64_t timestamp) const;

//  [[script::bindings::property]]
    const SafeVar<Numeric>& value() const;
//  [[script::bindings::property]]
    LayoutLength::LengthType type() const;

    float toPixelLength(float parentLength) const;

    bool isMatchParent() const;

//  [[plugin::builder]]
    class BUILDER final : public IBuilder<LayoutLength> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        LayoutLength build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _value;
        LengthType _type;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public IBuilder<LayoutLength> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        LayoutLength build(const Scope& args) override;

    private:
        String _value;
        sp<Builder<Numeric>> _value_builder;
    };

private:
    SafeVar<Numeric> _value;
    LengthType _type;

    friend class LayoutParam;
};

}
