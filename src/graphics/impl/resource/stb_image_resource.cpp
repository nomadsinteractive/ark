#include "graphics/impl/resource/stb_image_resource.h"

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

static int _stb_read_callback(void* user, char *data,int size)
{
    STBUserCallback* u = reinterpret_cast<STBUserCallback*>(user);
    return static_cast<int>(u->_readable->read(data, static_cast<uint32_t>(size)));
}

static void _stb_skip_callback(void* user, int n)
{
    STBUserCallback* u = reinterpret_cast<STBUserCallback*>(user);
    u->_readable->seek(n, SEEK_CUR);
}

static int _stb_eof_callback(void *user)
{
    STBUserCallback* u = reinterpret_cast<STBUserCallback*>(user);
    return static_cast<int>(u->_eof);
}

class STBImageArray : public Array<uint8_t> {
public:
    STBImageArray(stbi_uc* array, uint32_t length)
        : _array(array), _length(length) {
    }
    ~STBImageArray() {
        stbi_image_free(_array);
    }

    virtual uint8_t* array() override {
        return reinterpret_cast<uint8_t*>(_array);
    }

    virtual uint32_t length() override {
        return _length;
    }

private:
    stbi_uc* _array;
    uint32_t _length;
};

}

STBImageResource::STBImageResource(const sp<Asset>& asset, bool justDecodeBounds)
    : _asset(asset), _just_decode_bounds(justDecodeBounds)
{
}

bitmap STBImageResource::get(const String& name)
{
    const sp<Readable> readable = _asset->get(name);
    return readable ? load(readable) : nullptr;
}

bitmap STBImageResource::load(const sp<Readable>& readable)
{
    stbi_io_callbacks callback;
    callback.read = _stb_read_callback;
    callback.skip = _stb_skip_callback;
    callback.eof = _stb_eof_callback;

    STBUserCallback user(readable);

    int width, height, channels;
    if(_just_decode_bounds)
    {
        int ret = stbi_info_from_callbacks(&callback, &user, &width, &height, &channels);
        DCHECK(ret, "stbi_info_from_callbacks failure: %s", stbi_failure_reason());
        return bitmap::make(width, height, 0, channels);
    }

    stbi_uc* bytes = stbi_load_from_callbacks(&callback, &user, &width, &height, &channels, 0);
    DCHECK(bytes, "stbi_load_from_callbacks failure: %s", stbi_failure_reason());

    uint32_t stride = width * channels;
    return bitmap::make(width, height, stride, static_cast<uint8_t>(channels), sp<STBImageArray>::make(bytes, stride * height));
}

}

#endif
