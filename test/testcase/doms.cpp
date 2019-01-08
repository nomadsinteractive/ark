#include "core/types/shared_ptr.h"

#include <stdio.h>

#include "core/ark.h"
#include "core/base/string_buffer.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/impl/asset/directory_asset.h"
#include "core/impl/dictionary/xml_directory.h"
#include "core/util/documents.h"
#include "core/util/strings.h"
#include "core/types/weak_ptr.h"

#include "test/base/test_case.h"

#include "platform/platform.h"

namespace ark {
namespace unittest {

class DOMTestCase : public TestCase {
public:
    virtual int launch() {
        const document doc = Documents::loadFromReadable(Ark::instance().openAsset("application.xml"));
        if(!doc) {
            printf("application.xml not found!\n");
            return -1;
        }

        StringBuffer sb;
        Documents::print(doc, sb);

        for(const document& node : doc->children("frame"))
            Documents::print(node, sb);

        sp<Dictionary<document>> dict1 = sp<DictionaryByAttributeName>::make(doc, "id");
        sp<Dictionary<document>> dict2 = sp<DictionaryByAttributeName>::make(doc, "id", "expirable");

        if(!dict1->get("script"))
            return 1;
        if(!dict1->get("t4"))
            return 2;

        if(!dict2->get("e1"))
            return 3;
        if(!dict2->get("e004"))
            return 4;
        if(dict2->get("t4"))
            return 5;

        return 0;
    }

};

}
}


ark::unittest::TestCase* doms_create() {
    return new ark::unittest::DOMTestCase();
}
