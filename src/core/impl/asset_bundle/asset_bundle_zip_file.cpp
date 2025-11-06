#include "core/impl/asset_bundle/asset_bundle_zip_file.h"

#include "core/inf/asset.h"
#include "core/inf/readable.h"
#include "core/impl/asset_bundle/asset_bundle_with_prefix.h"
#include "core/util/strings.h"

#include "platform/platform.h"

namespace ark {

class AssetBundleZipFile::Stub {
public:
    Stub(sp<Readable> zipReadable, const String& zipLocation)
        : _zip_readable(std::move(zipReadable)), _zip_location(Platform::getRealPath(zipLocation)), _size(_zip_readable ? _zip_readable->remaining() : 0)
    {
        CHECK(_zip_readable, "Cannot open file %s", _zip_location.c_str());
        zip_error_t error = {0};
        _zip_source = zip_source_function_create(_local_zip_source_callback, this, &error);
        CHECK(_zip_source, "Zip function create error: %s", zip_error_strerror(&error));
        _zip_archive = zip_open_from_source(_zip_source, 0, &error);
        CHECK(_zip_archive, "Zip open error: %s", zip_error_strerror(&error));
    }
    ~Stub()
    {
        zip_close(_zip_archive);
    }

    const sp<Readable>& readable() const
    {
        return _zip_readable;
    }

    const String& location() const
    {
        return _zip_location;
    }
    int32_t size() const
    {
        return _size;
    }
    int32_t position() const
    {
        return _size - _zip_readable->remaining();
    }

    zip_t* archive()
    {
        return _zip_archive;
    }

    zip_source_t* source()
    {
        return _zip_source;
    }

private:
    sp<Readable> _zip_readable;
    String _zip_location;
    int32_t _size;

    zip_t* _zip_archive;
    zip_source_t* _zip_source;
};

class AssetBundleZipFile::ReadableZipFile final : public Readable {
public:
    ReadableZipFile(const sp<AssetBundleZipFile::Stub>& stub, zip_file_t* zf)
        : _stub(stub), _zip_file(zf){
    }

    ~ReadableZipFile() override {
        zip_fclose(_zip_file);
    }

    uint32_t read(void* buffer, uint32_t bufferSize) override {
        return static_cast<uint32_t>(zip_fread(_zip_file, buffer, bufferSize));
    }

    int32_t seek(int32_t position, int32_t whence) override {
        return zip_fseek(_zip_file, position, whence);
    }

    int32_t remaining() override {
        DFATAL("Unimplemented");
        return 0;
    }

private:
    sp<AssetBundleZipFile::Stub> _stub;
    zip_file_t* _zip_file;
};

class AssetBundleZipFile::AssetZipEntry final : public Asset {
public:
    AssetZipEntry(const sp<AssetBundleZipFile::Stub>& stub, String location, zip_file_t* zf)
        : _stub(stub), _location(std::move(location)), _zip_file(zf){
    }

    sp<Readable> open() override {
        return sp<Readable>::make<ReadableZipFile>(_stub, _zip_file);
    }

    String location() override {
        return _location;
    }

private:
    sp<AssetBundleZipFile::Stub> _stub;
    String _location;
    zip_file_t* _zip_file;
};

zip_int64_t AssetBundleZipFile::_local_zip_source_callback(void *userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd)
{
    const Stub* stub = static_cast<Stub*>(userdata);
    switch(cmd)
    {
    case ZIP_SOURCE_TELL:
        return stub->position();
    case ZIP_SOURCE_STAT:
    {
        zip_stat_t* stat = static_cast<zip_stat_t*>(data);
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

AssetBundleZipFile::AssetBundleZipFile(sp<Readable> zipReadable, const String& zipLocation)
    : _stub(sp<Stub>::make(std::move(zipReadable), zipLocation))
{
}

sp<Asset> AssetBundleZipFile::getAsset(const String& name)
{
    const zip_int64_t idx = zip_name_locate(_stub->archive(), name.c_str(), 0);
    zip_file_t* zf = idx >= 0 ? zip_fopen_index(_stub->archive(), static_cast<zip_uint64_t>(idx), 0) : nullptr;
    if(zf)
        return sp<Asset>::make<AssetZipEntry>(_stub, Strings::sprintf("%s/%s", _stub->location().c_str(), name.c_str()), zf);
    return nullptr;
}

sp<AssetBundle> AssetBundleZipFile::getBundle(const String& path)
{
    return sp<AssetBundle>::make<AssetBundleWithPrefix>(sp<AssetBundle>::make<AssetBundleZipFile>(*this), path.endsWith("/") ? path : path + "/");
}

Vector<String> AssetBundleZipFile::listAssets()
{
    DFATAL("Unimplemented");
    return {};
}

bool AssetBundleZipFile::hasEntry(const String& name) const
{
    return zip_name_locate(_stub->archive(), name.c_str(), 0) != -1;
}

}
