#include "test/base/test_case.h"

#include "core/ark.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/util/documents.h"

#include "app/base/application_context.h"

namespace ark {
namespace unittest {

TestCase::TestCase(const String& factoryFile)
    : _factory_file(factoryFile)
{
}

sp<BeanFactory> TestCase::getBeanFactory() const
{
    const document doc = Documents::loadFromReadable(Ark::instance().openAsset(_factory_file));
    DCHECK(doc, "%s not found!", _factory_file.c_str());
    const sp<Dictionary<document>> byId = sp<DictionaryByAttributeName>::make(doc, "id");
    return Ark::instance().createBeanFactory(byId);
}

sp<ResourceLoader> TestCase::getResourceLoader() const
{
    return Ark::instance().applicationContext()->createResourceLoader(_factory_file, Scope());
}

}
}
