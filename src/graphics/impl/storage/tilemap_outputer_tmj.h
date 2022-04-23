#ifndef ARK_GRAPHICS_IMPL_STORAGE_TILEMAP_OUTPUTER_TMJ_H_
#define ARK_GRAPHICS_IMPL_STORAGE_TILEMAP_OUTPUTER_TMJ_H_

#include "core/inf/builder.h"
#include "core/inf/storage.h"

#include "graphics/forwarding.h"

namespace ark {

class TilemapOutputerTmj : public Outputer<Tilemap> {
public:

    virtual void output(Tilemap& obj, const sp<Writable>& out) override;

//  [[plugin::builder::by-value("tmj")]]
    class DICTIONARY : public Builder<Outputer<Tilemap>> {
    public:
        DICTIONARY() = default;

        virtual sp<Outputer<Tilemap>> build(const Scope& args) override;
    };

};

}

#endif
