#include "test/base/test_case.h"

#include <chrono>
#include <thread>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/scope.h"
#include "core/base/thread.h"
#include "core/epi/expired.h"
#include "core/inf/dictionary.h"
#include "core/inf/variable.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/epi/expired.h"
#include "core/impl/numeric/numeric_expression.h"
#include "core/types/weak_ptr.h"

#include "graphics/base/vec2.h"
#include "graphics/base/transform.h"

#include "app/base/application_context.h"
#include "app/base/resource_loader.h"

#include "platform/platform.h"


namespace ark {
namespace unittest {

class FactoriesCase : public TestCase {
public:
    virtual int launch() {
        Ark& ark = Ark::instance();
        WeakPtr<Expired> we1;
        {
        const sp<ResourceLoader> resourceLoader = ark.applicationContext()->createResourceLoader("application.xml");
        BeanFactory& beanFactory = resourceLoader->beanFactory();
        ark.applicationContext()->clock()->setTicker(Platform::getSteadyClock());

        const sp<Expired> e1 = beanFactory.load<Expired>("e1");

        if(!e1 || e1->val())
            return 1;
        we1 = e1;

        if(!we1.lock())
            return 2;

        std::this_thread::sleep_for(std::chrono::seconds(3));

        if(!e1->val())
            return 4;

        sp<Numeric> g1 = beanFactory.build<Numeric>("25.0");
        if(!g1 || g1->val() != 25.0f)
            return 5;

        sp<Numeric> g2 = beanFactory.load<Numeric>("linear");
        if(!g2 || g2->val() != 100.0f)
            return 6;
        sp<Numeric> g3 = beanFactory.load<Numeric>("accelerate");
        if(!g3 || g3->val() != 110.0f)
            return 7;

        sp<Numeric> g4 = beanFactory.load<Numeric>("g4");
        if(!g4 || g4->val() != 0)
            return 8;

        sp<Numeric> g5 = beanFactory.load<Numeric>("g5");
        if(!g5 || g5->val() != 1.0f)
            return 9;

        sp<Numeric> g6 = beanFactory.load<Numeric>("g6");
        if(!g6 || g6->val() != 6.0f)
            return 10;

        const sp<Scope> args = sp<Scope>::make();
        args->put<Numeric>("t", sp<Numeric::Impl>::make(12.0f));
        sp<Numeric> g7 = beanFactory.load<Numeric>("g7", args);
        if(!g7 || g7->val() != 13.0f)
            return 11;

        sp<Vec2> vertex = beanFactory.load<Vec2>("vertex");
        if(!vertex || vertex->x() != 100.0f || vertex->y() != 110.0f)
            return -11;

        Class* clazz = Class::getClass<Vec2>();
        if(!clazz)
            return 12;

        sp<Vec2> v1 = beanFactory.load<Vec2>("v1");
        if(!v1 || v1->x() != 20 || v1->y() != 30)
            return 13;

        sp<Vec2> v2 = beanFactory.load<Vec2>("v2");
        if(!v2 || v2->x() != 100 || v2->y() != 30)
            return 14;

        sp<Vec2> v3 = beanFactory.load<Vec2>("v3");
        if(!v3 || v3->x() != 0 || v3->y() != 110)
            return 15;

        const sp<Transform> t1 = beanFactory.load<Transform>("t1");
        if(!t1 || t1->pivot()->val().x() != 20.0f || t1->pivot()->val().y() != 30.0f)
            return 17;

        sp<Transform> t2 = beanFactory.load<Transform>("t2");
        if(!t2 || t2->translation()->val().x() != 20.0f || t2->translation()->val().y() != 30.0f)
            return 18;

        sp<Transform> t3 = beanFactory.load<Transform>("t3");
        if(!t3 || t3->scale()->val().x() != 2.0f || t3->scale()->val().y() != 2.0f)
            return 19;

        sp<Transform> t4 = beanFactory.load<Transform>("t4");
        if(!t4 || t4->rotation()->val() != 1.0f)
            return 20;

        const sp<Expired> e004 = beanFactory.load<Expired>("e004");
        const sp<Expired> e004Copy = beanFactory.load<Expired>("e004");
        if(e004 != e004Copy)
            return 21;

        const sp<Numeric> g8 = beanFactory.load<Numeric>("g8");
        if(!g8 || g8->val() != 8.0f)
            return 22;

        const sp<Numeric> g9 = beanFactory.load<Numeric>("g9", args);
        if(!g9 || g9->val() != 16.0f)
            return 23;

        const sp<Numeric> g10 = beanFactory.load<Numeric>("g10", args);
        if(!g10 || g10->val() != 6.4f)
            return 24;

        const sp<Numeric> g11 = beanFactory.load<Numeric>("g11", args);
        if(!g11 || g11->val() != 14.4f)
            return 25;

        const sp<Numeric> g12 = beanFactory.load<Numeric>("g12", args);
        if(!g12 || g12->val() != 30.4f)
            return 26;

        const sp<Numeric> g13 = beanFactory.load<Numeric>("g13");
        if(!g13 || g13->val() != 4.0f)
            return 27;

        const sp<Boolean> g14 = beanFactory.load<Boolean>("g14");
        if(!g14 || g14->val() != true)
            return 28;

        const sp<Boolean> g15 = beanFactory.load<Boolean>("g15");
        if(!g15 || g15->val() != false)
            return 29;

        const sp<Boolean> g16 = beanFactory.load<Boolean>("g16");
        if(!g16 || g16->val() != true)
            return 30;
        }

        const sp<Expired> expired = we1.lock();
        if(expired)
            return 31;

        return 0;
    }
};

}
}


ark::unittest::TestCase* factories_create() {
    return new ark::unittest::FactoriesCase();
}
