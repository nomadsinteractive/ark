#pragma once

#include "core/types/shared_ptr.h"
#include "core/inf/builder.h"
#include "core/inf/string_bundle.h"


namespace ark {

class StringBundleImpl final : public StringBundle {
public:
    StringBundleImpl(Map<String, sp<StringBundle>> directories);

    sp<String> getString(const String& resid) override;
    Vector<String> getStringArray(const String& resid) override;

//  [[plugin::builder]]
    class BUILDER final : public Builder<StringBundle> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<StringBundle> build(const Scope& args) override;

    private:
        Map<String, sp<Builder<StringBundle>>> _directories;
    };

private:
    sp<StringBundle> split(const String& resid, String& remains);

private:
    Map<String, sp<StringBundle>> _directories;
};

}
