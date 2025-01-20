#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Tileset {
public:
//  [[script::bindings::auto]]
    Tileset(sp<Size> tileSize, sp<TilesetImporter> importer = nullptr);

//  [[script::bindings::property]]
    const std::unordered_map<int32_t, sp<Tile>>& tiles() const;

//  [[script::bindings::property]]
    const sp<Size>& tileSize() const;
//  [[script::bindings::property]]
    float tileWidth() const;
//  [[script::bindings::property]]
    float tileHeight() const;

//  [[script::bindings::auto]]
    void addTile(sp<Tile> t);
//  [[script::bindings::auto]]
    sp<Tile> getTile(int32_t id) const;

//  [[script::bindings::auto]]
    void load(const sp<Readable>& src);
//  [[script::bindings::auto]]
    void load(const String& src);

//  [[plugin::builder]]
    class BUILDER final : public Builder<Tileset> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Tileset> build(const Scope& args) override;

    private:
        float _tile_width;
        float _tile_height;
        builder<TilesetImporter> _importer;
    };

private:
    sp<Size> _tile_size;
    sp<TilesetImporter> _importer;

    HashMap<int32_t, sp<Tile>> _tiles;
};

}
