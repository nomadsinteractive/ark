#ifndef ARK_GRAPHICS_BASE_TILESET_H_
#define ARK_GRAPHICS_BASE_TILESET_H_

#include <unordered_map>

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Tileset {
public:
//  [[script::bindings::auto]]
    Tileset(uint32_t tileWidth, uint32_t tileHeight, const sp<TilesetImporter>& importer = nullptr);

//  [[script::bindings::property]]
    uint32_t tileWidth() const;
//  [[script::bindings::property]]
    uint32_t tileHeight() const;

//  [[script::bindings::auto]]
    void addTile(sp<Tile> t);
//  [[script::bindings::auto]]
    const sp<Tile>& getTile(int32_t id) const;

//  [[script::bindings::auto]]
    void load(const sp<Readable>& readable);
//  [[script::bindings::auto]]
    void load(const String& src);

//  [[plugin::builder]]
    class BUILDER : public Builder<Tileset> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Tileset> build(const Scope& args) override;

    private:
        uint32_t _tile_width;
        uint32_t _tile_height;
        sp<Builder<TilesetImporter>> _importer;
    };

private:
    uint32_t _tile_width;
    uint32_t _tile_height;
    sp<TilesetImporter> _importer;

    std::unordered_map<int32_t, sp<Tile>> _tiles;
};

}

#endif
