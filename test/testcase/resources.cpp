#include "test/base/test_case.h"

#include "core/ark.h"
#include "core/impl/asset_bundle/asset_bundle_zip_file.h"
#include "core/inf/asset.h"
#include "core/inf/readable.h"

#include "platform/platform.h"

namespace ark {
namespace unittest {

class ResourcesTestCase : public TestCase {
public:
    virtual int launch() {
        const sp<Readable> readable = Ark::instance().openAsset("testcase.zip");
        if(!readable)
            return -1;
        const sp<AssetBundle> zip = sp<AssetBundleZipFile>::make(readable, "/");
        const sp<Readable> fp1 = zip->getAsset("boxes.cpp")->open();
        if(!fp1)
            return 1;
        String content = Strings::loadFromReadable(fp1);
        if(content.length() == 0)
            return 2;
        return 0;
    }
};

}
}


ark::unittest::TestCase* resources_create() {
    return new ark::unittest::ResourcesTestCase();
}
