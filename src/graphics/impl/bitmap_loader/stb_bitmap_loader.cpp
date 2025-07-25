#include "graphics/impl/bitmap_loader/stb_bitmap_loader.h"

#ifdef ARK_USE_STB_IMAGE

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "core/inf/array.h"
#include "core/inf/readable.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"

namespace ark {

namespace {

struct STBUserCallback {
    sp<Readable> _readable;
    bool _eof;

    STBUserCallback(const sp<Readable>& readable)
        : _readable(readable), _eof(false) {
    }
};

int _stb_read_callback(void* user, char *data, const int size)
{
    const STBUserCallback* u = static_cast<STBUserCallback*>(user);
    return static_cast<int>(u->_readable->read(data, static_cast<uint32_t>(size)));
}

void _stb_skip_callback(void* user, const int n)
{
    const STBUserCallback* u = static_cast<STBUserCallback*>(user);
    u->_readable->seek(n, SEEK_CUR);
}

int _stb_eof_callback(void *user)
{
    const STBUserCallback* u = static_cast<STBUserCallback*>(user);
    return static_cast<int>(u->_eof);
}

class STBImageByteArray final : public Array<uint8_t> {
public:
    STBImageByteArray(void* array, const size_t length)
        : _array(array), _length(length) {
    }
    ~STBImageByteArray() {
        stbi_image_free(_array);
    }

    uint8_t* buf() override {
        return reinterpret_cast<uint8_t*>(_array);
    }

    size_t length() override {
        return _length;
    }

private:
    void* _array;
    size_t _length;
};

}

STBBitmapLoader::STBBitmapLoader(bool justDecodeBounds)
    : _just_decode_bounds(justDecodeBounds)
{
}

bitmap STBBitmapLoader::load(const sp<Readable>& readable)
{
    stbi_io_callbacks callback;
    callback.read = _stb_read_callback;
    callback.skip = _stb_skip_callback;
    callback.eof = _stb_eof_callback;

    STBUserCallback user(readable);

    int width, height, channels;
    if(_just_decode_bounds)
    {
        const int ret = stbi_info_from_callbacks(&callback, &user, &width, &height, &channels);
        DCHECK(ret, "stbi_info_from_callbacks failure: %s", stbi_failure_reason());
        return bitmap::make(width, height, 0, channels, false);
    }

    const uint32_t componentSize = stbi_is_hdr_from_callbacks(&callback, &user) ? 4 : 1;
    readable->seek(0, SEEK_SET);
    void* bytes = componentSize == 1 ? reinterpret_cast<void*>(stbi_load_from_callbacks(&callback, &user, &width, &height, &channels, 0))
                                     : reinterpret_cast<void*>(stbi_loadf_from_callbacks(&callback, &user, &width, &height, &channels, 0));
    DCHECK(bytes, "stbi_load_from_callbacks failure: %s", stbi_failure_reason());

    uint32_t stride = width * channels * componentSize;
    return bitmap::make(width, height, stride, static_cast<uint8_t>(channels), sp<ByteArray>::make<STBImageByteArray>(bytes, stride * height));
}

}

#endif
