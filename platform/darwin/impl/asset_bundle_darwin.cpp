#include "platform/darwin/impl/asset_bundle_darwin.h"

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
    BundleAsset(String location, CFURLRef url)
        : _location(std::move(location)), _url(url) {
    }
    ~BundleAsset() override {
        CFRelease(_url);
    }

    sp<Readable> open() override {
        CFReadStreamRef stream = CFReadStreamCreateWithFile(nullptr, _url);
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
            return sp<BytearrayReadable>::make(sp<ByteArray::Vector>::make(std::move(bytes)));

        return nullptr;
    }

    String location() override {
        return _location;
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
    CFBundleRef bundle = CFBundleGetMainBundle();
    CFStringRef cfDirectory = CFStringCreateWithCString(nullptr, _directory.c_str(), kCFStringEncodingUTF8);
    CFStringRef cfName = CFStringCreateWithCString(nullptr, name.c_str(), kCFStringEncodingUTF8);
    CFURLRef url = CFBundleCopyResourceURL(bundle, cfName, nullptr, cfDirectory);
    CFRelease(cfDirectory);
    CFRelease(cfName);
    return url ? sp<Asset>::make<BundleAsset>(Platform::pathJoin(_directory, name), url) : nullptr;
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
