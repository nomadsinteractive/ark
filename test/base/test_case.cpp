#include "test/base/test_case.h"

#include "core/ark.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/util/documents.h"


#include "app/base/application_context.h"

namespace ark {
namespace unittest {

sp<BeanFactory> TestCase::getBeanFactory() const
{
    const document doc = Documents::loadFromReadable(Ark::instance().openAsset("application.xml"));
    if(!doc) {
        printf("application.xml not found!\n");
        return nullptr;
    }
    const sp<Dictionary<document>> byId = sp<DictionaryByAttributeName>::make(doc, "id");
    return Ark::instance().createBeanFactory(byId);
}

sp<ResourceLoader> TestCase::getResourceLoader() const
{
    return Ark::instance().applicationContext()->createResourceLoader("application.xml", Scope());
}

}
}
