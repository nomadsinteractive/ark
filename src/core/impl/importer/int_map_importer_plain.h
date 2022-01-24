#ifndef ARK_CORE_IMPL_IMPORTER_INT_MAP_IMPORTER_PLAIN_H_
#define ARK_CORE_IMPL_IMPORTER_INT_MAP_IMPORTER_PLAIN_H_

#include "core/inf/builder.h"
#include "core/inf/importer.h"

namespace ark {

class IntMapImporterPlain : public IntMapImporter {
public:
    virtual void import(IntMap& obj, const sp<Readable>& src) override;

//  [[plugin::builder("plain")]]
    class BUILDER : public Builder<IntMapImporter> {
    public:
        BUILDER() = default;

        virtual sp<IntMapImporter> build(const Scope& args) override;

    };

};

}

#endif
