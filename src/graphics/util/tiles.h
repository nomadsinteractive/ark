#ifndef ARK_GRAPHICS_UTIL_TILES_H_
#define ARK_GRAPHICS_UTIL_TILES_H_

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/tile_maker.h"

namespace ark {

template<typename T> class Tile {
public:
    Tile()
        : _offset(0), _position(-1) {
    }
    Tile(const T& renderer, int32_t offset)
        : _renderer(renderer), _offset(offset), _position(-1) {
    }
    Tile(const Tile& other)
        : _renderer(other._renderer), _offset(other._offset), _position(other._position) {
    }

    explicit operator bool() const {
        return static_cast<bool>(_renderer);
    }

    void render(RenderCommandPipeline& pipeline, int32_t scroll, float x, float y) {
        _renderer.render(pipeline, scroll, x, y);
    }

    int32_t offset() const {
        return _offset;
    }

    void setOffset(int32_t offset) {
        _offset = offset;
    }

    void roll(int32_t offset) {
        _offset += offset;
    }

    int32_t position() const {
        return _position;
    }

    void setPosition(int32_t position) {
        _position = position;
    }

    const T& renderer() const {
        return _renderer;
    }

    T& renderer() {
        return _renderer;
    }

private:
    T _renderer;

    int32_t _offset;
    int32_t _position;

};

template<typename T> class RollingList {
public:
    RollingList(uint32_t itemCount)
        : _items(new T[itemCount]), _item_count(itemCount), _head(0) {
    }
    ~RollingList() {
        delete[] _items;
    }

    void roll(int32_t offset) {
        _head = (_head + (offset >= 0 ? offset : offset + _item_count)) % _item_count;
    }

    const T& operator[](uint32_t index) const {
        DCHECK(index < _item_count, "Index out of bounds");
        return _items[(_head + index) % _item_count];
    }

    T& operator[](uint32_t index) {
        DCHECK(index < _item_count, "Index out of bounds");
        return _items[(_head + index) % _item_count];
    }

private:
    T* _items;
    uint32_t _item_count;
    uint32_t _head;
};

}

#endif
