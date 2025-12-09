#include "platform/darwin/impl/asset_bundle_darwin.h"

#include <filesystem>

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFStream.h>

#include "core/base/string.h"
#include "core/inf/asset.h"
#include "core/inf/array.h"
#include "core/impl/readable/bytearray_readable.h"
#include "core/types/shared_ptr.h"

#include "platform/platform.h"

namespace ark::platform::darwin {

namespace {

class BundleAsset final : public Asset {
public:
    BundleAsset(String location, const CFURLRef url)
        : _location(std::move(location)), _url(url) {
    }
    ~BundleAsset() override {
        CFRelease(_url);
    }

    sp<Readable> open() override {
        const CFReadStreamRef stream = CFReadStreamCreateWithFile(nullptr, _url);
        Vector<uint8_t> bytes;
        if(stream)
        {
            CFReadStreamOpen(stream);
            CFIndex readlen;
            uint8_t buf[8192];
            while((readlen = CFReadStreamRead(stream, buf, sizeof(buf))) > 0) {
                bytes.insert(bytes.end(), std::begin(buf), std::begin(buf) + readlen);
            }
            CFReadStreamClose(stream);
        }

        if(stream)
            return sp<Readable>::make<BytearrayReadable>(sp<ByteArray>::make<ByteArray::Vector>(std::move(bytes)));

        return nullptr;
    }

    String location() override
    {
        return _location;
    }

    size_t size() override
    {
        return std::filesystem::file_size(_location.c_str());
    }

private:
    String _location;
    CFURLRef _url;
};

}

AssetBundleDarwin::AssetBundleDarwin(String directory)
    : _directory(std::move(directory))
{
}

sp<Asset> AssetBundleDarwin::getAsset(const String& name)
{
    const CFBundleRef bundle = CFBundleGetMainBundle();
    const CFStringRef cfDirectory = CFStringCreateWithCString(nullptr, _directory.c_str(), kCFStringEncodingUTF8);
    const CFStringRef cfName = CFStringCreateWithCString(nullptr, name.c_str(), kCFStringEncodingUTF8);
    CFURLRef url = CFBundleCopyResourceURL(bundle, cfName, nullptr, cfDirectory);
    CFRelease(cfDirectory);
    CFRelease(cfName);
    return url ? sp<Asset>::make<BundleAsset>(Platform::pathJoin(_directory, name), url) : sp<Asset>();
}

sp<AssetBundle> AssetBundleDarwin::getBundle(const String& path)
{
    return sp<AssetBundle>::make<AssetBundleDarwin>(Platform::pathJoin(_directory, path));
}

Vector<String> AssetBundleDarwin::listAssets(StringView dirname)
{
    return {};
}

}
