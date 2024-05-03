#include "platform/darwin/impl/asset_bundle_darwin.h"

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFStream.h>

#include "core/base/string.h"
#include "core/inf/asset.h"
#include "core/inf/array.h"
#include "core/impl/readable/bytearray_readable.h"
#include "core/types/shared_ptr.h"

#include "platform/platform.h"

namespace ark {
namespace platform {
namespace darwin {

namespace {

class BundleAsset : public Asset {
public:
    BundleAsset(String location, CFURLRef url)
        : _location(std::move(location)), _url(url) {
    }
    ~BundleAsset() override {
        CFRelease(_url);
    }

    virtual sp<Readable> open() override {
        CFReadStreamRef stream = CFReadStreamCreateWithFile(nullptr, _url);
        std::vector<uint8_t> bytes;
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

    virtual String location() override {
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
    return url ? sp<BundleAsset>::make(Platform::pathJoin(_directory, name), url) : sp<BundleAsset>::null();
}

sp<AssetBundle> AssetBundleDarwin::getBundle(const String& path)
{
    return sp<AssetBundleDarwin>::make(Platform::pathJoin(_directory, path));
}

bool AssetBundleDarwin::exists(const String& location)
{
    CFBundleRef bundle = CFBundleGetMainBundle();
    CFStringRef cfDirectory = CFStringCreateWithCString(nullptr, location.c_str(), kCFStringEncodingUTF8);
    CFURLRef url = CFBundleCopyResourceURL(bundle, nullptr, nullptr, cfDirectory);
    CFRelease(cfDirectory);
    bool r = static_cast<bool>(url);
    if(url)
        CFRelease(url);
    return r;
}

std::vector<sp<Asset>> AssetBundleDarwin::listAssets(const String &regex)
{
    return {};
}

}
}
}
