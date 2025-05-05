#pragma once

#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/inf/storage.h"

#include "renderer/forwarding.h"

namespace ark {

class AtlasImporterGenericXML final : public AtlasImporter {
public:
    AtlasImporterGenericXML(String src, float px, float py);

    void import(Atlas& atlas, const sp<Readable>& readable) override;

//  [[plugin::builder("generic-xml")]]
    class BUILDER final : public Builder<AtlasImporter> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<AtlasImporter> build(const Scope& args) override;

    private:
        sp<Builder<String>> _src;
        float _px;
        float _py;
    };

private:
    String _src;
    float _px;
    float _py;

};

}
