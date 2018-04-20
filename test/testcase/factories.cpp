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
#include "core/types/weak_ptr.h"
#include "core/util/math.h"

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

        TESTCASE_VALIDATE(e1 && e1->val() == 0.0f);

        we1 = e1;

        TESTCASE_VALIDATE(we1.lock());

        std::this_thread::sleep_for(std::chrono::seconds(3));

        TESTCASE_VALIDATE(e1->val());

        sp<Numeric> g1 = beanFactory.build<Numeric>("25.0");
        TESTCASE_VALIDATE(g1 && g1->val() == 25.0f);

        sp<Numeric> g2 = beanFactory.load<Numeric>("linear");
        TESTCASE_VALIDATE(g2 && g2->val() == 100.0f);
        sp<Numeric> g3 = beanFactory.load<Numeric>("accelerate");
        TESTCASE_VALIDATE(g3 && g3->val() == 110.0f);

        sp<Numeric> g4 = beanFactory.load<Numeric>("g4");
        TESTCASE_VALIDATE(g4 && g4->val() == 0);

        sp<Numeric> g5 = beanFactory.load<Numeric>("g5");
        TESTCASE_VALIDATE(g5 && Math::almostEqual(g5->val(), 1.0f, 4));

        sp<Numeric> g6 = beanFactory.load<Numeric>("g6");
        TESTCASE_VALIDATE(g6 && g6->val() == 5.0f);

        const sp<Scope> args = sp<Scope>::make();
        args->put<Numeric>("t", sp<Numeric::Impl>::make(12.0f));
        sp<Numeric> g7 = beanFactory.load<Numeric>("g7", args);
        TESTCASE_VALIDATE(g7 && g7->val() == 12.0f);

        sp<Vec2> vertex = beanFactory.load<Vec2>("vertex");
        TESTCASE_VALIDATE(vertex && vertex->x() == 100.0f && vertex->y() == 110.0f);

        Class* clazz = Class::getClass<Vec2>();
        TESTCASE_VALIDATE(clazz);

        sp<Vec2> v1 = beanFactory.load<Vec2>("v1");
        TESTCASE_VALIDATE(v1 && v1->x() == 20 && v1->y() == 30);

        sp<Vec2> v2 = beanFactory.load<Vec2>("v2");
        TESTCASE_VALIDATE(v2 && v2->x() == 100 && v2->y() == 30);

        sp<Vec2> v3 = beanFactory.load<Vec2>("v3");
        TESTCASE_VALIDATE(v3 && v3->x() == 0 && v3->y() == 110);

//        const sp<Transform> t1 = beanFactory.load<Transform>("t1");
//        if(!t1 || t1->pivot()->val().x() != 20.0f || t1->pivot()->val().y() != 30.0f)
//            return 17;

        sp<Transform> t2 = beanFactory.load<Transform>("t2");
        TESTCASE_VALIDATE(t2 && t2->translation()->val().x() == 20.0f && t2->translation()->val().y() == 30.0f);

        sp<Transform> t3 = beanFactory.load<Transform>("t3");
        TESTCASE_VALIDATE(t3 && t3->scale()->val().x() == 2.0f && t3->scale()->val().y() == 2.0f);

        sp<Transform> t4 = beanFactory.load<Transform>("t4");
        TESTCASE_VALIDATE(t4 && t4->rotation()->val() == 1.0f);

        const sp<Expired> e004 = beanFactory.load<Expired>("e004");
        const sp<Expired> e004Copy = beanFactory.load<Expired>("e004");
        TESTCASE_VALIDATE(e004 == e004Copy);

        const sp<Numeric> g8 = beanFactory.load<Numeric>("g8");
        TESTCASE_VALIDATE(g8 && g8->val() == 8.0f);

        const sp<Numeric> g9 = beanFactory.load<Numeric>("g9", args);
        TESTCASE_VALIDATE(g9 && g9->val() == 15.0f);

        const sp<Numeric> g10 = beanFactory.load<Numeric>("g10", args);
        TESTCASE_VALIDATE(g10 && g10->val() == 6.0f);

        const sp<Numeric> g11 = beanFactory.load<Numeric>("g11", args);
        TESTCASE_VALIDATE(g11 && g11->val() == 14.0f);

        const sp<Numeric> g12 = beanFactory.load<Numeric>("g12", args);
        TESTCASE_VALIDATE(g12 && g12->val() == 30.0f);

        const sp<Numeric> g13 = beanFactory.load<Numeric>("g13");
        TESTCASE_VALIDATE(g13 && Math::almostEqual(g13->val(), 4.0f, 20));

        const sp<Boolean> g14 = beanFactory.load<Boolean>("g14");
        TESTCASE_VALIDATE(g14 && g14->val() == true);

        const sp<Boolean> g15 = beanFactory.load<Boolean>("g15");
        TESTCASE_VALIDATE(g15 && g15->val() == false);

        const sp<Boolean> g16 = beanFactory.load<Boolean>("g16");
        TESTCASE_VALIDATE(g16 && g16->val() == true);

        const sp<Integer> g17 = beanFactory.load<Integer>("g17");
        TESTCASE_VALIDATE(g17);
        TESTCASE_VALIDATE(g17->val() == 1 && g17->val() == 2 && g17->val() == 3
                          && g17->val() == 1 && g17->val() == 2 && g17->val() == 3);

        const sp<Integer> g18 = beanFactory.load<Integer>("g18");
        TESTCASE_VALIDATE(g18);
        TESTCASE_VALIDATE(g18->val() == 1 && g18->val() == 2 && g18->val() == 3
                          && g18->val() == 2 && g18->val() == 1
                          && g18->val() == 2 && g18->val() == 3);

        }

        const sp<Expired> expired = we1.lock();
        TESTCASE_VALIDATE(!expired);

        return 0;
    }
};

}
}


ark::unittest::TestCase* factories_create() {
    return new ark::unittest::FactoriesCase();
}
