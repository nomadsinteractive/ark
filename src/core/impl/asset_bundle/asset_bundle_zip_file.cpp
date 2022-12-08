#include "core/impl/asset_bundle/asset_bundle_zip_file.h"

#include "core/inf/asset.h"
#include "core/inf/readable.h"
#include "core/impl/asset_bundle/asset_bundle_with_prefix.h"
#include "core/util/strings.h"

#include "platform/platform.h"

namespace ark {

namespace {

class ReadableZipFile : public Readable {
public:
    ReadableZipFile(const sp<AssetBundleZipFile::Stub>& stub, zip_file_t* zf)
        : _stub(stub), _zip_file(zf){
    }

    ~ReadableZipFile() override {
        zip_fclose(_zip_file);
    }

    virtual uint32_t read(void* buffer, uint32_t bufferSize) override {
        return static_cast<uint32_t>(zip_fread(_zip_file, buffer, bufferSize));
    }

    virtual int32_t seek(int32_t position, int32_t whence) override {
        return zip_fseek(_zip_file, position, whence);
    }

    virtual int32_t remaining() override {
        DFATAL("Unimplemented");
        return 0;
    }

private:
    sp<AssetBundleZipFile::Stub> _stub;
    zip_file_t* _zip_file;
};

class AssetZipEntry : public Asset {
public:
    AssetZipEntry(const sp<AssetBundleZipFile::Stub>& stub, const String& location, zip_file_t* zf)
        : _stub(stub), _location(location), _zip_file(zf){
    }

    virtual sp<Readable> open() override {
        return sp<ReadableZipFile>::make(_stub, _zip_file);
    }

    virtual String location() override {
        return _location;
    }

private:
    sp<AssetBundleZipFile::Stub> _stub;
    String _location;
    zip_file_t* _zip_file;

};

}

static zip_int64_t _local_zip_source_callback(void *userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd)
{
    AssetBundleZipFile::Stub* stub = reinterpret_cast<AssetBundleZipFile::Stub*>(userdata);
    switch(cmd)
    {
    case ZIP_SOURCE_TELL:
        return stub->position();
    case ZIP_SOURCE_STAT:
    {
        zip_stat_t* stat = reinterpret_cast<zip_stat_t*>(data);
        zip_stat_init(stat);
        stat->valid = ZIP_STAT_SIZE;
        stat->size = stub->size();
        return sizeof(zip_stat_t);
    }
    case ZIP_SOURCE_OPEN:
        return 0;
    case ZIP_SOURCE_READ:
        return stub->readable()->read(data, static_cast<uint32_t>(len));
    case ZIP_SOURCE_CLOSE:
        return 0;
    case ZIP_SOURCE_FREE:
        return 0;
    case ZIP_SOURCE_ERROR:
        return 0;
    case ZIP_SOURCE_SEEK:
    {
        zip_error_t _zip_error;
        zip_source_args_seek_t* args = ZIP_SOURCE_GET_ARGS(zip_source_args_seek_t, data, len, &_zip_error);
        return stub->readable()->seek(static_cast<int32_t>(args->offset), args->whence);
    }
    case ZIP_SOURCE_SUPPORTS:
        return ZIP_SOURCE_SUPPORTS_SEEKABLE;
    default:
        break;
    }
    return -1;
}

AssetBundleZipFile::AssetBundleZipFile(const sp<Readable>& zipReadable, const String& zipLocation)
    : _stub(sp<Stub>::make(zipReadable, zipLocation))
{
}

sp<Asset> AssetBundleZipFile::getAsset(const String& name)
{
    zip_int64_t idx = zip_name_locate(_stub->archive(), name.c_str(), 0);
    zip_file_t* zf = idx >= 0 ? zip_fopen_index(_stub->archive(), static_cast<zip_uint64_t>(idx), 0) : nullptr;
    if(zf)
        return sp<AssetZipEntry>::make(_stub, Strings::sprintf("%s/%s", _stub->location().c_str(), name.c_str()), zf);
    return nullptr;
}

sp<AssetBundle> AssetBundleZipFile::getBundle(const String& path)
{
    return sp<AssetBundleWithPrefix>::make(sp<AssetBundleZipFile>::make(*this), path.endsWith("/") ? path : path + "/");
}

bool AssetBundleZipFile::hasEntry(const String& name) const
{
    return zip_name_locate(_stub->archive(), name.c_str(), 0) != -1;
}

AssetBundleZipFile::Stub::Stub(const sp<Readable>& zipReadable, const String& zipLocation)
    : _zip_readable(zipReadable), _zip_location(Platform::getRealPath(zipLocation)), _size(_zip_readable ? _zip_readable->remaining() : 0)
{
    CHECK(_zip_readable, "Cannot open file %s", _zip_location.c_str());
    zip_error_t error = {0};
    _zip_source = zip_source_function_create(_local_zip_source_callback, this, &error);
    CHECK(_zip_source, "Zip function create error: %s", zip_error_strerror(&error));
    _zip_archive = zip_open_from_source(_zip_source, 0, &error);
    CHECK(_zip_archive, "Zip open error: %s", zip_error_strerror(&error));
}

AssetBundleZipFile::Stub::~Stub()
{
    zip_close(_zip_archive);
}

const sp<Readable>& AssetBundleZipFile::Stub::readable() const
{
    return _zip_readable;
}

const String& AssetBundleZipFile::Stub::location() const
{
    return _zip_location;
}

int32_t AssetBundleZipFile::Stub::size() const
{
    return _size;
}

int32_t AssetBundleZipFile::Stub::position() const
{
    return _size - _zip_readable->remaining();
}

zip_t* AssetBundleZipFile::Stub::archive()
{
    return _zip_archive;
}

zip_source_t* AssetBundleZipFile::Stub::source()
{
    return _zip_source;
}

}
